#include <stdint.h>
#include <cstddef>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "debug/PrettySerial.h"
#include "CRC/CRC16.h"

#define FLASH_HEADER_NAME "SENSIFY-FSv1.5"
#define FLASH_HEADER_NAME_LENGTH 15
#define FLASH_HEADER_ID 0xFF01FE00
#define FLASH_MAX_NUMBER_OF_STORAGE_AREAS 5
#define FLASH_STARTING_OFFSET 0xFF000
#define DEBUG_FLASHSTORAGE_NAME "Flash"
#define CRC_SEED 0xFFFF

class FlashStorageController
{
	public:
		struct StorageArea
		{
			bool used;
			uint32_t offset;
			size_t sectorsReserved;
			size_t blocksUsed;
			size_t blocksReserved;
			size_t objectSize;
			size_t objectQuantityPerBlock;
			size_t objectQuantityLastBlock;
		};

		FlashStorageController(char* deviceName);
		int GetNextStorageSlot();
		StorageArea* GetStorageHeader(int slot);
		bool FormatSlot(int slot, size_t objectSize, size_t objectQuantityPerBlock, size_t numberOfBlocks);
		bool Write(int slot, uint8_t* data, size_t objectQuantity);		
		size_t Read(int slot, uint8_t* data);
		void RevertLastWrite(int slot);
		void WriteHeader();
		void MarkBlockAsCompleteWrite(int slot);

	private:		

		struct HeaderStruct
		{
			uint32_t headerID;			
			StorageArea storageAreas[FLASH_MAX_NUMBER_OF_STORAGE_AREAS];
			uint16_t storageCRC;
			uint16_t deviceCRC;
			char headerName[FLASH_HEADER_NAME_LENGTH];
		};		
		const uint8_t *_flashPointer = (const uint8_t *) (XIP_BASE);
		
		HeaderStruct* _header;
		bool _hasHeader;
		int _slotsUsed = 0;
		uint16_t _deviceCRC;

		HeaderStruct* CreateBlankHeader();
		bool ReadHeader();
		bool WriteHeader(HeaderStruct* header);		
		bool ValidateHeader(HeaderStruct* header);
		uint32_t CalculateSlotOffset(int slot);
		uint32_t GetSlotBlockOffset(int slot, size_t block);
		void print_buf(const uint8_t *buf, size_t len);
		uint16_t CalculateStorageCRC(HeaderStruct* header);
};