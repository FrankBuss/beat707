/*
 * 
 * Created by William Kalfelz @ Beat707 (c) 2018 - http://www.Beat707.com
 * 
 */

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
SPIFlash flash;
byte segments[3][8];
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
byte lastVelocity = B00000011;
bool editingNote = false;
byte editStep = 0;
byte editVariation = 0;
byte prevPlayedNote[NOTE_TRACKS];
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
WSTEPS stepsData[STEPS];
WPATTERN patternData;
WCONFIG configData;

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
#define S_MUTE     B01001000 
#define S_UNMUTE   B01100011
#define S_1_       B00110000
#define S_W1       B01100100
#define S_W2       B01010010
#define S_X        B01000100
#define S_R        B01010111
#define S_DASH     B01000000
#define S_S        B01101101
#define S_Y        B01100100
#define S_L        B00111000
#define S_O        B00111111
#define S_U        B00111110
#define S_G        B00111101
#define S_F        B01110001
#define S_o        B01011100
#define S_C        B00111001
#define S_N        B00110111
#define S_E        B01111001
#define S_t        B01010000
#define S_n        B01010100
#define S_K        B01110110
#define S_r        B01010100
#define S_T        B00110001
#define S_d        B01011110
#define S_c        B01011000
#define S_A        B01110111
#define S_b        B01111100
#define S_P        B01110011
#define S_I        B00110000
#define S_H        B01110110
#define S__        B00001000
#define S_DOT      B10000000
const byte stepChars[4] PROGMEM = { B00000000, B00001000, B01010100, B00110111 };
const byte numbers[10] PROGMEM = { B00111111, B00000110, B01011011, B01001111, B01100110, B01101101, B01111101, B00000111, B01111111, B01100111};
