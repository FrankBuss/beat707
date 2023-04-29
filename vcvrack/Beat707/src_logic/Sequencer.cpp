/*
 * 
 * Created by William Kalfelz @ Beat707 (c) 2018 - http://www.Beat707.com
 * 
 */

#include "declarations.h"

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#define PPQ_TICK_DOUBLE_NOTE 3
#define PPQ_TICK_END 6
#define PPQ 24

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#if 0
ISR(TIMER1_COMPA_vect) 
{
  if (configData.midiClockInternal) 
  {
    if (configData.seqSyncOut) midi_write(0xF8); // Midi Clock Out //
    ledsBufferFlip();
    pulseOut(tickOutPinState);
    outputMIDIBuffer();
    calculateSequencer++;
  }
  else 
  {
    stopTimer(true);
    handleMIDIInput();
    startTimer(true);
  }
}
#endif

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void outputMIDIBuffer()
{
  while (calculateSequencer > 0) { doTickSequencer(); }
  sendOutputMIDIBuffer();
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void sendOutputMIDIBuffer()
{
  while (midiOutputBufferDTPosition > 0 || midiOutputBufferNTPosition > 0)
  {
    if (midiOutputBufferDTPosition > 0 && midiOutputBufferDT[0][midiOutputBufferDTPosition - 1] > 0)
    {
      midi_write(midiOutputBufferDT[0][midiOutputBufferDTPosition - 1]);
      midi_write(midiOutputBufferDT[1][midiOutputBufferDTPosition - 1]);
      if (midiOutputBufferDT[2][midiOutputBufferDTPosition - 1] != 0xFF) midi_write(midiOutputBufferDT[2][midiOutputBufferDTPosition - 1]);
      midiOutputBufferDT[0][midiOutputBufferDTPosition - 1] = 0;
      midiOutputBufferDTPosition--;
    }
    //
    if (midiOutputBufferNTPosition > 0 && midiOutputBufferNT[0][midiOutputBufferNTPosition - 1] > 0)
    {
      midi_write(midiOutputBufferNT[0][midiOutputBufferNTPosition - 1]);
      midi_write(midiOutputBufferNT[1][midiOutputBufferNTPosition - 1]);
      if (midiOutputBufferNT[2][midiOutputBufferNTPosition - 1] != 0xFF) midi_write(midiOutputBufferNT[2][midiOutputBufferNTPosition - 1]);
      midiOutputBufferNT[0][midiOutputBufferNTPosition - 1] = 0;
      midiOutputBufferNTPosition--;
    }
  }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void doTickSequencer()
{
  while (calculateSequencer > 0)
  {
    if (seqPlaying)
    {
      // --------------------------------------------------------------------------------------------------------------------------------------------------------------------------- //
      // Check For Echo //
      for (byte xe = 0; xe < ECHOS; xe++)
      {
        if (echoCounter[xe][0] > 0)
        {
          echoCounter[xe][1]--;
          if (echoCounter[xe][1] == 0) 
          {
            echoCounter[xe][0]--;
            if (echoCounter[xe][0] > 0) 
            {
              echoCounter[xe][1] = echoSpace[xe];
              //
              trackNoteOn(echoTrack[xe], echoNote[xe], getMPVelocity(patternData.trackProcessor[echoTrack[xe]-1], echoVelocity[xe]));
              //
              int newV = echoVelocity[xe] + echoAttackDecay[xe];
              if (newV < 0) newV = 0;
              if (newV > 127) newV = 127;
              echoVelocity[xe] = newV;
            }
          }
        }
      }
      //
      // --------------------------------------------------------------------------------------------------------------------------------------------------------------------------- //
      //
      // Check Mute Tracks //
      if (prevMuteTrack != configData.muteTrack)
      {
        for (byte x=0; x<(NOTE_TRACKS+DRUM_TRACKS+1); x++)
        {
          if (bitRead(prevMuteTrack, x) != bitRead(configData.muteTrack, x) && bitRead(configData.muteTrack, x) == 1)
          {
            if (x >= NOTE_TRACKS) stopDrumTrackPrevNote(x - NOTE_TRACKS, true); else stopDrumTrackPrevNote(x, false);
          }
        }
        prevMuteTrack = configData.muteTrack;
      }
      //
      if (seqCounter == 0 || seqCounter == PPQ_TICK_DOUBLE_NOTE)
      {
        midiClockBeats++;
        if (midiClockBeats >= 16) midiClockBeats = 0;
        //
        // --------------------------------------------------------------------------------------------------------------------------------------------------------------------------- //
        //
        for (byte x=0; x<DRUM_TRACKS; x++)
        {
          if (bitRead(configData.muteTrack, x) != 1)
          {
            byte xvel = bitRead(stepsData[seqPosition].steps[x], 1 + (variation * 2)) << 1;
            xvel |= bitRead(stepsData[seqPosition].steps[x], 0 + (variation * 2));
            bool isDouble = bitRead(stepsData[seqPosition].stepsDouble[variation], x);
            //
            if ((xvel > 0 && seqCounter == 0) || (seqCounter == PPQ_TICK_DOUBLE_NOTE && isDouble))
            {
              byte theVelocity = configData.accentValues[xvel - 1];
              //
              // Process Echo //
              for (byte xe = 0; xe < ECHOS; xe++)
              {
                if (((patternData.echoConfig[xe].track - 1) == x) && 
                ( (patternData.echoConfig[xe].type == echoTypeOnAllNotes) || ((patternData.echoConfig[xe].type == echoTypeForceMaxVelocity && xvel == 3)) || ((patternData.echoConfig[xe].type == echoTypeForceLowVelocity && xvel == 1)) ))
                {
                  echoCounter[xe][0] = patternData.echoConfig[xe].ticks;
                  echoCounter[xe][1] = patternData.echoConfig[xe].space;
                  echoAttackDecay[xe] = patternData.echoConfig[xe].attackDecay;
                  echoSpace[xe] = patternData.echoConfig[xe].space;
                  echoTrack[xe] = patternData.echoConfig[xe].track - 1;
                  echoNote[xe] = configData.trackNote[x];
                  if (patternData.echoConfig[xe].attackDecay > 0) echoVelocity[xe] = patternData.echoConfig[xe].attackDecay; else echoVelocity[xe] = configData.accentValues[xvel - 1];
                  theVelocity = echoVelocity[xe];
                }
              }
              //
              trackNoteOn(x, configData.trackNote[x], getMPVelocity(patternData.trackProcessor[x], theVelocity));
            }
            //
            if (seqCounter == 0 && patternData.sendCC[x] > 0) sendMIDICC(x, false, 0);
          }
        }
        //
        // --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- //
        //
        for (byte x=0; x<NOTE_TRACKS; x++)
        {
          if (bitRead(configData.muteTrack, x + DRUM_TRACKS) != 1)
          {
            byte xvel = bitRead(stepsData[seqPosition].noteStepsExtras[x][0], 1 + (variation * 2)) << 1;
            xvel |= bitRead(stepsData[seqPosition].noteStepsExtras[x][0], 0 + (variation * 2));
            //
            byte extra = bitRead(stepsData[seqPosition].noteStepsExtras[x][1], 1 + (variation * 2)) << 1;
            extra |= bitRead(stepsData[seqPosition].noteStepsExtras[x][1], 0 + (variation * 2));
            //
            bool isSlide = (extra == 1);
            bool isDouble = (extra == 2);
            bool isNoteOff = (extra == 3);
            byte xnote = stepsData[seqPosition].noteSteps[x][variation];
            //
            if ((xnote > 0 && xvel > 0 && seqCounter == 0) || (xnote > 0 && seqCounter == PPQ_TICK_DOUBLE_NOTE && isDouble))
            {
              if (isNoteOff)
              {
                sendMidiEvent(midiNoteOff, xnote, 0, configData.trackMidiCH[DRUM_TRACKS+x], 1);
                noteLenCounters[DRUM_TRACKS+x] = 0;
                if (xnote == prevPlayedNote[DRUM_TRACKS+x]) prevPlayedNote[DRUM_TRACKS+x] = 0;
              }
              else 
              {
                byte theVelocity = configData.accentValues[xvel - 1];
                //
                // Process Echo //
                for (byte xe = 0; xe < ECHOS; xe++)
                {
                  if (((patternData.echoConfig[xe].track - DRUM_TRACKS - 1) == x) && 
                  ( (patternData.echoConfig[xe].type == echoTypeOnAllNotes) || ((patternData.echoConfig[xe].type == echoTypeForceMaxVelocity && xvel == 3)) || ((patternData.echoConfig[xe].type == echoTypeForceLowVelocity && xvel == 1)) ))
                  {
                    echoCounter[xe][0] = patternData.echoConfig[xe].ticks;
                    echoCounter[xe][1] = patternData.echoConfig[xe].space;
                    echoAttackDecay[xe] = patternData.echoConfig[xe].attackDecay;
                    echoSpace[xe] = patternData.echoConfig[xe].space;
                    echoTrack[xe] = patternData.echoConfig[xe].track - 1;
                    echoNote[xe] = xnote;
                    if (patternData.echoConfig[xe].attackDecay > 0) echoVelocity[xe] = patternData.echoConfig[xe].attackDecay; else echoVelocity[xe] = configData.accentValues[xvel - 1];
                    theVelocity = echoVelocity[xe];
                  }
                }
                //     
                noteTrackNoteOn(DRUM_TRACKS+x, xnote, getMPVelocity(patternData.trackProcessor[DRUM_TRACKS+x], theVelocity), isSlide);
              }
            }
            //
            if (seqCounter == 0 && patternData.sendCC[x] > 0) sendMIDICC(x, false, 1);
          }
        }
        //
        // ---===-------===-------===-------===-------===-------===-------===-------===-------===---- //
        //
        if (recordEnabled && seqCounter == 0)
        {
          for (xm = 0; xm < recordBufferPosition; xm++)
          {
            recordInputCheck(recordBuffer[0][xm], recordBuffer[1][xm], recordBuffer[2][xm], curTrack);
          }
          //
          recordBufferPosition = 0;
        }
        //
        // ---===-------===-------===-------===-------===-------===-------===-------===-------===---- //
        //
        if (configData.tickOut > 0 && seqCounter == 0)
        {
          tickOutCounter++;
          if (tickOutCounter == (configData.tickOut - 1))
          {
            tickOutCounterLen = configData.tickOutLen;
            tickOutPinState = true;            
          }
          else if (tickOutCounter >= configData.tickOut)
          {
            tickOutCounter = 0;
          }
        }
        //
        // ---===-------===-------===-------===-------===-------===-------===-------===-------===---- //
        //
        if (seqCounter == PPQ_TICK_DOUBLE_NOTE && (seqPosition+1) >= STEPS && (variation+1) >= prevPatternTotalVariations)
        {
          if (streamNextPattern)
          {
            streamNextPattern = false;
            loadPatternNow = true;
            currentPlaysPattern = currentPlaysChain = 0;
          }
          else
          {
            if (patternData.playsPattern != 0 && (repeatMode == kRepeatModeNormal || repeatMode == kRepeatModeChain))
            {
              currentPlaysPattern++;
              if ((currentPlaysPattern - 1) == (patternData.playsPattern - 1))
              {
                currentPlaysPattern = 0;
                //
                if (patternData.playsChain != 1)
                {
                  currentPlaysChain++;
                  if (patternData.playsChain == 0 || repeatMode == kRepeatModeChain) currentPlaysChain = 0;
                  if (patternData.playsChain != 0 && currentPlaysChain >= (patternData.playsChain - 1))
                  {
                    currentPlaysChain = 0;
                    nextPattern = currentPattern + 1;
                    loadPatternNow = true;
                  }
                  else
                  {
                    if (patternData.nextPattern == 0) nextPattern = currentPattern + 1; 
                    else 
                    {
                      nextPattern = ((patternData.nextPattern - 1)% 16);
                      nextPatternBank = (patternData.nextPattern - 1) / 16;
                    }
                    loadPatternNow = true;                  
                  }
                }
                else
                {
                    if (patternData.nextPattern == 0) nextPattern = currentPattern + 1; 
                    else 
                    {
                      nextPattern = ((patternData.nextPattern - 1)% 16);
                      nextPatternBank = (patternData.nextPattern - 1) / 16;
                    }
                    loadPatternNow = true;                                   
                }
              }
            }
          }
        }
      }
      //
      if (tickOutCounterLen > 0)
      {
        tickOutCounterLen--;
        if (tickOutCounterLen == 0) tickOutPinState = false;
      }     
      //
      for (byte x=0; x<(DRUM_TRACKS + NOTE_TRACKS); x++)
      {
        if (noteLenCounters[x] > 0)
        {
          noteLenCounters[x]--;
          if (noteLenCounters[x] == 0) 
          {
            if (x < DRUM_TRACKS) sendMidiEvent(midiNoteOff, configData.trackNote[x], 0, configData.trackMidiCH[x], 0);
            else if (prevPlayedNote[x-DRUM_TRACKS] > 0)
            {
              sendMidiEvent(midiNoteOff, prevPlayedNote[x-DRUM_TRACKS], 0, configData.trackMidiCH[x], 0);
              prevPlayedNote[x-DRUM_TRACKS] = 0;
            }
          }
        }
      }
    }
    //
    // ---===-------===-------===-------===-------===-------===-------===-------===-------===---- //
    // ---===-------===-------===-------===-------===-------===-------===-------===-------===---- //
    // ---===-------===-------===-------===-------===-------===-------===-------===-------===---- //
    //
    seqCounter++; 
    if (seqCounter >= PPQ_TICK_END)
    {
      seqCounter = 0;
      seqPosition++;
      showBPMdotBuffer = false;
      //
      switch (seqPosition)
      {
        case 0: showBPMdotBuffer = true; break;
        case 4: showBPMdotBuffer = true; break;
        case 8: showBPMdotBuffer = true; break;
        case 12: showBPMdotBuffer = true; break;
      }
      //
      if (seqPosition >= STEPS)
      {
        seqPosition = 0; 
        showBPMdotBuffer = true;
        if (seqPlaying)
        {
          variation++;
          if (variation >= prevPatternTotalVariations)
          {
            variation = 0;
            checkPatternStream();
            if (loadPatternNow) showErrorMsg(99);
            prevPatternTotalVariations = patternData.totalVariations;
          }
        }    
      }
      //
      // Redo The Left LEDs Indicators //
      if (seqPlaying && curRightScreen != kRightTrackSelection)
      {
        chaseLEDsBuffer[0] = chaseLEDsBuffer[1] = 0;
        if (seqPosition < 8) bitSet(chaseLEDsBuffer[0], seqPosition);
        else bitSet(chaseLEDsBuffer[1], seqPosition - 8);
      }
      ledsVarSeqUpdate();
    }
    //
    calculateSequencer--;
  }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void recordInputCheck(byte data1, byte data2, byte channel, byte track)
{
  byte xvariation = variation;
  byte theStep = seqPosition;
  //
  if (theStep > 0)  theStep--;
  else 
  {
    theStep = STEPS - 1;
    if (xvariation > 0) xvariation--;
    else xvariation = 3;
  }
  //    
  byte xVar = xvariation;
  if (forceVariation >= 0) xVar = forceVariation;
  if (mirror) xVar = 0;
  //
  if (track < DRUM_TRACKS)
  {
    if (configData.trackMidiCH[track] == channel && configData.trackNote[track] == data1 && data2 > 0)
    {
        bitClear(stepsData[theStep].steps[track], (xVar * 2));
        bitClear(stepsData[theStep].steps[track], (xVar * 2) + 1);
        //
        somethingChangedPattern = true;
        stepsData[theStep].steps[track] |= 1 << (xVar * 2);
        //
        if (mirror)
        {
          stepsData[theStep].steps[track] = 0x00;
          stepsData[theStep].steps[track] |= 1;
          stepsData[theStep].steps[track] |= 1 << 2;
          stepsData[theStep].steps[track] |= 1 << 4;
          stepsData[theStep].steps[track] |= 1 << 6;
        }
    }
  }
  else
  {
    if (configData.trackMidiCH[track] == channel)
    {
      if (data2 > 0)
      {
        clearStepsExtrasBits(theStep, xVar, track-DRUM_TRACKS);
        patternData.lastNote[track-DRUM_TRACKS] = data1;
        //      
        lastVelocity = 1;
        if (data2 >= configData.accentValues[2]) lastVelocity = 3;
        else if (data2 >= configData.accentValues[1]) lastVelocity = 2;
        #if RECORD_FORCE_VEL
          lastVelocity = 3;
        #endif
        //
        stepsData[theStep].noteSteps[track-DRUM_TRACKS][xVar] = patternData.lastNote[track-DRUM_TRACKS];
        stepsData[theStep].noteStepsExtras[track-DRUM_TRACKS][0] |= lastVelocity << (xVar * 2);
        //
        checkIfMirrorAndCopy(theStep, track-DRUM_TRACKS);
        somethingChangedPattern = true;         
      }
      else if (stepsData[theStep].noteSteps[track-DRUM_TRACKS][xVar] == 0)
      {
        clearStepsExtrasBits(theStep, xVar, track-DRUM_TRACKS);
        patternData.lastNote[track-DRUM_TRACKS] = data1;
        //      
        stepsData[theStep].noteSteps[track-DRUM_TRACKS][xVar] = patternData.lastNote[track-DRUM_TRACKS];
        stepsData[theStep].noteStepsExtras[track-DRUM_TRACKS][0] |= lastVelocity << (xVar * 2);
        stepsData[theStep].noteStepsExtras[track-DRUM_TRACKS][1] |= 3 << (xVar * 2);
        //
        checkIfMirrorAndCopy(theStep, track-DRUM_TRACKS);
        somethingChangedPattern = true;          
      }
    }
  }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void ledsVarSeqUpdate()
{
  leftLEDsVarSeqBuffer = 0;
  bitSet(leftLEDsVarSeqBuffer, (seqPosition/4));
  byte xVar = variation;
  if (forceVariation >= 0) xVar = forceVariation;
  if (mirror) leftLEDsVarSeqBuffer |= 0B11110000; 
  else 
  {
    if (forceVariation >= 0)
    {
      leftLEDsVarSeqBuffer |= 0B11110000;
      bitClear(leftLEDsVarSeqBuffer, xVar + 4);
    }
    else bitSet(leftLEDsVarSeqBuffer, xVar + 4);
  }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void ledsBufferFlip()
{
  leftLEDsVarSeq = leftLEDsVarSeqBuffer;
  chaseLEDs[0] = chaseLEDsBuffer[0];
  chaseLEDs[1] = chaseLEDsBuffer[1];
  showBPMdot = showBPMdotBuffer;
  updateScreen = true;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
byte getMPVelocity(byte xproc, byte xorgvelocity)
{
  switch(xproc)
  {    
    case fadeIn:    
      return max(procFadeMin,min(127,xorgvelocity-(128-((seqPosition + (variation * 16)) * 2))));
      break;
      
    case fadeOut:   
      return max(procFadeMin,min(127,xorgvelocity-(((seqPosition + (variation * 16)) * 2))));
      break;
      
    case randomVel:
      return max(procFadeMin,min(127,xorgvelocity-byte(random(procFadeMin, 127))));
      break;
  }
  //
  return xorgvelocity;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void trackNoteOn(byte xtrack, byte xnote, byte xvelocity)
{
  sendMidiEvent(midiNoteOn, xnote, xvelocity, configData.trackMidiCH[xtrack], 0);
  if (noteLenCounters[xtrack] > 0) 
  {
    sendMidiEvent(midiNoteOff, xnote, 0, configData.trackMidiCH[xtrack], 0);
    noteLenCounters[xtrack] = 0;
  }
  //
  noteLenCounters[xtrack] =  configData.drumNoteLen[xtrack] + 1;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void noteTrackNoteOn(byte xtrack, byte xnote, byte xvelocity, bool slide)
{
  if (slide && prevPlayedNote[xtrack-DRUM_TRACKS] > 0 && prevPlayedNote[xtrack-DRUM_TRACKS] != xnote)  sendMidiEvent(midiNoteOff, prevPlayedNote[xtrack-DRUM_TRACKS], 0, configData.trackMidiCH[xtrack], 1);
  sendMidiEvent(midiNoteOn, xnote, xvelocity, configData.trackMidiCH[xtrack], 1);
  if ((!slide && prevPlayedNote[xtrack-DRUM_TRACKS] > 0) || prevPlayedNote[xtrack-DRUM_TRACKS] == xnote)  sendMidiEvent(midiNoteOff, prevPlayedNote[xtrack-DRUM_TRACKS], 0, configData.trackMidiCH[xtrack], 1);
  //  
  prevPlayedNote[xtrack-DRUM_TRACKS] = xnote;
  noteLenCounters[xtrack] =  configData.drumNoteLen[xtrack] + 1; 
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void resetSequencer()
{
  pulseOut(false);
  tickOutCounter = 0;
  tickOutCounterLen = 0;
  tickOutPinState = false;
  //
  seqPositionPrev = seqPosition;
  seqCounterPrev = seqCounter;
  variationPrev = variation;
  midiClockBeatsPrev = midiClockBeats;
  currentPlaysPattern = currentPlaysChain = 0;  
  recordBufferPosition = 0;
  //
  midiOutputBufferDTPosition = 0;
  midiOutputBufferNTPosition = 0;
  memset(midiOutputBufferDT, 0, sizeof(midiOutputBufferDT));
  memset(midiOutputBufferNT, 0, sizeof(midiOutputBufferNT));
  memset(noteLenCounters, 0, sizeof(noteLenCounters));
  //  
  for (byte xe = 0; xe < ECHOS; xe++)
  {
    echoCounter[xe][0] = echoCounter[xe][1] = 0;
  }
  //
  seqPosition = seqCounter = variation = midiClockBeats = 0;
  showBPMdot = showBPMdotBuffer = true;
  //
  for (byte x=0; x<DRUM_TRACKS; x++)
  {
    stopDrumTrackPrevNote(x, true);
  }
  //
  for (byte x=0; x<NOTE_TRACKS; x++)
  {
    stopDrumTrackPrevNote(x, false);
  }
  //
  if (configData.writeProtectFlash) ShowTemporaryMessage(kMemoryProtectMessage);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void stopDrumTrackPrevNote(byte track, bool isDrumTrack)
{
  if (isDrumTrack)
  {
    sendMidiEvent(midiNoteOff, configData.trackNote[track], 0, configData.trackMidiCH[track], 0);
  }
  else
  {
    if (prevPlayedNote[track] > 0)
    {
      sendMidiEvent(midiNoteOff, prevPlayedNote[track], 0, configData.trackMidiCH[DRUM_TRACKS+track], 1);
      prevPlayedNote[track] = 0;
    } 
  }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void startSequencer(bool docontinue)
{
  bool alreadyPlaying = seqPlaying;
  stopTimer(false);
  seqPlaying = true;
  if (docontinue)
  {
    seqPosition = seqPositionPrev;
    seqCounter = seqCounterPrev;
    variation = variationPrev;
    midiClockBeats = midiClockBeatsPrev;
    tickOutCounter = 0;
    pulseOut(false);
  }
  else 
  {
    resetSequencer();
    resetProgramChangeAndCC();
  }
  prevPatternTotalVariations = patternData.totalVariations;
  if (configData.seqSyncOut && configData.midiClockInternal) midi_write(0xFA); // MIDI Start
  if (alreadyPlaying)
  {
    midiOutputBufferDTPosition = midiOutputBufferNTPosition = 0;
    memset(midiOutputBufferDT, 0, sizeof(midiOutputBufferDT));
    memset(midiOutputBufferNT, 0, sizeof(midiOutputBufferNT));
    calculateSequencer = 1;
    doTickSequencer(); 
  }
  //
  outputMIDIBuffer();
  calculateSequencer = 1;
  //
  ledsVarSeqUpdate();
  startTimer(false);
  leftLEDsVarSeq = leftLEDsVarSeqBuffer;
  chaseLEDs[0] = chaseLEDs[1] = 0;
  chaseLEDs[0] = 0B10000000;
  updateScreen = true;
  //
  tickOutPinState = true;
  pulseOut(true);
  tickOutCounter = 0;
  tickOutCounterLen = configData.tickOutLen;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void startTimer(bool force)
{
  if (configData.midiClockInternal || force)
  {
    //TCCR1A = TCCR1B = 0;
    //if (configData.midiClockInternal) bitWrite(TCCR1B, CS11, 1); else bitWrite(TCCR1B, CS10, 1);
    //bitWrite(TCCR1B, WGM12, 1);
    updateSequencerSpeed(false);
    //bitWrite(TIMSK1, OCIE1A, 1); 
  }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void stopTimer(bool force)
{
  if (configData.midiClockInternal || force)
  {
    //bitWrite(TIMSK1, OCIE1A, 0);
    //TCCR1A = TCCR1B = OCR1A = 0; 
  }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void updateSequencerSpeed(bool force)
{
  // Calculates the Frequency for the Timer, used by the PPQ clock (Pulses Per Quarter Note) //
  // This uses the 16-bit Timer1, unused by the Arduino, unless you use the analogWrite or Tone functions //
  //
  //if (configData.midiClockInternal || force) OCR1A = (F_CPU / 8) / ((((configData.BPM)*(PPQ))/60)) - 1;    
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setupTimerForExternalMIDISync(bool active)
{
#if !DEBUG_MIDI_INPUT  
  if (active)
  {
    stopTimer(true);
    //OCR1A = EXTERNAL_CLOCK_TIMER;
    startTimer(true);
  }
  else
  {
    stopTimer(true);
    updateSequencerSpeed(true);
    startTimer(true);
  }
#endif
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void stopSequencer(void)
{
  // Stop Timer //
  stopTimer(false);
  //
  if (configData.seqSyncOut && configData.midiClockInternal) midi_write(0xFC); // MIDI Stop
  //
  seqPlaying = false;
  resetSequencer();
  MIDIallNotesOff();
  resetProgramChangeAndCC();
  outputMIDIBuffer();
  //
  checkIfDataNeedsSaving();  
  if (streamNextPattern)
  {
    streamNextPattern = false;
    loadPatternNow = true;
  }
  //
  startTimer(false);
  updateScreen = true;  
  calculateSequencer = 1;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void MIDIallNotesOff()
{
  for (byte x=0; x<midiChannels; x++)
  {
    sendMidiEvent(0xB0, 0x7B, 0x00, x, 0);
  }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void sendMidiEvent(byte type, byte byte1, byte byte2, byte channel, byte slot)
{
  if (slot == 0)
  {
    midiOutputBufferDT[0][midiOutputBufferDTPosition] = type + channel;
    midiOutputBufferDT[1][midiOutputBufferDTPosition] = byte1;
    midiOutputBufferDT[2][midiOutputBufferDTPosition] = byte2;
    midiOutputBufferDTPosition++;
  }
  else
  {
    midiOutputBufferNT[0][midiOutputBufferNTPosition] = type + channel;
    midiOutputBufferNT[1][midiOutputBufferNTPosition] = byte1;
    midiOutputBufferNT[2][midiOutputBufferNTPosition] = byte2;
    midiOutputBufferNTPosition++;
  }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void startMIDIinterface()
{
#if 0
  #if DEBUG_SERIAL
    Serial.begin(9600);
    Serial.println("Startup");
  #else
    #if MIDI_OVER_USB 
      Serial.begin(38400); 
    #else 
      Serial.begin(31250); // 31250 MIDI Interface //
    #endif
    Serial.flush();
    resetSequencer();
    MIDIallNotesOff();
  #endif
#endif
} 

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void sendMIDIProgramChange(byte track)
{
  if (patternData.programChange[track] > 0) sendMidiEvent(midiProgramChange, patternData.programChange[track] - 1, 0xFF, configData.trackMidiCH[track], 0);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void sendMIDICC(byte track, bool sendNow, byte slot)
{
  if (bitRead(patternData.sendCCValueLFO[curTrack], midiCCLFO) == 1)
  {
    int newValue = sendCCCurrentValue[track];
    bool isGoingUp = (bitRead(sendCCCurrentPosition[track], 7) == 0);
    byte theRate = (patternData.sendCCValueLFO[curTrack] & 0x7F);
    //
    if (theRate < 32)
    {
      sendCCCurrentPosition[track] = (sendCCCurrentPosition[track] & 0x7F) + 1;
      if (!isGoingUp) bitSet(sendCCCurrentPosition[track], 7);
      //
      if ((sendCCCurrentPosition[track] & 0x7f) > (32 - theRate))
      {
        if (isGoingUp)
        {
          newValue++;
          if (newValue > 127) 
          {
            newValue = 127;
            isGoingUp = false;
          }
        }
        else
        {
          newValue--;
          if (newValue < 0) 
          {
            newValue = 0;
            isGoingUp = true;
          }
        }
        sendCCCurrentPosition[track] = 0;
        if (!isGoingUp) bitSet(sendCCCurrentPosition[track], 7);
      }
    }
    else
    {
      if (isGoingUp) 
      {
        newValue += (theRate - 31); 
        if (newValue > 127) 
        {
          newValue = 127;
          isGoingUp = false;
          bitSet(sendCCCurrentPosition[track], 7);
        }
      }
      else 
      {
        newValue -= (theRate - 31);
        if (newValue < 0) 
        {
          newValue = 0;
          isGoingUp = true;
          bitClear(sendCCCurrentPosition[track], 7);
        }
      }
    }
    //
    if (byte(newValue) != sendCCCurrentValue[track]) 
    {
      sendCCCurrentValue[track] = byte(newValue);
      sendMidiEvent(midiCC, patternData.sendCC[track] - 1, sendCCCurrentValue[track] & 0x7F, configData.trackMidiCH[track], slot);
    }
  }
  else
  {
    if (sendNow) sendMidiEvent(midiCC, patternData.sendCC[track] - 1, patternData.sendCCValueLFO[curTrack], configData.trackMidiCH[track], slot);
 }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void resetProgramChangeAndCC()
{
  for (byte x = 0; x < (DRUM_TRACKS+NOTE_TRACKS); x++) 
  { 
    sendMIDIProgramChange(x);
    //
    if (patternData.sendCC[x] > 0)
    {
      sendCCCurrentValue[x] = 0;
      sendCCCurrentPosition[x] = 0;
      if (patternData.sendCC[x] > 0) sendMIDICC(x, true, (x < DRUM_TRACKS) ? 0 : 1);
    }
  }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void pulseOut(bool enable)
{
#if 0
  if (enable) PORTC = 0xFF; else PORTC = 0x00;
#endif
}
