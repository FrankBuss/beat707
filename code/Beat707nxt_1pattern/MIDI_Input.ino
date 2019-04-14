/*
 * 
 * Created by William Kalfelz @ Beat707 (c) 2019 - http://www.Beat707.com
 * 
 */

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void handleMIDIInput()
{
  if (Serial.available() > 0)
  {
    if (receivingExternalSysEx)
    {
      inputByte = Serial.read();
      if (inputByte == 0xF7) receivingExternalSysEx = false;
    }
    else if (midiInputStage == 0)
    {
      inputByte = Serial.read();
      //
      #if TRACK_DEBUG_MIDI
        lastMIDIinput[3] = lastMIDIinput[2];
        lastMIDIinput[2] = lastMIDIinput[1];
        lastMIDIinput[1] = lastMIDIinput[0];
        lastMIDIinput[0] = inputByte;
      #endif     
      //
      if (inputByte == 0xF8) // Midi Clock Sync Tick 24PPQ =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
      {
        if (!configData.midiClockInternal) 
        {
          if (configData.seqSyncOut) Serial.write(0xF8);
          ledsBufferFlip();
          pulseOut(tickOutPinState);
          outputMIDIBuffer();
          calculateSequencer++;
        }
      }
      else if (inputByte == 0xFA) // MIDI Start =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
      {
        if (!configData.midiClockInternal) 
        {
          if (configData.seqSyncOut) Serial.write(0xFA);
          startSequencer(false);
        }
      }
      else if (inputByte == 0xFC) // MIDI Stop =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
      {
        if (!configData.midiClockInternal) 
        {
          if (configData.seqSyncOut) Serial.write(0xFC);
          stopSequencer();
        }
      }
      else if (inputByte == 0xFB)   // MIDI Clock Continue
      {
        if (!configData.midiClockInternal) 
        {
          if (configData.seqSyncOut) Serial.write(0xFA);
          //
          #if EXTERNAL_CONTINUE
            startSequencer(true);
          #endif
        }       
      }
      else if (inputByte == 0xF0) // System Exclusive Data - SysEx =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
      {
        receivingExternalSysEx = true;
      }
      else if (inputByte == 0xFE   // Active Sensing
              || inputByte == 0xF6   // Tune Request
              || inputByte == 0xFF)  // System Reset
      {
        if (configData.seqSyncOut && !configData.midiClockInternal && inputByte == 0xFB) Serial.write(0xFB);
      }
      else
      {
        midiInputStage = 1;
        midiInputBuffer[0] = inputByte;
      }
    }
    else if (midiInputStage == 1)
    {
      midiInputBuffer[1] = Serial.read();
      //
      switch(midiInputBuffer[0] & 0xF0)
      {
        case 0x80:
        case 0x90:
        case 0xA0:
        case 0xB0:
        case 0xF2:
          midiInputStage = 2;
          break;
        //
        default:
          midiInputStage = 0;
          //
          #if TRACK_DEBUG_MIDI
            lastMIDIinput[3] = 0;
            lastMIDIinput[2] = 0;
            lastMIDIinput[1] = midiInputBuffer[0];
            lastMIDIinput[0] = midiInputBuffer[1];
          #endif              
          //
          break;
      }        
    }
    else if (midiInputStage == 2)
    {
      byte lastData = Serial.read();
      midiInputStage = 0;
      byte channel = midiInputBuffer[0] & 0xF;
      //
      #if TRACK_DEBUG_MIDI
        lastMIDIinput[3] = 0;
        lastMIDIinput[2] = midiInputBuffer[0];
        lastMIDIinput[1] = midiInputBuffer[1];
        lastMIDIinput[0] = lastData;
      #endif              
      //      
      switch(midiInputBuffer[0] & 0xF0)
      {
        case 0x80: // Note Off -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        {
          if (recordEnabled && seqPlaying) addRecordNotes(midiInputBuffer[1], 0, channel);
          //
          #if RECORD_ENABLED_ECHO
            isSendingMIDIEcho = true;
            Serial.write(midiInputBuffer[0]);
            Serial.write(midiInputBuffer[1]);
            Serial.write(lastData);
            checkLateSequencerTick(false);
          #endif
          //
          break;
        }
        //
        case 0x90: // Note On =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        {
          if (recordEnabled && seqPlaying) addRecordNotes(midiInputBuffer[1], lastData, channel);
          //
          #if RECORD_ENABLED_ECHO
            isSendingMIDIEcho = true;
            Serial.write(midiInputBuffer[0]);
            Serial.write(midiInputBuffer[1]);
            Serial.write(lastData);
            checkLateSequencerTick(false);
          #endif           
          //
          break; 
        }      
      }
    }
  }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void addRecordNotes(byte data1, byte data2, byte channel)
{
  if (recordBufferPosition < MIDI_INPUT_BUFFER)
  {
    recordBuffer[0][recordBufferPosition] = data1;
    recordBuffer[1][recordBufferPosition] = data2;
    recordBuffer[2][recordBufferPosition] = channel;
    recordBufferPosition++;
  }
}
