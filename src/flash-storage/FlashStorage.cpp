#include "FlashStorage.h"

FlashStorageController::FlashStorageController(char* deviceName)
{	
	PRINT_INFO(DEBUG_FLASHSTORAGE_NAME, "Flash initialising");
	_deviceCRC = CRC16::Calculate(CRC_SEED,deviceName,strlen(deviceName));
	_header = CreateBlankHeader();
	if (!ReadHeader())
	{
		//Uhoh? Couldn't read from flash... HELP
		PRINT_ERR(DEBUG_FLASHSTORAGE_NAME,"Could not read from flash. Fatal unrecoveredable error.");
	}

	PRINT_INFO(DEBUG_FLASHSTORAGE_NAME,"Header tag was: %x", _header->headerID);
	PRINT_INFO(DEBUG_FLASHSTORAGE_NAME,"Header name was: %s", _header->headerName);

	if(!ValidateHeader(_header))
	{		
		//Not a valid header, dispose it and create a new one.
		free(_header);
		_header = CreateBlankHeader();
		WriteHeader(_header);
	}

	for(int i=0; i<FLASH_MAX_NUMBER_OF_STORAGE_AREAS;i++)
	{
		PRINT_VERBOSE(DEBUG_FLASHSTORAGE_NAME, "StorageArea Offset %lu BlockUsed %zu InUse:%d ArraySize: %zu", _header->storageAreas[i].offset, _header->storageAreas[i].blocksUsed, _header->storageAreas[i].used, _header->storageAreas[i].objectQuantityPerBlock);
	}
	
}

int FlashStorageController::GetNextStorageSlot()
{	
	if(_slotsUsed > FLASH_MAX_NUMBER_OF_STORAGE_AREAS)
	{
		PRINT_ERR(DEBUG_FLASHSTORAGE_NAME, "All storage areas are in use.");
	}
	return _slotsUsed++;
}

FlashStorageController::StorageArea* FlashStorageController::GetStorageHeader(int slot)
{
	return &(_header->storageAreas[slot]);
}

bool FlashStorageController::FormatSlot(int slot, size_t objectSize, size_t objectQuantityPerBlock, size_t numberOfBlocks)
{	
	PRINT_INFO(DEBUG_FLASHSTORAGE_NAME,"Formatting & configuring new storage slot...");
	FlashStorageController::StorageArea* _storageSlot = &(_header->storageAreas[slot]);
		
	size_t pagesReservedPerBlock = ((objectSize * objectQuantityPerBlock) / FLASH_PAGE_SIZE) + 1;
	size_t sectorsReservedPerBlock = ((pagesReservedPerBlock*FLASH_PAGE_SIZE) / FLASH_SECTOR_SIZE) + 1;

	_storageSlot->used = true;
	_storageSlot->offset = CalculateSlotOffset(slot);
	_storageSlot->sectorsReserved = sectorsReservedPerBlock * numberOfBlocks;
	_storageSlot->blocksUsed = 0;
	_storageSlot->blocksReserved = numberOfBlocks;
	_storageSlot->objectSize = objectSize;
	_storageSlot->objectQuantityPerBlock = objectQuantityPerBlock;
	_storageSlot->objectQuantityLastBlock = 0;	

	for (int i = slot+1; i < FLASH_MAX_NUMBER_OF_STORAGE_AREAS; i++)
	{
		//Wipe subsequent blocks as offsets will change
		_header->storageAreas[i].used = false;
	}

	if ((_storageSlot->offset + _storageSlot->sectorsReserved * FLASH_SECTOR_SIZE) >= PICO_FLASH_SIZE_BYTES)
	{
		PRINT_ERR(DEBUG_FLASHSTORAGE_NAME, "Tried to allocate a storage area beyond flash limits");
		return false;
	}
	else
	{ 		
		uint32_t ints = save_and_disable_interrupts();
		flash_range_erase(_storageSlot->offset, _storageSlot->sectorsReserved*FLASH_SECTOR_SIZE);
		restore_interrupts(ints);
		PRINT_SUCCESS(DEBUG_FLASHSTORAGE_NAME, "Flash storage allocated!");
		return true;
	}
}

