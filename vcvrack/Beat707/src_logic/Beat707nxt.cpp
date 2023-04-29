/*
 * 
 * Beat707NXT_BETA2 - Works with 3 x TM1638 boards connected in paralel
 * Created by William Kalfelz @ Beat707 (c) 2018 - http://www.Beat707.com / http://www.Wusik.com / http://www.Kalfelz.com
 * Creative Development by Gert Borovčak
 * 
 * ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * 
 *  * 2019 Changes to make
 * 
 *  - Invert the SPI Signal for the new SMD Buffer IC: Frank says: change "SPDR = data;" to "SPDR = data ^ 0xff;" in SPI.h
 *  - just to have it all in one sentence: we have that bank 0 issue, the stuttering when saving, the wish to put mutepage to shift&bpm -, invert lines from atmel to flash
 *  
 * ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * 
 *      Nov 20 2018
 *    - Added the Output Pulse Click. Check the Global Menu for the new option. The Click value is in 1/16 steps, and the Len(ght) is in 24-PPQ Ticks. Use the Header A54, any pin should work, as I'm using PORTC on the ATmega328, which holds all Analog Pins (they work as digital pins too).
 * 
 *      Nov 19 2018
 *    - Changed how MIDI CC works. Now you have two options: send a single value when the pattern starts, or LFO mode. This uses much less RAM. When in LFO mode instead of Value you will see Rate, which starts very slowly and goes up to get very fast. So you have 3 options in total: CC value, CC LFO On/Off, CC Single Value or LFO Rate.
 *    - Internal change to the MIDI Output Buffers, to use less RAM.
 *    Version 2
 *    - Added Note Len to DRUM TRACKS. By default it is 1/16, if I'm not mistaken, otherwise play around with the values and let me know which value I should use. Just open the global editor and you will see a new option after Note Number. The number is in ticks. Each step is 6 ticks, so 3 ticks is 1/32 step and 6 ticks is 1/16 step.
 *    Version 3
 *    - Added Note Len to NOTE TRACKS. Also default is 1/16. If you set a long note the note OFF option can stop the note before the actual note time is reached.
 *    - When changing the Program Change of a track, it will send the new value right away.
 *      
 *      Nov 14 2018
 *    - Removed Accent Tracks. By default you get velocity MAX and if you hold Button 5 you can cycle the other velocities.
 *    - Removed some MIDI Processors and left Fade In/Out and Random only.
 *    - Set 10 Drum Tracks and 6 Note Tracks.
 *    - Added Program Change and MIDI CC per track in the global menu. This is sent when a new pattern loads up or when you press play/stop on the sequencer. The MIDI CC you can set a Start and End value. If you want just to set 100 as the value and don't change it overtime, just set Start as 100 and leave the rest untouched. To Glide values you just set Start/End and the Rate value, which should not be zero. There is also the MODE option: Slow or Normal. In Normal mode, for each step it will add the Rate to the Start value, until it reaches the End value. In Slow mode, it will count how many rate-values has passed, and raise the Start value by one or -1 if rate is negative. And finally there's the LFO option for MIDI CC which will go up an down from Start to End and End to Start and repeat until you stop the sequencer.
 *    - Changed the Record behaviour to only record on the current selected track (the onscreen one).
 *    - Updated the Global menu to now show things that are not needed. This goes for the Echo/Delay options when the selected one is OFF, and the same goes for Track MIDI CC, if it's OFF, it won't show the other options until you select a MIDI CC for that track.
 *    - Removed SYSEX support as it will need to be revised and redone.
 *    Version 2
 *    - Reduced memory usage for the Track MIDI CCs.
 *    
 * ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * 
 *  List of FUTURE Features Changes (*) Done
 *    - (*) 10 Drum Tracks (no accent track)
 *    - (*) 6 Note Tracks
 *    - 512 Patterns (no pattern bank, but use A B C D ... and 64 patterns per letter)
 *    --------------------
 *    - (*) Output Click (5VCC Gate Out). Gate On every XXX Ticks, duration of XXX Ticks. !!!! If you want, it could play a pulse sound, as we have PWM output that supports that.
 *    - (*) Program Change and CC per track with the option to Glide CC up/down values with a value (+/-). EG: CC Glide +02 every XXX ticks.
 *    - (*) Note Lenght in Ticks.
 *    - (*) Record only the selected track (no record per track option)
 *    - New Flash storage code to use better the flash memory IC, avoiding it getting dead-write-sectors so soon.
 *  
 */

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#include "hardware.h"
#include "declarations.h"
#include "Functions.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
byte segments[3][16];
byte leds[3];
byte buttons[3]; // raw button values
byte buttonEvent[3][8];
byte buttonEventWasHolding[3] = {0, 0, 0};
byte buttonIsHolding[8];
byte buttonDownTime[3][8]; // time of the button press
byte variation = 0; // ABCD Variations
char forceVariation = -1;
byte curTrack = 0;
bool updateScreen = true;
byte curLeftScreen = 0;
byte curRightScreen = 0;
byte currentPattern = 0;
byte nextPattern = 0;
byte currentPatternBank = 0;
byte nextPatternBank = 0;
uint16_t patternBitsSelector;
bool mirror = true;
bool somethingClicked = false;
bool somethingHappened = false;
bool forceAccent = false;
bool showBPMdot = false;
bool showBPMdotBuffer = false;
bool seqPlaying = false;
byte seqPosition = 0;
byte seqCounter = 0;
byte midiClockBeats = 0;
byte seqPositionPrev = 0;
byte seqCounterPrev = 0;
byte variationPrev = 0;
byte midiClockBeatsPrev = 0;
bool somethingChangedPattern = false; // if true saves the pattern (steps, double steps, midi processor and variations)
bool somethingChangedConfig = false; // if true saves the config data (BPM, track Note #, track MIDI CC, SeqSyncOut)
bool streamNextPattern = false;
bool loadPatternNow = false;
byte ignoreButtons = false;
byte lastVelocity = 0B00000011;
bool editingNote = false;
byte editStep = 0;
byte editVariation = 0;
byte prevPlayedNote[NOTE_TRACKS + DRUM_TRACKS];
byte menuPosition = 0;
byte initMode = 0;
char flashHeader[8];
byte receivingExternalSysEx = false;
uint32_t prevMuteTrack = 0;
byte totalFlashErrors = 0;
byte prevPatternTotalVariations = 4;
byte midiInputStage = 0;
byte midiInputBuffer[2] = {0, 0};
byte midiOutputBufferDT[3][DRUM_TRACKS * 3];
byte midiOutputBufferNT[3][NOTE_TRACKS * 3];
byte midiOutputBufferDTPosition = 0;
byte midiOutputBufferNTPosition = 0;
byte calculateSequencer = 0;
uint16_t patternPagePos = 0;
uint16_t pagePos = 0;
char editingNoteTranspose = -127;
bool noteTransposeWasChanged = false;
bool noteTransposeEditAllSteps = false;
byte leftLEDsVarSeq = 0;
byte leftLEDsVarSeqBuffer = 0;
uint16_t chaseLEDs[2] = {0, 0};
uint16_t chaseLEDsBuffer[2] = {0, 0};
byte stepVelocity = 0;
byte noteStepGlideDoubleOff = 0;
bool ignoreNextButton = false;
byte currentPlaysPattern = 0;
byte currentPlaysChain = 0;
byte repeatMode = 0;
byte showTemporaryMessage = 0;
byte temporaryMessageCounter = 0;
byte xm = 0;
byte inputByte = 0;
byte sendCCCurrentValue[DRUM_TRACKS+NOTE_TRACKS];
byte sendCCCurrentPosition[DRUM_TRACKS+NOTE_TRACKS]; // 0 to 127 and the bit 8 is to know if its going up or down //
byte noteLenCounters[DRUM_TRACKS+NOTE_TRACKS];
byte tickOutCounter = 0;
byte tickOutCounterLen = 0;
bool tickOutPinState = false;
bool isSelectingBank = false;
//
#if TRACK_DEBUG_MIDI
  byte lastMIDIinput[4] = {0, 0, 0, 0};
