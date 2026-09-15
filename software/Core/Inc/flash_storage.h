#ifndef FLASH_STORAGE_H
#define FLASH_STORAGE_H

#include <stdint.h>
#include <stddef.h>

#define FLASHWORD_SIZE 32

uint8_t Flash_Storage_Write(const void* data,const size_t length,const uint8_t ID);
void *Flash_Storage_Read(const size_t length,const uint8_t ID);

#endif

