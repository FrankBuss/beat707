/* 
 *  
 * Arduino SPIFlash Library v.2.6.0
 * Copyright (C) 2018 by Prajwal Bhattaram
 * 
 */

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
SPIFlash::SPIFlash(uint8_t cs, bool overflow) 
{
  pageOverflow = overflow;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool SPIFlash::_prep(uint8_t opcode, uint32_t address, uint32_t size) 
{
  switch (opcode) {
    case PAGEPROG:
    if (!_addressCheck(address, size)) {
      return false;
    }
    if(!_notBusy() || !_writeEnable()){
      return false;
    }
    #ifndef HIGHSPEED
    if(!_notPrevWritten(address, size)) {
      return false;
    }
    #endif
    return true;
    break;

    default:
    if (!_addressCheck(address, size)) {
      return false;
    }
    if (!_notBusy()){
      return false;
    }
    return true;
    break;
  }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool SPIFlash::_prep(uint8_t opcode, uint32_t page_number, uint8_t offset, uint32_t size)
{
  uint32_t address = _getAddress(page_number, offset);
  return _prep(opcode, address, size);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool SPIFlash::_transferAddress(void) 
{
  _nextByte(_currentAddress >> 16);
  _nextByte(_currentAddress >> 8);
  _nextByte(_currentAddress);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool SPIFlash::_startSPIBus(void) 
{
#ifndef SPI_HAS_TRANSACTION
    noInterrupts();
#endif

  #if defined (ARDUINO_ARCH_AVR)
    //save current SPI settings
      _SPCR = SPCR;
      _SPSR = SPSR;
  #endif
  #ifdef SPI_HAS_TRANSACTION
    WSPI.beginTransaction(_settings);
  #else
    //WSPI.setClockDivider(SPI_CLOCK_DIV4);
    //WSPI.setDataMode(SPI_MODE0);
    //WSPI.setBitOrder(MSBFIRST);

    SPCR = _BV(SPE) | _BV(MSTR) | 0x00 | (SPI_MODE0 & SPI_MODE_MASK) | ((0x01 >> 1) & SPI_CLOCK_MASK);
    SPSR = 0x01 & SPI_2XCLOCK_MASK;

    
  #endif

  SPIBusState = true;
  return true;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool SPIFlash::_beginSPI(uint8_t opcode)
{
  if (!SPIBusState) {
    _startSPIBus();
  }
  CHIP_SELECT
  switch (opcode) {
    case FASTREAD:
    _nextByte(opcode);
    _nextByte(DUMMYBYTE);
    _transferAddress();
    break;

    case READDATA:
    _nextByte(opcode);
    _transferAddress();
    break;

    case PAGEPROG:
    _nextByte(opcode);
    _transferAddress();
    break;

    default:
    _nextByte(opcode);
    break;
  }
  return true;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t SPIFlash::_nextByte(uint8_t data) 
{
  return xfer(data);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint16_t SPIFlash::_nextInt(uint16_t data) 
{
  return WSPI.transfer16(data);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void SPIFlash::_nextBuf(uint8_t opcode, uint8_t *data_buffer, uint32_t size) 
{
  uint8_t *_dataAddr = &(*data_buffer);
  switch (opcode) {
    case READDATA:
    #if defined (ARDUINO_ARCH_SAM)
      _dueSPIRecByte(&(*data_buffer), size);
    #elif defined (ARDUINO_ARCH_AVR)
      WSPI.transfer(&data_buffer[0], size);
    #else
      for (uint16_t i = 0; i < size; i++) {
        *_dataAddr = xfer(NULLBYTE);
        _dataAddr++;
      }
      #endif
    break;

    case PAGEPROG:
    #if defined (ARDUINO_ARCH_SAM)
      _dueSPISendByte(&(*data_buffer), size);
    #elif defined (ARDUINO_ARCH_AVR)
      WSPI.transfer(&(*data_buffer), size);
    #else
      for (uint16_t i = 0; i < size; i++) {
        xfer(*_dataAddr);
        _dataAddr++;
      }
    #endif
    break;
  }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void SPIFlash::_endSPI(void) 
{
  CHIP_DESELECT
  #ifdef SPI_HAS_TRANSACTION
  WSPI.endTransaction();
  #else
  interrupts();
  #endif

  SPIBusState = false;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t SPIFlash::_readStat1(void) 
{
  _beginSPI(READSTAT1);
  uint8_t stat1 = _nextByte();
  CHIP_DESELECT
  return stat1;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool SPIFlash::_notBusy(uint32_t timeout)
{
  uint32_t startTime = millis();

  do {
    state = _readStat1();
    if((millis()-startTime) > timeout){
      errorcode = CHIPBUSY;
      _endSPI();
      return false;
    }
  } while(state & BUSY);
  return true;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool SPIFlash::_writeEnable(uint32_t timeout) 
{
  uint32_t startTime = millis();
  if (!(state & WRTEN)) {
    do {
      _beginSPI(WRITEENABLE);
      CHIP_DESELECT
      state = _readStat1();
      if((millis()-startTime) > timeout) {
        errorcode = CANTENWRITE;
        _endSPI();
        return false;
       }
     } while (!(state & WRTEN));
  }
  return true;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool SPIFlash::_writeDisable(void)
{
  _beginSPI(WRITEDISABLE);
  CHIP_DESELECT
  return true;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint32_t SPIFlash::_getAddress(uint16_t page_number, uint8_t offset)
{
  uint32_t address = page_number;
  return ((address << 8) + offset);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool SPIFlash::_getManId(uint8_t *b1, uint8_t *b2) 
{
  if(!_notBusy())
    return false;
  _beginSPI(MANID);
  _nextByte();
  _nextByte();
  _nextByte();
  *b1 = _nextByte();
  *b2 = _nextByte();
  CHIP_DESELECT
  return true;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool SPIFlash::_getJedecId(void) 
{
  if(!_notBusy()) {
    return false;
  }
  _beginSPI(JEDECID);
  _chip.manufacturerID = _nextByte(NULLBYTE);   // manufacturer id
  CHIP_DESELECT
  return true;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool SPIFlash::_getSFDP(void) 
{
  if(!_notBusy()) {
    return false;
  }
  _beginSPI(READSFDP);
  _currentAddress = 0x00;
  _transferAddress();
  _nextByte(DUMMYBYTE);
  CHIP_DESELECT
  return true;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool SPIFlash::_chipID(void) 
{
  //Get Manfucturer/Device ID so the library can identify the chip
  _getSFDP();
  if (!_getJedecId()) {
    return false;
  }
  //
  errorcode = UNKNOWNCHIP;    //Error code for unidentified chip
  //
  return true;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool SPIFlash::_addressCheck(uint32_t address, uint32_t size)
{
  if (errorcode == UNKNOWNCAP || errorcode == NORESPONSE) {
    return false;
  }
  for (uint32_t i = 0; i < size; i++) {
    if (false) { // address + i >= _chip.capacity) {
      if (!pageOverflow) {
        errorcode = OUTOFBOUNDS;
        return false;         // At end of memory - (!pageOverflow)
      }
      else {
        _currentAddress = 0x00;
        return true;          // At end of memory - (pageOverflow)
      }
    }
  }
  _currentAddress = address;
  return true;        // Not at end of memory if (address < _chip.capacity)
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool SPIFlash::_notPrevWritten(uint32_t address, uint32_t size) 
{
  _beginSPI(READDATA);
  for (uint16_t i = 0; i < size; i++) {
    if (_nextByte() != 0xFF) {
      CHIP_DESELECT;
      return false;
    }
  }
  CHIP_DESELECT
  return true;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool SPIFlash::begin(uint32_t _chipSize) 
{
  BEGIN_SPI
#ifdef SPI_HAS_TRANSACTION
  //Define the settings to be used by the SPI bus
  _settings = SPISettings(SPI_CLK, MSBFIRST, SPI_MODE0);
#endif
  if(!_chipID()) {
    return false;
  } 
  return true;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef SPI_HAS_TRANSACTION
  void SPIFlash::setClock(uint32_t clockSpeed) 
  {
    _settings = SPISettings(clockSpeed, MSBFIRST, SPI_MODE0);
  }
#endif

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t SPIFlash::error(bool _verbosity)
{
  if (!_verbosity) {
    return errorcode;
  }
  else {
    _troubleshoot();
    return errorcode;
  }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool SPIFlash::libver(uint8_t *b1, uint8_t *b2, uint8_t *b3) 
{
  *b1 = LIBVER;
  *b2 = LIBSUBVER;
  *b3 = BUGFIXVER;
  return true;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint16_t SPIFlash::getManID(void)
{
  uint8_t b1, b2;
    _getManId(&b1, &b2);
    uint32_t id = b1;
    id = (id << 8)|(b2 << 0);
    return id;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint32_t SPIFlash::getJEDECID(void)
{
    uint32_t id = _chip.manufacturerID;
    return id;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint32_t SPIFlash::getAddress(uint16_t size) 
{
  if (!_addressCheck(currentAddress, size))
  {
    errorcode = OUTOFBOUNDS;
    return false;
  }
  else {
    uint32_t address = currentAddress;
    currentAddress+=size;
    return address;
  }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool SPIFlash::getAddress(uint16_t size, uint16_t &page_number, uint8_t &offset)
{
  uint32_t address = getAddress(size);
  offset = (address >> 0);
  page_number = (address >> 8);
  return true;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool SPIFlash::eraseSector(uint32_t address) 
{
  if(!_notBusy()||!_writeEnable())
    return false;

  _beginSPI(SECTORERASE);
  _nextByte(address >> 16);
  _nextByte(address >> 8);
  _nextByte(0);
  _endSPI();

  if(!_notBusy(500L))
    return false; //Datasheet says erasing a sector takes 400ms max
    
  return true;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool SPIFlash::eraseSector(uint16_t page_number, uint8_t offset)
{
  uint32_t address = _getAddress(page_number, offset);
  return eraseSector(address);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool SPIFlash::eraseChip(void) 
{
  if(!_notBusy()||!_writeEnable())
    return false;

  _beginSPI(CHIPERASE);
  _endSPI();
  if(!_notBusy(100000L))
    return false; //Datasheet says erasing chip takes 100s max

  return true;
}

