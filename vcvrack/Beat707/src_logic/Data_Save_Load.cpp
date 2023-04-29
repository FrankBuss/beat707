/*
 * 
 * Created by William Kalfelz @ Beat707 (c) 2018 - http://www.Beat707.com
 * 
 */
#include "hardware.h"
#include "declarations.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void checkIfDataNeedsSaving()
{
  if (somethingChangedPattern)
  {
    somethingChangedPattern = false;
    if (!configData.writeProtectFlash)
    {
      stopTimer(true);
      savePatternData(true);
      saveStepsData();
      startTimer(true);
    }
  }
  // -------=========---------- //
  if (somethingChangedConfig)
  {
    somethingChangedConfig = false;
    if (!configData.writeProtectFlash) saveConfigData(true);
  }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void eraseSector(uint16_t _pagePos)
{
#if 0
  #if !DISABLE_FLASH
    if (!flash.eraseSector(_pagePos, 0)) showErrorMsg(flash.error());  
  #endif
#endif
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void saveConfigData(byte _eraseSector)
{
#if 0
  #if !DISABLE_FLASH
    stopTimer(true);
    if (_eraseSector) eraseSector(16 + (currentPatternBank * ((16 * 16) + 16)));
    if (!flash.writeAnything     (16 + (currentPatternBank * ((16 * 16) + 16)), (uint8_t) 0, configData)) showErrorMsg(flash.error());  
    startTimer(true);
  #endif
#endif
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void savePatternData(byte _eraseSector)
{
#if 0
  #if !DISABLE_FLASH
    if (_eraseSector) eraseSector(16 + (currentPatternBank * ((16 * 16) + 16)) + (currentPattern * 16) + 16);
    if (!flash.writeAnything     (16 + (currentPatternBank * ((16 * 16) + 16)) + (currentPattern * 16) + 16, (uint8_t) 0, patternData)) showErrorMsg(flash.error());   
  #endif
#endif
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void saveStepsData()
{
#if 0
  #if !DISABLE_FLASH
    if (!flash.writeAnything     (16 + 1 + (currentPatternBank * ((16 * 16) + 16)) + (currentPattern * 16) + 16, (uint8_t) 0, stepsData)) showErrorMsg(flash.error());
  #endif
#endif
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void loadPattern(byte pattern, bool force)
{
#if 0
  checkIfDataNeedsSaving();
  //
  if (!seqPlaying || force)
  {
    ignoreButtons = true;
    //
    #if !DISABLE_FLASH
      patternPagePos = ((currentPatternBank * ((16 * 16) + 16)) + 16) + (pattern * 16) + 16;
      if (!flash.readAnything(patternPagePos, (uint8_t) 0, patternData)) showErrorMsg(flash.error());
      patternPagePos++;
      if (!flash.readAnything(patternPagePos, (uint8_t) 0, stepsData)) showErrorMsg(flash.error());
    #endif
    //
    currentPattern = pattern;
    #if !DISABLE_FLASH
      if (currentPatternBank != nextPatternBank) if (!flash.readAnything((16 + (currentPatternBank * ((16 * 16) + 16))), (uint8_t) 0, configData)) showErrorMsg(flash.error()); 
    #endif
    currentPatternBank = nextPatternBank;
    patternBitsSelector = 0;
    //
    if (isSelectingBank) bitSet(patternBitsSelector, nextPatternBank);   
      else bitSet(patternBitsSelector, currentPattern);   
    //
    //
    ignoreButtons = false;
  }
  else
  {
    streamNextPattern = true;
    ignoreButtons = true;
  }
#endif
}

/*
 * 
 * Created by William Kalfelz @ Beat707 (c) 2018 - http://www.Beat707.com
 * 
 * Flash Page Size = 256 bytes
 * Capacity =  8388608 bytes
 * Number of Pages = 32768
 * 
 * Config: 1 page
 * PatternBank: 1 page
 * Pattern Size: 2 pages
 * 
 * Page 0 is just the B707 header
 * Page 1 starts the data
 *                      Pg1    Config
 *                      Pg2    PatternBank
 *                      Pg3    Pattern 01
 *                      Pg4    Pattern 01 (continued)
 *                                  ~
 *                             Pattern 64
 * Total of 130 pages per patternBank
 * The problem is that the W25Q64 IC only erases pages in groups of 16. So we end up with only 2048 pages.
 * So we need a full 16 pages for the patternBank data + config, them another 16 pages per pattern. Total of 1040 pages per patternBank.
 * 
 * ----------------------------------------------------------------------
 * New format
 * We have 32768 pages, so with 2 bytes (16 bits) we can store a full page address
 * 
 */

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void flashInit(bool force)
{
#if 0
  #if !DISABLE_FLASH
    totalFlashErrors = 0;
    if (!flash.begin(FLASH_CHIPSIZE)) { showErrorMsg(127); }
    waitMs(120);
    if (!flash.readAnything(0, (uint8_t) 0, flashHeader)) showErrorMsg(flash.error());
    //
    if (flashHeader[0] != 'B' || flashHeader[1] != '7' || flashHeader[2] != '0' || flashHeader[3] != '7' || flashHeader[4] != 'V' || flashHeader[5] != FLASH_VERSION) showErrorMsg(99);
    //
    #if INIT_FLASH_MEMORY
      force = true;
    #endif
    //
    if (force)
    {
      stopTimer(false);
      showWaitMsg(-1);
      waitMs(2000);
      #if INIT_ENTIRE_FLASH
        bool sectorErase = false;
        if (!flash.eraseChip()) sectorErase = true;
      #else
        bool sectorErase = true;
      #endif
      int porc = 0;
      //
      // Start Saving PatternBanks/Patterns //
      for (byte x = 0; x < PT_BANKS; x++) // patternBanks
      {
        initPatternBank(x, sectorErase, porc, false);
      }
      // Reset the Init Area //
      pagePos = 16 + (PT_BANKS * ((16 * 16) + 16)) + 16;
      if (sectorErase) eraseSector(pagePos);
      if (!flash.writeAnything(pagePos, (uint8_t) 0, patternData)) showErrorMsg(flash.error());
      pagePos++;
      if (!flash.writeAnything(pagePos, (uint8_t) 0, stepsData)) showErrorMsg(flash.error());
      //
      // Reset the Copy/Paste Area //
      pagePos = 16 + (PT_BANKS * ((16 * 16) + 16));
      if (sectorErase) eraseSector(pagePos);
      if (!flash.writeAnything(pagePos, (uint8_t) 0, patternData)) showErrorMsg(flash.error());
      pagePos++;
      if (!flash.writeAnything(pagePos, (uint8_t) 0, stepsData)) showErrorMsg(flash.error());
      //
      // Finish //
      showWaitMsg(-1);
      saveHeader(sectorErase);
      //
      #if DEBUG_SERIAL
        Serial.println(flashHeader);
      #endif  
      //    
      if (!flash.readAnything(0, (uint8_t) 0, flashHeader)) showErrorMsg(flash.error());
      if (flashHeader[0] != 'B' || flashHeader[1] != '7' || flashHeader[2] != '0' || flashHeader[3] != '7' || flashHeader[4] != 'V' || flashHeader[5] != FLASH_VERSION) showErrorMsg(98);
      //
      if (totalFlashErrors > 0)
      {
        showErrorMsg(totalFlashErrors, true);
        waitMs(4000);
        totalFlashErrors = 0;
      }
      //
      startTimer(false);
    }
  #endif
#endif
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void saveHeader(bool sectorErase)
{
#if 0
  createFlashHeader();
  //
  #if !DISABLE_FLASH
    if (sectorErase) eraseSector(0);
    if (!flash.writeAnything(0, (uint8_t) 0, flashHeader)) showErrorMsg(flash.error());
  #endif
#endif
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void createFlashHeader()
{
  flashHeader[0] = 'B';
  flashHeader[1] = '7';
  flashHeader[2] = '0';
  flashHeader[3] = '7';
  flashHeader[4] = 'V';
  flashHeader[5] = FLASH_VERSION;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void initPatternBank(byte patternBank, bool sectorErase, int &porc, bool patternBankOnly)
{
#if 0
  pagePos = 16 + (patternBank * ((16 * 16) + 16));
  //
  #if !DISABLE_FLASH
    if (sectorErase) eraseSector(pagePos);
    if (!flash.writeAnything(pagePos, (uint8_t) 0, configData)) showErrorMsg(flash.error());
    pagePos += 16;
    //
    for (byte p = 0; p < PATTERNS; p ++)
    {
      if (sectorErase) eraseSector(pagePos);
      if (!flash.writeAnything(pagePos, (uint8_t) 0, patternData)) showErrorMsg(flash.error());
      pagePos++;
      if (!flash.writeAnything(pagePos, (uint8_t) 0, stepsData)) showErrorMsg(flash.error());
      pagePos += 15;
      porc++;
      if (patternBankOnly) showWaitMsg(porc * 2); else showWaitMsg(byte(porc / 20));
    } 
  #endif
#endif
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void checkPatternStream()
{
  if (loadPatternNow)
  {
    loadPattern(nextPattern, true);
    loadPatternNow = false;
    streamNextPattern = false;
    ignoreButtons = false;
    resetProgramChangeAndCC();
  }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void loadPatternBank(byte patternBank)
{
#if 0
  currentPattern = nextPattern = 0;
  currentPatternBank = nextPatternBank = patternBank;
  loadPattern(0);
  //
  #if !DISABLE_FLASH
    if (!flash.readAnything((16 + (currentPatternBank * ((16 * 16) + 16))), (uint8_t) 0, configData)) showErrorMsg(flash.error()); 
  #endif
#endif
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void changePattern(char thedirection)
{
  if (thedirection > 0)
  {
    if (currentPattern < (PATTERNS - 1)) nextPattern = currentPattern + 1;
    else
    {
      if (currentPatternBank < PT_BANKS)
      {
        nextPattern = 0;
        nextPatternBank++;
      }
    }
  }
  else
  {
    if (currentPattern > 0) nextPattern = currentPattern - 1;
    else
    {
      if (currentPatternBank > 0)
      {
        nextPattern = PATTERNS - 1;
        nextPatternBank--;        
      }
    }
  }
  //
  if (currentPattern != nextPattern || nextPatternBank != currentPatternBank) 
  {
    loadPattern(nextPattern);
    //
    if (repeatMode != kRepeatModeNormal)
    {
      repeatMode = kRepeatModeNormal;
      ShowTemporaryMessage(kPatternRepeatMessage);   
    }
  }
}
