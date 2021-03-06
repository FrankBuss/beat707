/*
 * 
 * Created by William Kalfelz @ Beat707 (c) 2018 - http://www.Beat707.com
 * 
 */

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void reset();
void resetPatternBank();
void waitMs(int mstime);
void checkIfDataNeedsSaving();
void eraseSector(uint16_t _pagePos);
void saveConfigData(byte _eraseSector);
void savePatternData(byte _eraseSector);
void saveStepsData();
void flashInit(bool force);
void saveHeader(bool sectorErase);
void createFlashHeader();
void initPatternBank(byte patternBank, bool sectorErase, int &porc, bool patternBankOnly);
void loadPattern(byte pattern, bool force = false);
void checkPatternStream();
void loadPatternBank(byte patternBank);
void changePattern(char thedirection);
void initTM1638();
void sendData(byte data);
void sendDataConst(byte data1, byte data2, byte data3);
void sendScreen();
void readButtons(void);
void printNumber(byte segment, byte offset, int number);
void showErrorMsg(byte error);
void showErrorMsg(byte error, bool errors);
void showWaitMsg(char porcentage);
void printDashDash(byte segment, byte offset);
void printMIDInote(byte note, byte segment, byte offset, byte offsetOctave);
void sendScreenAndWait(int wait);
void splashScreen();
void createScreen();
void checkInterface();
void checkIfMirrorAndCopy(byte thestep, byte track);
void clearStepsExtrasBits(byte thestep, byte xVar, byte track);
void resetSegments(byte xs, byte xe);
void printStopSequencer();
void getStepVelocity(byte theStep, byte track, byte variation, bool cleanBits);
void getNoteStepGlideDoubleOff(byte theStep, byte track, byte variation, bool cleanBits);
void showMenuCopyPaste();
void processMenuCopyPaste(byte button);
void showMenu();
void processMenuOK();
void processMenu(char value);
void checkMenuClose();
void showOnOrOff(bool showOn);
void showMemoryProtected();
void handleMIDIInput();
void createScreenMute();
void checkInterfaceMute();
void outputMIDIBuffer();
void sendOutputMIDIBuffer();
void doTickSequencer();
void ledsVarSeqUpdate();
void ledsBufferFlip();
void trackNoteOn(byte xtrack, byte xnote, byte xvelocity);
void noteTrackNoteOn(byte xtrack, byte xnote, byte xvelocity, bool slide);
void resetSequencer();
void stopDrumTrackPrevNote(byte track, bool isDrumTrack);
void startSequencer(bool docontinue);
void startTimer(bool force);
void stopTimer(bool force);
void updateSequencerSpeed(bool force);
void setupTimerForExternalMIDISync(bool active);
void stopSequencer(void);
void MIDIallNotesOff();
void sendMidiEvent(byte type, byte byte1, byte byte2, byte channel, byte slot);
void startMIDIinterface();
void sendFinalZero();
bool sendValueCheckZero(byte value);
void send4BitsBytes(byte value);
byte receive4BitsBytes();
void updatePorc(byte value);
void ShowTemporaryMessage(byte message);
byte getMPVelocity(byte xproc, byte xorgvelocity);
char stepBeenHold();
void getTimer1SeqOCR(float divider);
//
void addRecordNotes(byte data1, byte data2, byte channel);
void recordInputCheck(byte data1, byte data2, byte channel, byte track);
//
#if DEBUG_MIDI_INPUT | TRACK_DEBUG_MIDI
  void printNumber2(byte segment, byte offset, byte number);
#endif
//
#if DEBUG_MIDI_INPUT
  void debugCreateScreen();
  void debugCheckInterface();
  void debugCheckMIDIInput();
#endif
//
void sendMIDIProgramChange(byte track);
void sendMIDICC(byte track, bool sendNow, byte slot);
void resetProgramChangeAndCC();
void checkIfMidiCCok();
void checkIfEchoOK();
void pulseOut(bool enable);
char getBankLetter(byte value);
