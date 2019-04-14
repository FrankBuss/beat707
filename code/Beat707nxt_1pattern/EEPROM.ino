/*
 * 
 * Created by William Kalfelz @ Beat707 (c) 2019 - http://www.Beat707.com
 * 
 */

#if EEPROM_STORAGE

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#define DEBUG_EEPROM 0
#define DEBUG_EEPROM_RAW 0
#define DEBUG_EEPROM_WAIT_MS 3000
#define EEPROM_Version 1
#define EEPROM_SHOW_INITIAL_WAIT_MESSAGE 1
#define EEPROM_SHOW_WAIT_MESSAGE 0
#define EEPROM_SHOW_MEM_LEFT 1
#define EEPROM_SHOW_MEM_LEFT_DELAY 4000
#define EEPROM_CONFIG_START 4
#define EEPROM_CONFIG_SIZE sizeof(WCONFIG)
#define EEPROM_PATTERN_START (EEPROM_CONFIG_START + EEPROM_CONFIG_SIZE)
#define EEPROM_STEPS_START (EEPROM_PATTERN_START + sizeof(WPATTERN))

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void EEPROM_Write_IFD(int address, byte value, bool outputValueToScreen = false)
{
  #if DEBUG_EEPROM
    if (outputValueToScreen)
    {
      printNumber(1, 0, address);
      printNumber(2, 0, value);
      sendScreenAndWait(DEBUG_EEPROM_WAIT_MS);
    } 
  #endif
  //
  if (address >= 1024) return;
  if (EEPROM.read(address) != value) EEPROM.write(address, value);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void EEPROM_Config_Read()
{
  if (EEPROM.read(0) == 'n' && EEPROM.read(1) == 'x' && EEPROM.read(2) == 't')
  {
    pointerData = (byte*) &configData;
    for (byte x = 0; x < sizeof(WCONFIG); x++)
    {
      pointerData[x] = EEPROM.read(x + EEPROM_CONFIG_START);
    }
  } 
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void EEPROM_Config_Write()
{
  EEPROM_Write_IFD(0, 'n');
  EEPROM_Write_IFD(1, 'x');
  EEPROM_Write_IFD(2, 't');
  EEPROM_Write_IFD(3, EEPROM_Version);
  //
  pointerData = (byte*) &configData;
  for (byte x = 0; x < sizeof(WCONFIG); x++)
  {
    EEPROM_Write_IFD(x + EEPROM_CONFIG_START, pointerData[x]);
  }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void EEPROM_Pattern_Read()
{
  if (EEPROM.read(0) == 'n' && EEPROM.read(1) == 'x' && EEPROM.read(2) == 't')
  {
    #if DEBUG_EEPROM_RAW
      int eepromPosDB = EEPROM_STEPS_START;
      while (1)
      {
        byte nDataDB = EEPROM.read(eepromPosDB);
        printNumber(1, 0, eepromPosDB);
        printNumber(2, 0, nDataDB);
        sendScreenAndWait(DEBUG_EEPROM_WAIT_MS);           
        eepromPosDB++;
        if (nDataDB == 0xFF) break;
      }
      //
      printNumber(1, 0, 0);
      printNumber(2, 0, 0);
      sendScreenAndWait(DEBUG_EEPROM_WAIT_MS * 4);                
    #endif
    //
    pointerData = (byte*) &patternData;
    for (byte x = 0; x < sizeof(WPATTERN); x++)
    {
      pointerData[x] = EEPROM.read(x + EEPROM_PATTERN_START);
    }
    //
    pointerData = (byte*) &stepsData;
    int dataPos = 0;
    int eepromPos = EEPROM_STEPS_START;
    //
    while (eepromPos < 1024 && dataPos < (sizeof(WSTEPS)*STEPS))
    {
      byte nData = EEPROM.read(eepromPos);
      eepromPos++;
      //
      if (nData == 0xFF) 
      {
        return;
      }
      else if (nData == 0xFE) // Equal N Datas
      {
        nData = EEPROM.read(eepromPos);
        eepromPos++;
        //
        while (1)
        {
          byte nTimes = EEPROM.read(eepromPos);
          eepromPos++;
          if (nTimes == 0xFF) break;
          //
          for (byte dD = 0; dD < nTimes; dD++)
          {
            pointerData[dataPos] = nData;
            dataPos++;
          }
        }
      }
      else if (nData <= 0x7F) // Zeros
      {
        #if DEBUG_EEPROM
          printNumber(1, 0, 990);
          printNumber(2, 0, nData);
          sendScreenAndWait(DEBUG_EEPROM_WAIT_MS);     
        #endif
        //      
        if (nData == 0) dataPos++; else dataPos += nData;
      }
      else
      {
        nData &= 0x7F;
        //
        #if DEBUG_EEPROM
          printNumber(1, 0, 999);
          printNumber(2, 0, nData);
          sendScreenAndWait(DEBUG_EEPROM_WAIT_MS);     
        #endif        
        //
        for (byte dD = 0; dD < nData; dD++)
        {
          pointerData[dataPos] = EEPROM.read(eepromPos);
          eepromPos++;
          //
          #if DEBUG_EEPROM
            printNumber(1, 0, 994);
            printNumber(2, 0, pointerData[dataPos]);
            sendScreenAndWait(DEBUG_EEPROM_WAIT_MS);     
          #endif                  
          //
          dataPos++;
          //
          if (eepromPos >= 1024 || dataPos >= (sizeof(WSTEPS)*STEPS)) return;
        }
      }
    }
  }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void EEPROM_Internal_CompressZeros(int& address, int zeros)
{
  #if DEBUG_EEPROM
    printNumber(1, 0, 555);
    printNumber(2, 0, zeros);
    sendScreenAndWait(DEBUG_EEPROM_WAIT_MS);
  #endif
  //
  if (zeros == 0)
  {
      EEPROM_Write_IFD(address, 0, true);
      address++;   
  }
  else
  {
    while (zeros > 0 && address < 1020)
    {
      byte saveZeros = 127;
      if (zeros < 127) saveZeros = zeros;
      //
      EEPROM_Write_IFD(address, saveZeros, true);
      address++;
      zeros -= 127;
    }
  }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void EEPROM_Internal_CompressNonZeroData(int& address, int dataLen, int dataPos)
{
  dataPos -= dataLen;
  //
  // Check If All Data is Not The Same //
  // Before, full Track 1 and Track 2 = Left 802 - after new code, Left 818 (success!)
  if (dataLen > 10)
  {
    bool allTheSame = true;
    for (int xdata = 1; xdata < dataLen; xdata++)
    {
      if (pointerData[0] != pointerData[xdata])
      {
        allTheSame = false;
        break;
      }
    }
    //
    if (allTheSame)
    {
      EEPROM_Write_IFD(address, 0xFE, true);
      address++;
      EEPROM_Write_IFD(address, pointerData[dataPos], true);
      address++;
      //
      while (dataLen > 0 && address < 1020)
      {
        byte saveDatas = 254;
        if (dataLen < 254) saveDatas = dataLen;
        //
        EEPROM_Write_IFD(address, saveDatas, true);
        address++;
        //
        dataLen -= 254;
      }
      //
      EEPROM_Write_IFD(address, 0xFF, true);
      address++;
      //
      return;
    }
  }
  //
  // Write Data Normally //
  while (dataLen > 0 && address < 1020)
  {
    #if DEBUG_EEPROM
      printNumber(1, 0, 222);
      printNumber(2, 0, dataLen);
      sendScreenAndWait(DEBUG_EEPROM_WAIT_MS);
    #endif
    //
    byte saveDatas = 125;
    if (dataLen < 125) saveDatas = dataLen;
    //
    EEPROM_Write_IFD(address, saveDatas | 0x80, true);
    address++;
    //
    for (byte wx = 0; wx < saveDatas; wx++)
    {
      if (address > 1020) return;
      //
      EEPROM_Write_IFD(address, pointerData[dataPos], true);
      address++;
      dataPos++;
    }
    //
    dataLen -= 125;
  }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void EEPROM_Pattern_Write()
{
  if (!patternHasChanges) return;
  patternHasChanges = false;
  //
  #if !DEBUG_EEPROM & EEPROM_SHOW_WAIT_MESSAGE
    showWaitMsg(0);
  #endif
  #if EEPROM_SHOW_INITIAL_WAIT_MESSAGE
    showWaitMsg(0);
  #endif
  //
  pointerData = (byte*) &patternData;
  for (byte x = 0; x < sizeof(WPATTERN); x++)
  {
    EEPROM_Write_IFD(x + EEPROM_PATTERN_START, pointerData[x]);
  }
  //
  pointerData = (byte*) &stepsData;
  int eepromPos = EEPROM_STEPS_START;
  int thePosition = 0;
  //
  int zerosCounter = 0;
  byte nonZeroCounter = 0;
  float percTotal = 100.0f / float((sizeof(WSTEPS)*STEPS));
  //
  while (thePosition < (sizeof(WSTEPS)*STEPS) && eepromPos < 1022)
  {
    if (pointerData[thePosition] == 0)
    {
      if (nonZeroCounter != 0) 
      {
        EEPROM_Internal_CompressNonZeroData(eepromPos, nonZeroCounter, thePosition);
        nonZeroCounter = 0;
      }
      zerosCounter++;
    }
    else 
    {
      if (zerosCounter != 0) 
      {
        EEPROM_Internal_CompressZeros(eepromPos, zerosCounter);
        zerosCounter = 0;
      }
      nonZeroCounter++;
    }
    //
    thePosition++;
    //
    #if !DEBUG_EEPROM & EEPROM_SHOW_WAIT_MESSAGE
      showWaitMsg(percTotal * thePosition);
    #endif
  }
  //
  if (nonZeroCounter != 0) EEPROM_Internal_CompressNonZeroData(eepromPos, nonZeroCounter, thePosition);
  //
  if (eepromPos < 1024) EEPROM_Write_IFD(eepromPos, 0xFF, true);
  eepromPos++;
  //
  #if EEPROM_SHOW_MEM_LEFT
    memset(segments, 0, sizeof(segments));
    segments[2][0] = S_N;
    segments[2][1] = S_N;
    segments[2][2] = S_E;
    segments[2][3] = S_N;
    segments[2][4] = S_N;
    //
    if (eepromPos > 1024) printNumber(2, 5, 0);
      else printNumber(2, 5, 1024 - eepromPos);
    sendScreenAndWait(EEPROM_SHOW_MEM_LEFT_DELAY);
  #endif
}
 
#endif
