#include "Encryption.h"

Encryption::Encryption()
{          
    uint8_t volatile keyDefinition[DM_ENC_KEY_SIZE] = DM_ENC_TAG;
    _key = (Key*)malloc(sizeof(Key));
    
    for(int i=1; i < DM_ENC_KEY_SIZE; i++)
    {
        _key->key[i] = keyDefinition[i] + keyDefinition[0];
    }
    _key->key[0] = keyDefinition[0];
}

Encryption::~Encryption()
{
    delete _key;
}

void Encryption::DecryptMem(uint8_t* data, size_t size)
{        
    uint8_t* key = _key->key;

    //Start offset is size/4 (+ byte at offset)
    size_t offset = size/4;
    offset+=key[offset];

    for(size_t i = 0; i < size; i++)
    {
        data[i] = data[i] ^ key[(DM_ENC_KEY_SIZE - offset - i) % DM_ENC_KEY_SIZE];
    }    
}