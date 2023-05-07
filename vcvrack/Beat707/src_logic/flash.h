#ifndef FLASH_H
#define FLASH_H

#include <string.h>
#include <stdint.h>

#define PAGESIZE   0x100

extern uint8_t flash_content[1024*1024*16];

//
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
class SPIFlash 
{
public:
  template <class T> bool writeAnything(uint32_t address, const T& value, bool errorCheck = true);
  template <class T> bool writeAnything(uint16_t page_number, uint8_t offset, const T& value, bool errorCheck = true);
  template <class T> bool readAnything(uint32_t address, T& value, bool fastRead = false);
  template <class T> bool readAnything(uint16_t page_number, uint8_t offset, T& value, bool fastRead = false);
  bool     eraseSector(uint32_t address);
  bool     eraseSector(uint16_t page_number, uint8_t offset);
  bool     eraseChip(void);
  int error() { return 0; }
};

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
template <class T> bool SPIFlash::writeAnything(uint32_t address, const T& value, bool errorCheck)
{
  uint16_t maxBytes = PAGESIZE-(address % PAGESIZE);  // Force the first set of bytes to stay within the first page
  uint16_t length = sizeof(value);

    uint32_t writeBufSz;
    uint16_t data_offset = 0;
    const uint8_t* p = ((const uint8_t*)(const void*)&value);

    while (length > 0)
    {
      writeBufSz = (length<=maxBytes) ? length : maxBytes;

      for (uint16_t i = 0; i < writeBufSz; ++i) {
        flash_content[address++] = *p++;
      }
      data_offset += writeBufSz;
      length -= writeBufSz;
      maxBytes = 256;   // Now we can do up to 256 bytes per loop
    }
    return true;
}

template <class T> bool SPIFlash::writeAnything(uint16_t page_number, uint8_t offset, const T& value, bool errorCheck) 
{
  return writeAnything(page_number * PAGESIZE + offset, value, errorCheck);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
template <class T> bool SPIFlash::readAnything(uint32_t address, T& value, bool fastRead) 
{
    uint8_t* p = (uint8_t*)(void*)&value;
  for (uint16_t i = 0; i < sizeof(value); i++) {
    *p++ = flash_content[address++];
  }
  return true;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
template <class T> bool SPIFlash::readAnything(uint16_t page_number, uint8_t offset, T& value, bool fastRead)
{
  return readAnything(page_number * PAGESIZE + offset, value, fastRead);
}

extern SPIFlash flash;

#endif // FLASH_H_