bool FlashStorageController::Write(int slot, uint8_t* data, size_t objectQuantity)
{
	
	if (_header->storageAreas[slot].blocksUsed == _header->storageAreas[slot].blocksReserved)
	{
		return false;
	}
	uint32_t offset = GetSlotBlockOffset(slot, _header->storageAreas[slot].blocksUsed);
	size_t pagesRequired = ((objectQuantity * _header->storageAreas[slot].objectSize) / FLASH_PAGE_SIZE) + 1;
	size_t sectorsRequired = ((pagesRequired*FLASH_PAGE_SIZE) / FLASH_SECTOR_SIZE) + 1;
	PRINT_INFO(DEBUG_FLASHSTORAGE_NAME,"Writing %zu pages to slot %d", pagesRequired, slot);
	uint32_t ints = save_and_disable_interrupts();
	flash_range_erase(offset, sectorsRequired*FLASH_SECTOR_SIZE);
	flash_range_program(offset, data, FLASH_PAGE_SIZE * pagesRequired);
	restore_interrupts(ints);
	PRINT_SUCCESS(DEBUG_FLASHSTORAGE_NAME, "Data saved to flash");
	_header->storageAreas[slot].objectQuantityLastBlock = objectQuantity;
	return true;
}

void FlashStorageController::MarkBlockAsCompleteWrite(int slot)
{
	_header->storageAreas[slot].blocksUsed++;
	_header->storageAreas[slot].objectQuantityLastBlock = 0;	
}

size_t FlashStorageController::Read(int slot, uint8_t* data)
{	
	if(_header->storageAreas[slot].objectQuantityLastBlock == 0)
	{
		RevertLastWrite(slot);
	}
	
	size_t numberOfObjects = _header->storageAreas[slot].objectQuantityLastBlock;
	size_t objectSize = _header->storageAreas[slot].objectSize;
	uint32_t offset = GetSlotBlockOffset(slot, _header->storageAreas[slot].blocksUsed);

	if (numberOfObjects > 0) 
	{
		PRINT_INFO(DEBUG_FLASHSTORAGE_NAME,"Reading %lu bytes (%zu objects) from slot %d...", numberOfObjects*objectSize,numberOfObjects, slot);
		memcpy(data, _flashPointer + offset, numberOfObjects*objectSize);
		_header->storageAreas[slot].objectQuantityLastBlock = 0;
		return numberOfObjects;
	}
	else
	{
		return 0;
	}	
}

void FlashStorageController::RevertLastWrite(int slot)
{	
	if(_header->storageAreas[slot].blocksUsed > 0)
	{
		_header->storageAreas[slot].blocksUsed--;
		_header->storageAreas[slot].objectQuantityLastBlock = _header->storageAreas[slot].objectQuantityPerBlock;
	}
}

void FlashStorageController::WriteHeader()
{
	WriteHeader(_header);
}

///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////


bool FlashStorageController::ReadHeader()
{
	//READ FROM FLASH
	uint8_t* arrayPointer = reinterpret_cast<uint8_t*>(_header);
	memcpy(arrayPointer,_flashPointer+FLASH_STARTING_OFFSET, sizeof(HeaderStruct) );	    
	return true;
}

FlashStorageController::HeaderStruct* FlashStorageController::CreateBlankHeader()
{
	FlashStorageController::HeaderStruct* newHeader = new FlashStorageController::HeaderStruct();
	newHeader->headerID = FLASH_HEADER_ID;

	for (int i = 0; i < FLASH_HEADER_NAME_LENGTH; i++)
	{
		newHeader->headerName[i] = FLASH_HEADER_NAME[i];
	}	

	for (int i = 0; i < FLASH_MAX_NUMBER_OF_STORAGE_AREAS; i++)
	{
		newHeader->storageAreas[i].used = false;
	}

	newHeader->deviceCRC = _deviceCRC;
	//Storage area CRC gets computed during header write.

	return newHeader;
}


