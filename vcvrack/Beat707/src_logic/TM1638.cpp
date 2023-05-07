/*
 * 
 * Created by William Kalfelz @ Beat707 (c) 2018 - http://www.Beat707.com
 * 
 */

#include <stdio.h>
#include "declarations.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#define BUTTON_HOLD 240 // Max 255 for any value
#define BUTTON_CLICK 10
//
// The Following Values are Hardcoded using direct port manipulation //
//
//#define STROBE 8    //#define CLOCK 9   //#define SETIO1 5    //#define SETIO2 6    //#define SETIO3 7
//
#define CLOCK_LOW     PORTB &= 0B11111101
#define STROBE_LOW    PORTB &= 0B11111110
#define SETI123_LOW   PORTD &= 0B00011111
#define SETI01_LOW    PORTD &= 0B11011111
#define SETI02_LOW    PORTD &= 0B10111111
#define SETI03_LOW    PORTD &= 0B01111111
//
#define S_C_HIGH      PORTB |= 0B00000011
#define CLOCK_HIGH    PORTB |= 0B00000010
#define STROBE_HIGH   PORTB |= 0B00000001   
#define SETI123_HIGH  PORTD |= 0B11100000
#define SETI01_HIGH   PORTD |= 0B00100000
#define SETI02_HIGH   PORTD |= 0B01000000
#define SETI03_HIGH   PORTD |= 0B10000000
//
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void initTM1638()
{
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void sendData(byte data)
{
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void sendDataConst(byte data1, byte data2, byte data3)
{
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void readButtons(void)
{
  for (byte x=0; x < 8; x++)
  {
    for (byte i = 0; i < 4; i++)
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
  segments[segment][offset] = (char)numbers[0 + x];
  number -= x * 100;
  //
  x = number / 10;
  segments[segment][offset + 1] = (char)numbers[0 + x];
  number -= x * 10;
  //
  segments[segment][offset + 2] = (char)numbers[0 + number];
  //
  if (!isPositive) segments[segment][offset] = 0B01000000;
}

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
  segments[segment][offsetOctave] = (char)numbers[xn];
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
    uint32_t doneCrazy = 0x00;
    while (doneCrazy != 0xFFFFFF)
    {
      for (int xs = 0; xs < 8; xs++)
      {
        segments[0][xs] = random(0, 0xFF);
        segments[1][xs] = random(0, 0xFF);
        segments[2][xs] = random(0, 0xFF);
      }
      //
      leds[0] = random(0, 0xFF);
      leds[1] = random(0, 0xFF);
      leds[2] = random(0, 0xFF);
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
      bitSet(doneCrazy, random(0, 24));
    }
      printf("crazy done\n");
    //
    memset(segments, 0, sizeof(segments));
    leds[0] = leds[1] = leds[2] = 0x00;
    sendScreenAndWait(888);
    //
    segments[0][0] = (char)numbers[2];
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
    segments[1][5] = (char)numbers[1];
    sendScreenAndWait(100);
    segments[1][5] |= S_DOT;
    segments[1][6] = (char)numbers[0];
    sendScreenAndWait(100);
    segments[1][6] |= S_DOT;
    segments[1][7] = (char)numbers[8];
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
