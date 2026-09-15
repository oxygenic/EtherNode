// flash_storage.c
#include "flash_storage.h"
#include "stm32h7xx_hal.h"
#include <string.h>
#include <stdbool.h>

#define FLASH_STORAGE_START_ADDR ((uint32_t)0x08060000)
#define FLASH_STORAGE_END_ADDR   ((uint32_t)FLASH_STORAGE_START_ADDR+(1024*64)) //use only half of the sector, may be we need the other halve for something else later...
#define FLASH_STORAGE_SIZE       (FLASH_STORAGE_END_ADDR-FLASH_STORAGE_START_ADDR)

static bool Flash_Storage_Erase(void)
{
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef eraseInit;
    uint32_t pageError = 0xFFFFFFFF;

    eraseInit.TypeErase     = FLASH_TYPEERASE_SECTORS;
    eraseInit.Banks         = FLASH_BANK_1;
    eraseInit.Sector        = FLASH_SECTOR_3;
    eraseInit.NbSectors     = 1;
    eraseInit.VoltageRange  = FLASH_VOLTAGE_RANGE_3;

    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&eraseInit, &pageError);
    HAL_FLASH_Lock();
    SCB_InvalidateDCache_by_Addr((uint32_t*)FLASH_STORAGE_START_ADDR,
                                 FLASH_STORAGE_END_ADDR - FLASH_STORAGE_START_ADDR + 1);

    if (status != HAL_OK)
    {
       // Erase selbst fehlgeschlagen
       return false;
    }

    uint32_t start = FLASH_STORAGE_START_ADDR;
    uint32_t end   = FLASH_STORAGE_END_ADDR;
    for (uint32_t addr = start; addr < end; addr += 4)
    {
       uint32_t val = *(volatile uint32_t*)addr;
       if (val != 0xFFFFFFFF)
       {
          HAL_FLASH_Unlock();
          status = HAL_FLASHEx_Erase(&eraseInit, &pageError);
          HAL_FLASH_Lock();
          SCB_InvalidateDCache_by_Addr((uint32_t*)FLASH_STORAGE_START_ADDR,
                                       FLASH_STORAGE_END_ADDR - FLASH_STORAGE_START_ADDR + 1);

          if (status != HAL_OK)
          {
             return false;
          }

          // Nochmals prüfen
          addr = start;
          continue;
       }
    }

    return true;
}


uint8_t Flash_Storage_Write(const void* data,const size_t length,const uint8_t ID)
{
    if (!data || length == 0) return 0;

    uint32_t base_addr = FLASH_STORAGE_START_ADDR;
    uint32_t final_addr = base_addr + length;

    uint32_t *finalData=(uint32_t*)base_addr;
    const uint8_t *flashID=(uint8_t*)final_addr;
    if ((*flashID!=ID) && (*flashID!=0xFF)) final_addr=FLASH_STORAGE_END_ADDR;

    while ((final_addr<FLASH_STORAGE_END_ADDR) &&
          (*finalData!=0xFFFFFFFF))
    {
       base_addr+=length;
       final_addr+=length;
       finalData=(uint32_t*)base_addr;
    }

    if (final_addr>=FLASH_STORAGE_END_ADDR)
    {
       if (!Flash_Storage_Erase()) return false;
       base_addr = FLASH_STORAGE_START_ADDR;
       final_addr = base_addr + length;
    }

    if (final_addr>=FLASH_STORAGE_END_ADDR)
     return 0;

    __disable_irq();
    HAL_FLASH_Unlock();

    uint32_t write_addr = base_addr;
    uint32_t end_addr   = final_addr;

    const uint8_t* src = (const uint8_t*)data;

    __attribute__((aligned(32))) uint8_t flashword[FLASHWORD_SIZE];

    while (write_addr < end_addr) {
        memcpy(flashword, (const void*)write_addr, FLASHWORD_SIZE);

        for (uint32_t i = 0; i < FLASHWORD_SIZE; ++i) {
            uint32_t abs_index = write_addr + i;
            if (abs_index >= base_addr && abs_index < final_addr) {
                flashword[i] = src[abs_index - base_addr];
            }
        }

        uint8_t needs_write =0;
        for (int i = 0; i < FLASHWORD_SIZE; ++i) {
            if (((uint8_t*)write_addr)[i] != flashword[i]) {
                needs_write = 1;
                break;
            }
        }

        if (needs_write) {
            const uint64_t* qw = (const uint64_t*)flashword;
            HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, write_addr, (uint32_t)qw);
            if (status != HAL_OK) {
                HAL_FLASH_Lock();
                return false;
            }
        }

        write_addr += FLASHWORD_SIZE;
    }

    HAL_FLASH_Lock();
    __enable_irq();

    SCB_InvalidateDCache_by_Addr((uint32_t*)FLASH_STORAGE_START_ADDR,
                                 FLASH_STORAGE_END_ADDR - FLASH_STORAGE_START_ADDR + 1);

    return 1;
}

void *Flash_Storage_Read(const size_t length,const uint8_t ID)
{
   if (length == 0) return NULL;

   uint32_t base_addr = FLASH_STORAGE_START_ADDR;
   uint32_t final_addr = base_addr + length;

   uint32_t *finalData=(uint32_t*)final_addr;
   const uint8_t *flashID=(uint8_t*)base_addr;
   if ((*flashID!=ID) && (*flashID!=0xFF)) return NULL;
   while ((final_addr<FLASH_STORAGE_END_ADDR) &&
         (*finalData!=0xFFFFFFFF))
   {
      base_addr+=length;
      final_addr+=length;
      finalData=(uint32_t*)final_addr;
   }

   return (void*)base_addr;
}