bool FlashStorageController::WriteHeader(FlashStorageController::HeaderStruct* header)
{
	//Calcluate CRC
	header->storageCRC = CalculateStorageCRC(header);

	//Writing to a temporary location padded with zeros for a cleaner write
	uint8_t writeData[256]  = { 255 };

	//Write to flash
	uint8_t* arrayPointer = reinterpret_cast<uint8_t*>(header);
	memcpy(writeData, arrayPointer, sizeof(HeaderStruct) );	
	PRINT_INFO(DEBUG_FLASHSTORAGE_NAME,"Writing header...");
	uint32_t ints = save_and_disable_interrupts();
	flash_range_erase(FLASH_STARTING_OFFSET, FLASH_SECTOR_SIZE);
	flash_range_program(FLASH_STARTING_OFFSET, writeData, FLASH_PAGE_SIZE);
	restore_interrupts(ints);
	PRINT_SUCCESS(DEBUG_FLASHSTORAGE_NAME, "Header written to flash");
	return true;
}

bool FlashStorageController::ValidateHeader(FlashStorageController::HeaderStruct* header)
{
	//Check Header
	if (header->headerID != FLASH_HEADER_ID)
	{
		PRINT_WARN(DEBUG_FLASHSTORAGE_NAME,"Invalid header tag. Re-initilising.");
		return false;
	}

	//Check Version
	char correctHeader[FLASH_HEADER_NAME_LENGTH] = FLASH_HEADER_NAME;
	for(int i=0; i<FLASH_HEADER_NAME_LENGTH;i++)
	{
		if (header->headerName[i] != correctHeader[i])
		{
			PRINT_WARN(DEBUG_FLASHSTORAGE_NAME,"Invalid header version. Re-initilising.");
			return false;
		}
	}	

	//Check Device CRC
	if (header->deviceCRC != _deviceCRC)
	{
		PRINT_WARN(DEBUG_FLASHSTORAGE_NAME,"Storage devicename mismatch. Re-initialising.");
		return false;
	}

	//Check StorageDefinition CRC
	if (header->storageCRC != CalculateStorageCRC(header))
	{
		PRINT_ERR(DEBUG_FLASHSTORAGE_NAME,"CRC validation failed on storage definitions. Re-initialising.");
		return false;
	}

	PRINT_SUCCESS(DEBUG_FLASHSTORAGE_NAME,"Flash header validated");
	return true;
}

uint32_t FlashStorageController::CalculateSlotOffset(int slot)
{
	uint32_t offset = FLASH_STARTING_OFFSET + FLASH_SECTOR_SIZE;

	for (int i = 0; i < slot; i++)
	{
		if(_header->storageAreas[i].used)
		{
			offset = _header->storageAreas[i].offset + (_header->storageAreas[i].sectorsReserved+1)*FLASH_SECTOR_SIZE + FLASH_SECTOR_SIZE;
		}		
	}	
	return offset;
}

uint32_t FlashStorageController::GetSlotBlockOffset(int slot, size_t block)
{
	uint32_t slotOffset = _header->storageAreas[slot].offset;
	uint32_t sectorsPerBlock = _header->storageAreas[slot].sectorsReserved / _header->storageAreas[slot].blocksReserved;
	uint32_t out = slotOffset + (sectorsPerBlock*((uint32_t)(block))*FLASH_SECTOR_SIZE);
	return out;
}

void FlashStorageController::print_buf(const uint8_t *buf, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        printf("%02x", buf[i]);
        if (i % 16 == 15)
            printf("\n");
        else
            printf(" ");
    }
}

uint16_t FlashStorageController::CalculateStorageCRC(FlashStorageController::HeaderStruct* header)
{
	return CRC16::Calculate(CRC_SEED, header->storageAreas, sizeof(StorageArea)*FLASH_MAX_NUMBER_OF_STORAGE_AREAS);
}
