/*
 * 
 * Created by William Kalfelz @ Beat707 (c) 2019 - http://www.Beat707.com
 * 
 */

#if DEBUG_MIDI_INPUT | TRACK_DEBUG_MIDI
int numberInt = 0;
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void printNumber2(byte segment, byte offset, byte number)
{
 numberInt = number;
 //
 do
 {
   byte hexValue = numberInt % 16;
   if (hexValue <= 9) segments[segment][offset] = (char)pgm_read_word(&numbers[hexValue]);
   else if (hexValue == 10) segments[segment][offset] = S_A;
   else if (hexValue == 11) segments[segment][offset] = S_b;
   else if (hexValue == 12) segments[segment][offset] = S_C;
   else if (hexValue == 13) segments[segment][offset] = S_d;
   else if (hexValue == 14) segments[segment][offset] = S_E;
   else if (hexValue == 15) segments[segment][offset] = S_F;
   numberInt /= 16;
   offset--;
 }
 while(offset >= 0 && numberInt % 16 != 0);
}
#endif

#if DEBUG_MIDI_INPUT
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void debugCreateScreen()
{
  segments[0][0] = segments[0][1] = segments[0][2] = segments[0][3] = 
  segments[0][4] = segments[0][5] = segments[0][6] = segments[0][7] = 0;
  //
  if (midiInputItens > 0)
  {
    printNumber(0, 0, midiInputItensEditPos + 1);
    if (midiInputShowHex) printNumber2(0, 7, midiInput[midiInputItensEditPos]); else printNumber(0, 4, midiInput[midiInputItensEditPos]);
  }
  //
  segments[2][0] = S_T;
  segments[2][1] = S_O;
  segments[2][2] = S_T;
  printNumber(2, 4, midiInputItens); 
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void debugCheckInterface()
{
  if (buttonEvent[0][0] >= kButtonClicked || buttonEvent[0][1] >= kButtonClicked)
  {
    buttonEvent[0][0] = buttonEvent[0][1] = 0;
    midiInputItensEditPos = 0;
    midiInputItens = 0;
    updateScreen = true;
  }
  //
  if (buttonEvent[0][3] >= kButtonClicked)
  {
    buttonEvent[0][3] = 0;
    if (midiInputItensEditPos < (midiInputItens-1)) midiInputItensEditPos++;
    updateScreen = true;
  }
  //
  if (buttonEvent[0][2] >= kButtonClicked)
  {
    buttonEvent[0][2] = 0;
    if (midiInputItensEditPos >  0) midiInputItensEditPos--;
    updateScreen = true;
  } 
  //
  if (buttonEvent[2][7] >= kButtonClicked)
  {
    buttonEvent[2][7] = 0;
    midiInputShowHex = !midiInputShowHex;
    updateScreen = true;
  }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void debugCheckMIDIInput()
{
  while (Serial.available() > 0)
  {
    if (midiInputItens < (DEBUG_MIDI_BUFFER_SIZE-1))
    {
      midiInput[midiInputItens] = Serial.read();
      midiInputItens++;
      updateScreen = true;
    } else Serial.read();
  }
}
#endif
