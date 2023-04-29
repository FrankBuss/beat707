#include <string.h>
#include <stdint.h>

#include "hardware.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#define DEFAULT_NOTE 36       // When adding a new note the default note value for the very first note - Set to 0 so it asks what is the default note everytime you add the first one on a pattern
#define DEFAULT_MIDI_CH 10    // MIDI Channel for all tracks: 1 ~ 16
#define DRUM_TRACKS 10        // This can't go above 16
#define NOTE_TRACKS 6         // This can't go above 8
#define PATTERNS 16           // This can't go above 64
#define PT_BANKS 16           // # of Pattern Banks - should be the MAX that the Winbond chip can take (usually 28)
#define ECHOS 2               // Depends on RAM left, usually can't go above 9
#define STEPS 16              // This can't go above 16
#define TEMPORARY_MESSAGE_TIME 80
#define EXTERNAL_CLOCK_TIMER (F_CPU / 3960) // Timer speed used to read the External MIDI Clock - MIDI is 31250 bauds = 3906 bytes per second
#define MIDI_INPUT_BUFFER (4 * 2) // The size of the internal buffer used for recording
#define DEBUG_MIDI_BUFFER_SIZE 512
#define FLASH_CHIPSIZE MB64
#define FLASH_VERSION '2'

#include <Functions.h>

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
enum
{
  fadeIn = 1, fadeOut, randomVel,
  kButtonNone = 0, kButtonClicked, kButtonHold, kButtonRelease, kButtonReleaseNothingClicked,
  kRightSteps = 0, kRightABCDVariations, kRightTrackSelection, kRightPatternSelection, kRightMenuCopyPaste, kRightMenu, kMuteMenu,
  midiNoteOn = 0x90, midiNoteOff = 0x80, midiCC = 0xB0, midiProgramChange = 0xC0, midiChannels = 16,
  echoTypeOnAllNotes = 0, echoTypeForceMaxVelocity, echoTypeForceLowVelocity,
  midiCCLFO = 7,
  procFadeMin = 4, 
  //
  menuFirst = 0, 
      menuPtNext = 0,
      menuPtPlays, 
      menuPtPlaysChain, 
      menuMIDIChannel, 
      menuNote, 
      menuNoteLen,
      menuAccent1, 
      menuAccent2, 
      menuAccent3, 
      menuVariationsABCD, 
      menuProc, 
      menuEcho, 
      menuEchoTrack, 
      menuEchoTicks, 
      menuEchoSpace, 
      menuEchoAttackDecay, 
      menuEchoType, 
      menuClockType, 
      menuSyncOut, 
      menuProtect, 
      menuProgramChange,
      menuMIDICC,
      menuMIDICCLFO,
      menuMIDICCValueRate,
      menuPulseOut,
      menuPulseOutLen,
      menuInit, 
      menuMIDIinPattern, 
        lastMenu = menuMIDIinPattern,
  //
  kLeftMain = 0,
  kMemoryProtectMessage = 0, kPatternRepeatMessage,
  kRepeatModeNormal = 0, kRepeatModeChain, kRepeatModePattern
};

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
extern byte segments[3][8];
extern byte leds[3];
extern byte buttons[3]; // raw button values
extern byte buttonEvent[3][8];
extern byte buttonEventWasHolding[3];
extern byte buttonIsHolding[8];
extern byte buttonDownTime[3][8]; // time of the button press
extern byte variation; // ABCD Variations
extern char forceVariation;
extern byte curTrack;
extern bool updateScreen;
extern byte curLeftScreen;
extern byte curRightScreen;
extern byte currentPattern;
extern byte nextPattern;
extern byte currentPatternBank;
extern byte nextPatternBank;
extern uint16_t patternBitsSelector;
extern bool mirror;
extern bool somethingClicked;
extern bool somethingHappened;
extern bool forceAccent;
extern bool showBPMdot;
extern bool showBPMdotBuffer;
extern bool seqPlaying;
extern byte seqPosition;
extern byte seqCounter;
extern byte midiClockBeats;
extern byte seqPositionPrev;
extern byte seqCounterPrev;
extern byte variationPrev;
extern byte midiClockBeatsPrev;
extern bool somethingChangedPattern; // if true saves the pattern (steps, double steps, midi processor and variations)
extern bool somethingChangedConfig; // if true saves the config data (BPM, track Note #, track MIDI CC, SeqSyncOut)
extern bool streamNextPattern;
extern bool loadPatternNow;
extern byte ignoreButtons;
extern byte lastVelocity;
extern bool editingNote;
extern byte editStep;
extern byte editVariation;
extern byte prevPlayedNote[NOTE_TRACKS];
extern byte menuPosition;
extern byte initMode;
extern char flashHeader[8];
extern byte receivingExternalSysEx;
extern uint32_t prevMuteTrack;
extern byte totalFlashErrors;
extern byte prevPatternTotalVariations;
extern byte midiInputStage;
extern byte midiInputBuffer[2];
extern byte midiOutputBufferDT[3][DRUM_TRACKS * 3];
extern byte midiOutputBufferNT[3][NOTE_TRACKS * 3];
extern byte midiOutputBufferDTPosition;
extern byte midiOutputBufferNTPosition;
extern byte calculateSequencer;
extern uint16_t patternPagePos;
extern uint16_t pagePos;
extern char editingNoteTranspose;
extern bool noteTransposeWasChanged;
extern bool noteTransposeEditAllSteps;
extern byte leftLEDsVarSeq;
extern byte leftLEDsVarSeqBuffer;
extern uint16_t chaseLEDs[2];
extern uint16_t chaseLEDsBuffer[2];
extern byte stepVelocity;
extern byte noteStepGlideDoubleOff;
extern bool ignoreNextButton;
extern byte currentPlaysPattern;
extern byte currentPlaysChain;
extern byte repeatMode;
extern byte showTemporaryMessage;
extern byte temporaryMessageCounter;
extern byte xm;
extern byte inputByte;
extern byte sendCCCurrentValue[DRUM_TRACKS+NOTE_TRACKS];
extern byte sendCCCurrentPosition[DRUM_TRACKS+NOTE_TRACKS]; // 0 to 127 and the bit 8 is to know if its going up or down //
extern byte noteLenCounters[DRUM_TRACKS+NOTE_TRACKS];
extern byte tickOutCounter;
extern byte tickOutCounterLen;
extern bool tickOutPinState;
extern bool isSelectingBank;
//
#if TRACK_DEBUG_MIDI
extern   byte lastMIDIinput[4];
#endif
//
#if DEBUG_MIDI_INPUT
extern   byte midiInput[DEBUG_MIDI_BUFFER_SIZE];
extern   int midiInputItens;
extern   int midiInputItensEditPos;
extern   bool midiInputShowHex;
#endif
//
extern bool recordEnabled;
extern byte recordBuffer[3][MIDI_INPUT_BUFFER];
extern byte recordBufferPosition;
//
extern byte echoCounter[ECHOS][2];
extern byte echoVelocity[ECHOS];
extern char echoTrack[ECHOS];
extern char echoAttackDecay[ECHOS];
extern byte echoSpace[ECHOS];
extern byte echoNote[ECHOS];
extern byte echoEdit;
//
struct WECHO
{
  byte track, ticks, space, type;
  char attackDecay;
  //
  void init()
  {
    track = type = 0;
    ticks = 24;
    space = 14;
    attackDecay = 5;
  }
};
//
struct WSTEPS
{
  byte steps[DRUM_TRACKS]; // steps are stored as 8 bits variations in AABBCCDD format. 2 bits per step per variation. 0~3
  uint16_t stepsDouble[4]; // 16 drum tracks, 4 bits each for the ABCD variations
  byte noteSteps[NOTE_TRACKS][4];
  byte noteStepsExtras[NOTE_TRACKS][2]; // 2 bits packed as ABCD: [0] = Velocity (2 bits) ABCD, [1] = Glide/Double ABCD (2 bits)
  //
  void init()
  {
    memset(steps, 0, sizeof(steps));
    memset(stepsDouble, 0, sizeof(stepsDouble));
    memset(noteSteps, 0, sizeof(noteSteps));
    memset(noteStepsExtras, 0, sizeof(noteStepsExtras));
  }
};
struct WPATTERN // 
{
  byte totalVariations;
  byte trackProcessor[DRUM_TRACKS+NOTE_TRACKS]; 
  byte lastNote[NOTE_TRACKS];
  WECHO echoConfig[ECHOS];
  uint16_t nextPattern;
  byte playsPattern, playsChain;
  byte programChange[DRUM_TRACKS+NOTE_TRACKS];
  byte sendCC[DRUM_TRACKS+NOTE_TRACKS];
  byte sendCCValueLFO[DRUM_TRACKS+NOTE_TRACKS]; // stores the value in the first 7 bits, last bit set if its a LFO and the value is the rate instead
  //
  void init()
  {
    for (byte xe=0; xe < ECHOS; xe++) { echoConfig[xe].init(); }
    //
    memset(trackProcessor, 0, sizeof(trackProcessor));
    memset(lastNote, DEFAULT_NOTE, sizeof(lastNote));
    memset(programChange, 0, sizeof(programChange));
    memset(sendCC, 0, sizeof(sendCC));
    memset(sendCCValueLFO, 0, sizeof(sendCCValueLFO));
    //
    totalVariations = 4;
    nextPattern = 0;
    playsPattern = 0;
    playsChain = 1;
  }
};
//
struct WCONFIG
{
  byte drumNoteLen[DRUM_TRACKS+NOTE_TRACKS];
  byte trackNote[DRUM_TRACKS];
  byte trackMidiCH[DRUM_TRACKS+NOTE_TRACKS]; // 0~15 
  byte accentValues[3];
  byte BPM;
  bool seqSyncOut;
  uint32_t muteTrack;
  byte midiInputToPatternChannel;
  bool midiClockInternal;
  bool writeProtectFlash;
  byte tickOut;
  byte tickOutLen;
  //
  void init()
  {
    trackNote[0] = 36;
    trackNote[1] = 40;
    trackNote[2] = 42;
    trackNote[3] = 44;
    trackNote[4] = 46;
    trackNote[5] = 48;
    trackNote[6] = 49;
    trackNote[7] = 50;
    trackNote[8] = 51;
    trackNote[9] = 52;
    //
    memset(drumNoteLen, 6, sizeof(drumNoteLen));
    memset(trackMidiCH, (DEFAULT_MIDI_CH-1), sizeof(trackMidiCH));
    for (xm = 0; xm < NOTE_TRACKS; xm++)
    {
      trackMidiCH[DRUM_TRACKS + xm] = xm;
    }
    //
    tickOut = 2;
    tickOutLen = 2;
    muteTrack = 0;
    BPM = 120;
    seqSyncOut = false;
    accentValues[0] = 80;
    accentValues[1] = 100;
    accentValues[2] = 127;
    midiInputToPatternChannel = 8;
    writeProtectFlash = false;
    //
    #if EXTERNAL_CLOCK
      midiClockInternal = false;
    #else
      midiClockInternal = true;
    #endif
  }
};
//
extern WSTEPS stepsData[STEPS];
extern WPATTERN patternData;
extern WCONFIG configData;

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
 *    A     --
 *   F B   |  |
 *    G     --
 *   E C   |  |
 *    D     --
 *       X      .
 *       
 *       
 *  XGFEDCBA
 *  
 */

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                  XGFEDCBA
#define S_MUTE     0B01001000 
#define S_UNMUTE   0B01100011
#define S_1_       0B00110000
#define S_W1       0B01100100
#define S_W2       0B01010010
#define S_X        0B01000100
#define S_R        0B01010111
#define S_DASH     0B01000000
#define S_S        0B01101101
#define S_Y        0B01100100
#define S_L        0B00111000
#define S_O        0B00111111
#define S_U        0B00111110
#define S_G        0B00111101
#define S_F        0B01110001
#define S_o        0B01011100
#define S_C        0B00111001
#define S_N        0B00110111
#define S_E        0B01111001
#define S_t        0B01010000
#define S_n        0B01010100
#define S_K        0B01110110
#define S_r        0B01010100
#define S_T        0B00110001
#define S_d        0B01011110
#define S_c        0B01011000
#define S_A        0B01110111
#define S_b        0B01111100
#define S_P        0B01110011
#define S_I        0B00110000
#define S_H        0B01110110
#define S__        0B00001000
#define S_DOT      0B10000000
extern const byte stepChars[4];
extern const byte numbers[10];
