/* 
 *  
 * Arduino SPIFlash Library v.2.6.0
 * Copyright (C) 2018 by Prajwal Bhattaram
 * 
 */

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#ifndef SPIFLASH_H
#define SPIFLASH_H
#include "SPI.h"
#include <EEPROM.h>
//
// Flash SS Pin is Hardcoded to Digital Pin# 10 //
//
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#define FLASH_SS_HIGH PORTB |= B00000100
#define FLASH_SS_LOW  PORTB &= B11111011
//
#define MANID        0x90
#define PAGEPROG     0x02
#define READDATA     0x03
#define FASTREAD     0x0B
#define WRITEDISABLE 0x04
#define READSTAT1    0x05
#define READSTAT2    0x35
#define WRITESTAT    0x01
#define WRITEENABLE  0x06
#define SECTORERASE  0x20
#define BLOCK32ERASE 0x52
#define CHIPERASE    0xC7
#define SUSPEND      0x75
#define ID           0x90
#define RESUME       0x7A
#define JEDECID      0x9F
#define RELEASE      0xAB
#define POWERDOWN    0xB9
#define BLOCK64ERASE 0xD8
#define READSFDP     0x5A
//
#define B8            8L
#define B64           64L
#define B16           16L
#define KB8           B8 * K
#define MB64          B64 * M
#define MB16          B16 * M
//
#define WINBOND_MANID    0xEF
#define PAGESIZE   0x100
//
#define MICROCHIP_MANID    0xBF
//
#define BUSY          0x01
#define SPI_CLK       104000000       //Hex equivalent of 104MHz
#define WRTEN         0x02
#define SUS           0x80
#define WSE           0x04
#define WSP           0x08
#define DUMMYBYTE     0xEE
#define NULLBYTE      0x00
#define NULLINT       0x0000
#define NO_CONTINUE   0x00
#define PASS          0x01
#define FAIL          0x00
#define NOOVERFLOW    false
#define NOERRCHK      false
#define VERBOSE       true
#if defined (SIMBLEE)
#define BUSY_TIMEOUT  100L
#else
#define BUSY_TIMEOUT  10L
#endif
#define arrayLen(x)   (sizeof(x) / sizeof(*x))
#define lengthOf(x)   (sizeof(x))/sizeof(byte)
#define K             1024L
#define M             K * K
#define S             1000L
//
#define CS SS
//
#define SUCCESS      0x00
#define CALLBEGIN    0x01
#define UNKNOWNCHIP  0x02
#define UNKNOWNCAP   0x03
#define CHIPBUSY     0x04
#define OUTOFBOUNDS  0x05
#define CANTENWRITE  0x06
#define PREVWRITTEN  0x07
#define LOWRAM       0x08
#define SYSSUSPEND   0x09
#define UNSUPPORTED  0x0A //10
#define ERRORCHKFAIL 0x0B //11
#define NORESPONSE   0x0C //12
#define UNKNOWNERROR 0xFE //13

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#define CHIP_SELECT   FLASH_SS_LOW;
#define CHIP_DESELECT FLASH_SS_HIGH;
#define xfer(n)   WSPI.transfer(n)
#define BEGIN_SPI WSPI.begin();
//
#define LIBVER 2
#define LIBSUBVER 6
#define BUGFIXVER 0
//
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
class SPIFlash 
{
public:
  SPIFlash(uint8_t cs = CS, bool overflow = true);
  bool     begin(uint32_t _chipSize = 0);
  void     setClock(uint32_t clockSpeed);
  bool     libver(uint8_t *b1, uint8_t *b2, uint8_t *b3);
  uint8_t  error(bool verbosity = false);
  uint16_t getManID(void);
  uint32_t getJEDECID(void);
  bool     getAddress(uint16_t size, uint16_t &page_number, uint8_t &offset);
  uint32_t getAddress(uint16_t size);
  template <class T> bool writeAnything(uint32_t address, const T& value, bool errorCheck = true);
  template <class T> bool writeAnything(uint16_t page_number, uint8_t offset, const T& value, bool errorCheck = true);
  template <class T> bool readAnything(uint32_t address, T& value, bool fastRead = false);
  template <class T> bool readAnything(uint16_t page_number, uint8_t offset, T& value, bool fastRead = false);
  bool     eraseSector(uint32_t address);
  bool     eraseSector(uint16_t page_number, uint8_t offset);
  bool     eraseChip(void);
  //
  void     _troubleshoot(void);
  void     _printErrorCode(void);
  void     _printSupportLink(void);
  void     _endSPI(void);
  bool     _prep(uint8_t opcode, uint32_t address, uint32_t size);
  bool     _prep(uint8_t opcode, uint32_t page_number, uint8_t offset, uint32_t size);
  bool     _startSPIBus(void);
  bool     _beginSPI(uint8_t opcode);
  bool     _notBusy(uint32_t timeout = BUSY_TIMEOUT);
  bool     _notPrevWritten(uint32_t address, uint32_t size = 1);
  bool     _writeEnable(uint32_t timeout = 10L);
  bool     _writeDisable(void);
  bool     _getJedecId(void);
  bool     _getManId(uint8_t *b1, uint8_t *b2);
  bool     _getSFDP(void);
  bool     _chipID(void);
  bool     _transferAddress(void);
  bool     _addressCheck(uint32_t address, uint32_t size = 1);
  uint8_t  _nextByte(uint8_t data = NULLBYTE);
  uint16_t _nextInt(uint16_t = NULLINT);
  void     _nextBuf(uint8_t opcode, uint8_t *data_buffer, uint32_t size);
  uint8_t  _readStat1(void);
  uint32_t _getAddress(uint16_t page_number, uint8_t offset = 0);
  template <class T> bool _writeErrorCheck(uint32_t address, const T& value);
  //
  #ifdef SPI_HAS_TRANSACTION
    SPISettings _settings;
  #endif
  bool        pageOverflow, SPIBusState;
  uint8_t     errorcode, state, _SPCR, _SPSR;
  struct      chipID 
  {
    uint8_t manufacturerID;
  };
  chipID _chip;
  uint32_t    currentAddress, _currentAddress = 0;
};

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
template <class T> bool SPIFlash::writeAnything(uint32_t address, const T& value, bool errorCheck)
{
  if (!_prep(PAGEPROG, address, sizeof(value))) {
    return false;
  }
  uint16_t maxBytes = PAGESIZE-(address % PAGESIZE);  // Force the first set of bytes to stay within the first page
  uint16_t length = sizeof(value);

    uint32_t writeBufSz;
    uint16_t data_offset = 0;
    const uint8_t* p = ((const uint8_t*)(const void*)&value);

    if (!SPIBusState) {
      _startSPIBus();
    }
    while (length > 0)
    {
      writeBufSz = (length<=maxBytes) ? length : maxBytes;

      if(!_notBusy() || !_writeEnable()){
        return false;
      }

      CHIP_SELECT
      (void)xfer(PAGEPROG);
      _transferAddress();

      for (uint16_t i = 0; i < writeBufSz; ++i) {
        _nextByte(*p++);
      }
      _currentAddress += writeBufSz;
      data_offset += writeBufSz;
      length -= writeBufSz;
      maxBytes = 256;   // Now we can do up to 256 bytes per loop
      CHIP_DESELECT
    }

  if (!errorCheck) {
    _endSPI();
    return true;
  }
  else {
    return _writeErrorCheck(address, value);
  }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
template <class T> bool SPIFlash::writeAnything(uint16_t page_number, uint8_t offset, const T& value, bool errorCheck) 
{
  uint32_t address = _getAddress(page_number, offset);
  return writeAnything(address, value, errorCheck);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
template <class T> bool SPIFlash::readAnything(uint32_t address, T& value, bool fastRead) 
{
  if (!_prep(READDATA, address, sizeof(value)))
    return false;

    uint8_t* p = (uint8_t*)(void*)&value;
    if(!fastRead)
      _beginSPI(READDATA);
    else
      _beginSPI(FASTREAD);
  for (uint16_t i = 0; i < sizeof(value); i++) {
    *p++ =_nextByte();
  }
  _endSPI();
  return true;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
template <class T> bool SPIFlash::readAnything(uint16_t page_number, uint8_t offset, T& value, bool fastRead)
{
  uint32_t address = _getAddress(page_number, offset);
  return readAnything(address, value, fastRead);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
template <class T> bool SPIFlash::_writeErrorCheck(uint32_t address, const T& value) 
{
  if (!_prep(READDATA, address, sizeof(value)) && !_notBusy()) {
    return false;
  }
  const uint8_t* p = (const uint8_t*)(const void*)&value;
  _beginSPI(READDATA);
  uint8_t _v;
  for(uint16_t i = 0; i < sizeof(value);i++)
  {
    if(*p++ != _nextByte())
    {
      errorcode = ERRORCHKFAIL;
      return false;
    }
  }
  _endSPI();
  return true;
}
#endif // _SPIFLASH_H_

