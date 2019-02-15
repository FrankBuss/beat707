/*
 * 
 * Created by William Kalfelz @ Beat707 (c) 2018 - http://www.Beat707.com
 * 
 */

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#define BUTTON_HOLD 240 // Max 255 for any value
#define BUTTON_CLICK 10
//
// The Following Values are Hardcoded using direct port manipulation //
//
//#define STROBE 8    //#define CLOCK 9   //#define SETIO1 5    //#define SETIO2 6    //#define SETIO3 7
//
#define CLOCK_LOW     PORTB &= B11111101
#define STROBE_LOW    PORTB &= B11111110
#define SETI123_LOW   PORTD &= B00011111
#define SETI01_LOW    PORTD &= B11011111
#define SETI02_LOW    PORTD &= B10111111
#define SETI03_LOW    PORTD &= B01111111
//
#define S_C_HIGH      PORTB |= B00000011
#define CLOCK_HIGH    PORTB |= B00000010
#define STROBE_HIGH   PORTB |= B00000001   
#define SETI123_HIGH  PORTD |= B11100000
#define SETI01_HIGH   PORTD |= B00100000
#define SETI02_HIGH   PORTD |= B01000000
#define SETI03_HIGH   PORTD |= B10000000
//
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void initTM1638()
{
  DDRB = B11101111; // PB4 is INPUT for MISO on Flash SPI //
  PORTB = B11111111; // Also set SS Flash HIGH //
  DDRD = B11111111;
  //
  S_C_HIGH;
  SETI123_LOW;
  //
  sendData(0x8F); // Init
  //
  sendData(0x40); // set auto increment mode
  STROBE_LOW;
  sendDataConst(0xc0, 0xc0, 0xc0); // Start Address
  SETI123_LOW;
  for(uint8_t i = 0; i < (16 * 8); i++)
  {
    CLOCK_LOW;
    waitMs(0);
    CLOCK_HIGH;
  }
  STROBE_HIGH;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void sendData(byte data)
{
  STROBE_LOW;
  for (byte x=0; x<8; x++)
  {
    CLOCK_LOW;
    if (bitRead(data, x) == 0) SETI123_LOW; else SETI123_HIGH;
    CLOCK_HIGH;
  }
  STROBE_HIGH;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void sendDataConst(byte data1, byte data2, byte data3)
{
  for (byte x=0; x<8; x++)
  {
    CLOCK_LOW;
    //
    PORTD &= B00011111;
    if (bitRead(data1, x) == 1) PORTD |= B00100000;
    if (bitRead(data2, x) == 1) PORTD |= B01000000;
    if (bitRead(data3, x) == 1) PORTD |= B10000000;
    //
    CLOCK_HIGH;
  }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void sendScreen()
{
  sendData(0x40); // set auto increment mode
  STROBE_LOW;
  sendDataConst(0xc0, 0xc0, 0xc0); // Start Address
  for (byte x=0; x<8; x++)
  {
    sendDataConst(segments[0][x], segments[1][x], segments[2][x]);
    sendDataConst(bitRead(leds[0],x), bitRead(leds[1],x), bitRead(leds[2],x));
  }
  STROBE_HIGH;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void readButtons(void)
{
  STROBE_LOW;
  sendDataConst(0x42, 0x42, 0x42); // Request Button States
  //
  DDRD = B00000000;
  //
  buttons[0] = buttons[1] = buttons[2] = 0x00;
  for (byte i = 0; i < 4; i++)
  {
    byte v[3] = { 0x00, 0x00, 0x00 };
    for (byte x=0; x<8; x++)
    {
      CLOCK_LOW;
      bitWrite(v[0], x, bitRead(PIND, 5));
      bitWrite(v[1], x, bitRead(PIND, 6));
      bitWrite(v[2], x, bitRead(PIND, 7));
      CLOCK_HIGH;
    }
    buttons[0] |= v[0] << i;
    buttons[1] |= v[1] << i;
    buttons[2] |= v[2] << i;
  }
  // 
  DDRD = B11111111;
  //
  STROBE_HIGH;
  //
  for (byte x=0; x < 8; x++)
  {
    for (byte i = 0; i < 3; i++)
    {
      if (bitRead(buttons[i], x) && (!ignoreButtons || (i == 0 && x <= 1 && curRightScreen != kMuteMenu) || (i == 2 && x == 7 && curRightScreen == kMuteMenu)))
      {
        if (buttonDownTime[i][x] < BUTTON_HOLD) buttonDownTime[i][x]++;
        if ((buttonDownTime[i][x] >= BUTTON_HOLD && !bitRead(buttonEventWasHolding[i], x)) ||
            (i == 0 && curRightScreen != kMuteMenu && (x >= 6 || x == 4) && buttonDownTime[i][x] > BUTTON_CLICK && !bitRead(buttonEventWasHolding[i],x)))
        {
          somethingClicked = false;
          bitSet(buttonEventWasHolding[i], x);
          if (i == 0 && curRightScreen != kMuteMenu && (x == 2 || x == 3))
          {
            buttonEvent[i][x] = kButtonClicked;
            bitClear(buttonEventWasHolding[i], x);
          }
          else 
          {
            buttonEvent[i][x] = kButtonHold;
            //
            if (i >= 1 && curRightScreen == kRightSteps && curTrack >= DRUM_TRACKS)
            {
              if (editingNoteTranspose == -127 && !forceAccent)
              {
                editingNoteTranspose = 0;
                noteTransposeWasChanged = false;
                updateScreen = true;
              }
            }
          }
          somethingHappened = true;
        }
      }
      else
      {
        if (buttonDownTime[i][x] > 0)
        {
          if ((buttonDownTime[i][x]) > BUTTON_CLICK)
          {
            if (forceAccent && x == 5 && i == 0) 
            {
              curRightScreen = kMuteMenu;
              forceAccent = false;
              ignoreNextButton = true;
            }
            else if (ignoreNextButton)
            {
              ignoreNextButton = false;
            }
            else
            {
              if ((buttonDownTime[i][x]) >= BUTTON_HOLD || bitRead(buttonEventWasHolding[i], x))
              {
                if (somethingClicked) buttonEvent[i][x] = kButtonRelease; else buttonEvent[i][x] = kButtonReleaseNothingClicked;
                if (buttonEvent[i][x] == kButtonReleaseNothingClicked && i == 0 && x == 6 && curRightScreen != kMuteMenu && buttonDownTime[i][x] >= BUTTON_HOLD) buttonEvent[i][x] = kButtonRelease;
                somethingHappened = true;
              }
              else 
              {
                if ((buttonDownTime[i][x]) < BUTTON_HOLD)
                {
                  buttonEvent[i][x] = kButtonClicked;
                  somethingHappened = true;
                }
                somethingClicked = true;
              }
            }
          }
        }
        buttonDownTime[i][x] = 0;
        bitClear(buttonEventWasHolding[i], x);
      }
    }
  }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void printNumber(byte segment, byte offset, int number)
{
  bool isPositive = true;
  if (number < 0) 
  {
    isPositive = false;
    number *= -1;
  }
  //
  int x = number / 100;
  segments[segment][offset] = (char)pgm_read_word(&numbers[0 + x]);
  number -= x * 100;
  //
  x = number / 10;
  segments[segment][offset + 1] = (char)pgm_read_word(&numbers[0 + x]);
  number -= x * 10;
  //
  segments[segment][offset + 2] = (char)pgm_read_word(&numbers[0 + number]);
  //
  if (!isPositive) segments[segment][offset] = B01000000;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#if SHOW_FREE_RAM
  extern unsigned int __data_start;
  extern unsigned int __data_end;
  extern unsigned int __bss_start;
  extern unsigned int __bss_end;
  extern unsigned int __heap_start;
  extern void *__brkval;
  
  void freeMemory(void)
  {
    int free_memory;
    if((int)__brkval == 0) free_memory = ((int)&free_memory) - ((int)&__bss_end);
      else free_memory = ((int)&free_memory) - ((int)__brkval);
    if (free_memory > 999)
    {
      int x = free_memory / 1000;
      segments[2][0] = (char)pgm_read_word(&numbers[0 + x]);
      free_memory -= x * 1000;
    }
    printNumber(2, 1, free_memory);
  }
#endif

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void showErrorMsg(byte error) { showErrorMsg(error, false); }
void showErrorMsg(byte error, bool errors)
{
  memset(segments, 0, sizeof(segments));
  segments[2][0] = S_E;
  segments[2][1] = S_r;
  segments[2][2] = S_r;
  segments[2][3] = S_o;
  segments[2][4] = S_r;
  printNumber(2, 5, error);
  if (errors) segments[2][5] = S_S;
  sendScreen();
  if (totalFlashErrors < 0xFF) totalFlashErrors++;
  waitMs(2000);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void showWaitMsg(char porcentage)
{
  memset(segments, 0, sizeof(segments));
  segments[2][0] = 0x3c;
  segments[2][1] = 0x1e;
  segments[2][2] = S_A;
  segments[2][3] = S_I;
  segments[2][4] = S_T;
  if (porcentage >=0) printNumber(2, 5, porcentage);
  sendScreen();
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void printDashDash(byte segment, byte offset)
{
  segments[segment][offset] = S_DASH;
  segments[segment][offset + 1] = S_DASH;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void printMIDInote(byte note, byte segment, byte offset, byte offsetOctave)
{
  byte xNote = note - ((note / 12) * 12);
  switch (xNote)
  {
    case 0: segments[segment][offset] = S_C;                                         break;
    case 1: segments[segment][offset] = S_C;  segments[segment][offset + 1] = S_H;   break;
    case 2: segments[segment][offset] = S_d;                                         break;
    case 3: segments[segment][offset] = S_d;  segments[segment][offset + 1] = S_H;   break;
    case 4: segments[segment][offset] = S_E;                                         break;
    case 5: segments[segment][offset] = S_F;                                         break;
    case 6: segments[segment][offset] = S_F;  segments[segment][offset + 1] = S_H;   break;
    case 7: segments[segment][offset] = S_G;                                         break;
    case 8: segments[segment][offset] = S_G;  segments[segment][offset + 1] = S_H;   break;
    case 9: segments[segment][offset] = S_A;                                         break;
    case 10: segments[segment][offset] = S_A; segments[segment][offset + 1] = S_H;   break;
    case 11: segments[segment][offset] = S_b;                                        break;
  }
  //
  byte xn = (note / 12);
  if (xn >= 10) xn = 9;
  segments[segment][offsetOctave] = (char)pgm_read_word(&numbers[xn]);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void sendScreenAndWait(int wait)
{
  sendScreen();
  waitMs(wait); 
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void splashScreen()
{
  #if INTRO_ANIMATION
    randomSeed(analogRead(0));
    uint32_t doneCrazy = 0x00;
    while (doneCrazy != 0xFFFFFF)
    {
      for (int xs = 0; xs < 8; xs++)
      {
        segments[0][xs] = random(0xFF);
        segments[1][xs] = random(0xFF);
        segments[2][xs] = random(0xFF);
      }
      //
      leds[0] = random(0xFF);
      leds[1] = random(0xFF);
      leds[2] = random(0xFF);
      //
      for (int xs = 0; xs < 8; xs++)
      {
        if (bitRead(doneCrazy, xs) == 1) 
        {
          segments[0][xs] = 0x00;
          bitClear(leds[0], xs);
        }
        if (bitRead(doneCrazy, xs + 8) == 1) 
        {
          segments[1][xs] = 0x00;
          bitClear(leds[1], xs);
        }
        if (bitRead(doneCrazy, xs + 16) == 1) 
        {
          segments[2][xs] = 0x00;
          bitClear(leds[2], xs);
        }
      }
      //
      sendScreenAndWait(28);
      //
      bitSet(doneCrazy, random(24));
    }
    //
    memset(segments, 0, sizeof(segments));
    leds[0] = leds[1] = leds[2] = 0x00;
    sendScreenAndWait(888);
    //
    segments[0][0] = (char)pgm_read_word(&numbers[2]);
    sendScreenAndWait(100);
    segments[0][1] = S_O;
    sendScreenAndWait(100);
    segments[0][2] = S_N;
    sendScreenAndWait(100);
    segments[0][3] = S_E;
    sendScreenAndWait(100);
    segments[0][4] = S_S;
    sendScreenAndWait(100);
    segments[0][5] = S_T;
    sendScreenAndWait(100);
    segments[0][6] = S_E;
    sendScreenAndWait(100);
    segments[0][7] = S_P;
    sendScreenAndWait(800);
    //
    segments[1][0] = S_U;
    sendScreenAndWait(100);
    segments[1][1] = S_E;
    sendScreenAndWait(100);
    segments[1][2] = S_R;
    sendScreenAndWait(100);
    segments[1][3] = S_S;
    sendScreenAndWait(100);
    //
    segments[1][5] = (char)pgm_read_word(&numbers[1]);
    sendScreenAndWait(100);
    segments[1][5] |= S_DOT;
    segments[1][6] = (char)pgm_read_word(&numbers[0]);
    sendScreenAndWait(100);
    segments[1][6] |= S_DOT;
    segments[1][7] = (char)pgm_read_word(&numbers[8]);
    sendScreenAndWait(800);
    //
    segments[2][1] = S_W1;
    sendScreenAndWait(100);
    segments[2][2] = S_W2;
    sendScreenAndWait(100);
    segments[2][3] = S_U;
    sendScreenAndWait(100);
    segments[2][4] = S_S;
    sendScreenAndWait(100);
    segments[2][5] = S_I;
    sendScreenAndWait(100);
    segments[2][6] = S_K;
    sendScreenAndWait(800);
    //
    for (int xs = 0; xs < 24; xs++)
    {
      leds[0] = leds[1] = leds[2] = 0;
      //
      if (xs <= 7) bitSet(leds[0], xs);
      else if (xs <= 15) bitSet(leds[1], xs - 8);
      else bitSet(leds[2], xs - 16);
      //
      sendScreenAndWait(240);    
    }
    //
    leds[0] = leds[1] = leds[2] = 0;
    sendScreen();
    waitMs(4888);
    memset(segments, 0, sizeof(segments));
    leds[0] = leds[1] = leds[2] = 0;
    sendScreen();
    waitMs(500);  
  #endif
}
