#include "flash.h"

uint8_t flash_content[1024*1024*16];
SPIFlash flash;

bool SPIFlash::eraseSectorAddress(uint32_t address) 
{
  address >>= 8;
  for (int i = 0; i < 4096; i++) flash_content[address++] = 0xff;
  return true;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool SPIFlash::eraseSector(uint16_t page_number, uint8_t offset)
{
  return eraseSectorAddress(PAGESIZE * page_number + offset);
}

bool SPIFlash::eraseChip()
{
  memset(flash_content, 0xff, sizeof(flash_content));
  return true;
}

