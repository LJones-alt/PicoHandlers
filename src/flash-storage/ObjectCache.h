#ifndef DMFLASHSTORAGE_H
#define DMFLASHSTORAGE_H

#include <cstring>
#include "FlashStorage.h"
#include <cstddef>
#include "CRC/CRC16.h"


template<class T>
class ObjectCache
{
	public:
		ObjectCache(size_t cacheArraySize, size_t numberOfBlocks, FlashStorageController* flashStorageController);
		bool Add(T* object);
		bool Pop(T** returnedObject);
		size_t Count();
		bool IsFull();
		void Save();
		int _storageSlot;

	private:
		struct StoredObject
		{
			T object;
			uint16_t crc;
		};

		FlashStorageController* _flashStorageController;		
		FlashStorageController::StorageArea* _storageArea;

		StoredObject* _objectArray;
		size_t _objectArraySize;
		size_t _objectArrayUsed;

		void WriteBufferToFlash();
		bool ReadFromStorage();
};


template<class T>
inline ObjectCache<T>::ObjectCache(size_t cacheArraySize, size_t numberOfBlocks, FlashStorageController* flashStorageController)
{
	_flashStorageController = flashStorageController;	
	_storageSlot = _flashStorageController->GetNextStorageSlot();

	//Initialise array (and initialise bigger than the memory page).
	size_t pagesRequired = ((cacheArraySize * sizeof(StoredObject)) / FLASH_PAGE_SIZE) + 1;
	_objectArray = new StoredObject[(pagesRequired*FLASH_PAGE_SIZE)/sizeof(StoredObject)]{};
	_objectArraySize = cacheArraySize;
	_objectArrayUsed = 0;

	//Get Storage Info
	_storageArea = _flashStorageController->GetStorageHeader(_storageSlot);

	//If storage area doesn't match or isn't in use, then initialise it.
	if (_storageArea->objectSize != sizeof(StoredObject) || _storageArea->used == false || numberOfBlocks != _storageArea->blocksReserved || cacheArraySize != _storageArea->objectQuantityPerBlock)
	{
		_flashStorageController->FormatSlot(_storageSlot, sizeof(StoredObject), cacheArraySize, numberOfBlocks);
		PRINT_WARN(DEBUG_FLASHSTORAGE_NAME, "Initialised a new slot %d, reserving a total of %zu sectors", _storageSlot, _storageArea->sectorsReserved);
	}
	else
	{
		PRINT_SUCCESS(DEBUG_FLASHSTORAGE_NAME, "Found and recovered data from slot %d, %zu sectors in use", _storageSlot, _storageArea->sectorsReserved);
		ReadFromStorage();
	}	
}

template<class T>
bool ObjectCache<T>::Add(T* object)
{	
	if (_objectArrayUsed >= _objectArraySize)
	{
		//Write to flash.
		WriteBufferToFlash();
		_flashStorageController->MarkBlockAsCompleteWrite(_storageSlot);
		_objectArrayUsed = 0;
	}

	if (_storageArea->blocksReserved == _storageArea->blocksUsed)
	{
		//Full...
		return false;
	}

	memcpy(&_objectArray[_objectArrayUsed],object , sizeof(T));
	_objectArray[_objectArrayUsed].crc = CRC16::Calculate(CRC_SEED, object, sizeof(T));
	_objectArrayUsed++;
	return true;
}

template<class T>
bool ObjectCache<T>::Pop(T** returnedObject)
{
	if (_objectArrayUsed <= 0)
	{		
		bool hasNewData = ReadFromStorage();
		if( !hasNewData )
		{			
			//Tried to fetch new data, none found.
			returnedObject = NULL;
			return false;
		}		
	}

	_objectArrayUsed--;
	*returnedObject = &_objectArray[_objectArrayUsed].object;
	return true;
}

template<class T>
bool ObjectCache<T>::IsFull()
{
	if(_storageArea->blocksUsed == _storageArea->blocksReserved)
	{
		//Full in flash
		return true;
	}
	else if(_storageArea->blocksUsed == (_storageArea->blocksReserved -1) && _objectArrayUsed>=(_objectArraySize - 1))
	{
		//Full in flash, but first chunk read into ram
		return true;
	}
	else
	{
		return false;
	}
}

template<class T>
inline void ObjectCache<T>::WriteBufferToFlash()
{
	PRINT_INFO(DEBUG_FLASHSTORAGE_NAME, "Writing buffer to flash");
	uint8_t* arrayPointer = reinterpret_cast<uint8_t*>(_objectArray);
	_flashStorageController->Write(_storageSlot, arrayPointer, _objectArrayUsed);
}

template<class T>
inline size_t ObjectCache<T>::Count()
{
	return _storageArea->blocksUsed * _storageArea->objectQuantityPerBlock + _objectArrayUsed;		
}

template<class T>
inline void ObjectCache<T>::Save()
{
	WriteBufferToFlash();
	_flashStorageController->WriteHeader();
}

template<class T>
inline bool ObjectCache<T>::ReadFromStorage()
{	
	uint8_t* arrayPointer = reinterpret_cast<uint8_t*>(_objectArray);
	_objectArrayUsed = _flashStorageController->Read(_storageSlot, arrayPointer);	

	//Check CRC	
	for(int i=0; i<_objectArrayUsed; i++)
	{
		uint16_t objectCRC = CRC16::Calculate(CRC_SEED, &_objectArray[i].object, sizeof(T));
		 if(_objectArray[i].crc != objectCRC)
		 {
			//CRC Mismatch
			PRINT_ERR(DEBUG_FLASHSTORAGE_NAME, "CRC Failure on object %d - Got %u, expected %u.", i, objectCRC, _objectArray[i].crc);
			//Todo send alert.

			//Report as zero read.
			_objectArrayUsed = 0;					
			return false;
		 }
	}

	return _objectArrayUsed > 0;
}


#endif