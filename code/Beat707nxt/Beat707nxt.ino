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
#define DISABLE_FLASH 1       // When set it won't use the external flash ic
#define INTRO_ANIMATION 0     // When set will show a random animation them the product name, version and company name

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#define INIT_FLASH_MEMORY 1   // Warning, this will erase the entire flash contents
#define INIT_ENTIRE_FLASH 1   // When selected with the above, the entire flash chip will be erased first. This takes around 20 seconds or less to perform before we can write data to the flash chip. If this is not set, the code will erase the flash chip by sector, which takes a bit more time to perform.
#define SHOW_FREE_RAM 0       // Will show how much RAM is left on the display during initiation
#define DEBUG_SERIAL 0        // Sets the serial output to 9600 bauds and sends some debuging information out.
#define DEBUG_MIDI_INPUT 0    // When set the whole unit will operate only to display midi input messages. You will need to set DRUM_TRACKS and NOTE_TRACKS to 1, as this Debug mode uses a lot of memory to store data. Pressing STOP clears the buffer, and pressing +/- shows next/previous event recorded. Pressing the last button on the right toggles between HEX or normal values for the messages.
#define MIDI_OVER_USB 0       // When set will use 38400 bauds for the Serial interface
#define MIDI_IN_TO_PATTERN 1  // When set it will use any MIDI NOTE Input on the selected MIDI Channel (default 1) to select the next playing pattern. Starting from MIDI_IN_START_NOTE
#define MIDI_IN_START_NOTE 24 // MIDI Input to Pattern Selection Starting Note
#define RECORD_ENABLED_ECHO 1 // Enables the midi thru echo on all input notes
#define RECORD_FORCE_VEL 1    // When set velocity will always be the maximum for note track recording
#define EXTERNAL_CLOCK 0      // When set external clock is set by default
#define EXTERNAL_CONTINUE 0   // When set will use it to continue the sequener clock
#define TRACK_DEBUG_MIDI 0    // When set the code will store the last 4 bytes of MIDI Input Message and show those values when you hold the track selector button (Bt#8 on the left).

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#include "Lib_Flash.h"
#include "Functions.h"
#include "Variables.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup()
{
  DDRC = 0xFF; // For the Pulse-Out (analog pins, A54 Header)
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