#endif
//
#if DEBUG_MIDI_INPUT
  byte midiInput[DEBUG_MIDI_BUFFER_SIZE];
  int midiInputItens = 0;
  int midiInputItensEditPos = 0;
  bool midiInputShowHex = true;
#endif
//
bool recordEnabled = false;
byte recordBuffer[3][MIDI_INPUT_BUFFER];
byte recordBufferPosition = 0;
//
byte echoCounter[ECHOS][2];
byte echoVelocity[ECHOS];
char echoTrack[ECHOS];
char echoAttackDecay[ECHOS];
byte echoSpace[ECHOS];
byte echoNote[ECHOS];
byte echoEdit = 0;

WSTEPS stepsData[STEPS];
WPATTERN patternData;
WCONFIG configData;

byte stepChars[4] = { 0B00000000, 0B00001000, 0B01010100, 0B00110111 };
byte numbers[10] = { 0B00111111, 0B00000110, 0B01011011, 0B01001111, 0B01100110, 0B01101101, 0B01111101, 0B00000111, 0B01111111, 0B01100111};




// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup()
{
  pulseOut(false);
  //
  startMIDIinterface();
  initTM1638();
  reset();
  splashScreen();
  //
  #if SHOW_FREE_RAM
    freeMemory();
    sendScreen();
    waitMs(2000);
  #endif 
  flashInit(false);
  loadPatternBank(0);
  calculateSequencer++;
  //
  setupTimerForExternalMIDISync(!configData.midiClockInternal); 
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop() 
{
  #if !DEBUG_MIDI_INPUT
    doTickSequencer();
    checkPatternStream();
    readButtons();
    //
    if (editingNoteTranspose != -127 && stepBeenHold() == -2) editingNoteTranspose = -127;
    //
    checkInterface(); 
    //
    if (configData.midiClockInternal) handleMIDIInput();
    if (updateScreen)
    {
      updateScreen = false;
      createScreen(); 
      //
      sendScreen();
    } 
  #else
    debugCheckMIDIInput();
    readButtons();
    debugCheckInterface();
    //
    if (updateScreen)
    {
      updateScreen = false;
      debugCreateScreen();
      //
      sendScreen();
    }
  #endif
}
