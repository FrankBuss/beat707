/*
 * 
 * Created by William Kalfelz @ Beat707 (c) 2019 - http://www.Beat707.com
 * 
 */

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void reset()
{
  resetSegments(0, 2);
  memset(trackPosition, 0, sizeof(trackPosition));
  memset(midiOutputBuffer, 0, sizeof(midiOutputBuffer));
  memset(echoSpace, 0, sizeof(echoSpace));
  memset(echoAttackDecay, 0, sizeof(echoAttackDecay));
  memset(echoTrack, 0, sizeof(echoTrack));
  memset(echoCounter, 0, sizeof(echoCounter));
  memset(echoVelocity, 0, sizeof(echoVelocity));  
  memset(echoNote, 0, sizeof(echoNote));
  memset(leds, 0, sizeof(leds));
  memset(buttons, 0, sizeof(buttons));  
  memset(buttonEvent, 0, sizeof(buttonEvent));
  memset(buttonDownTime, 0, sizeof(buttonDownTime));
  memset(prevPlayedNote, 0, sizeof(prevPlayedNote));
  configData.init();
  midiOutputBufferPosition = 0;
  memset(recordBuffer, 0, sizeof(recordBuffer));
  recordBufferPosition = 0;
  memset(sendCCCurrentValue, 0, sizeof(sendCCCurrentValue));
  memset(sendCCCurrentPosition, 0, sizeof(sendCCCurrentPosition));
  memset(noteLenCounters, 0, sizeof(noteLenCounters));
  //
  resetPatternBank();
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void resetPatternBank()
{
  for (byte x=0; x < STEPS; x++) { stepsData[x].init(); }
  patternData.init();  
  //
  #if DEBUG_MIDI_INPUT
    memset(midiInput, 0, sizeof(midiInput));
    midiInputItens = 0;
  #endif 
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void waitMs(int mstime)
{
  for (int xx=0; xx<mstime; xx++) 
  { 
    for (int xms=0; xms<1000; xms++)
    {
      __asm__("nop"); 
    }
  }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void PatternChanged() 
{ 
  patternHasChanges = true; 
}
