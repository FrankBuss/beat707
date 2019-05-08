# Development environment

First install the Arduino IDE. It was tested with Arduino 1.8.9. Then add the URL
https://mcudude.github.io/MiniCore/package_MCUdude_MiniCore_index.json to
"File->Preferences->Settings->Additional Board Manager URLs". Then you can install the
new board with "Tools->Board->Boards Manager". Install "MiniCore by MCUdude", tested with version 2.0.2.

For programming you need an USBasp. You need to reduce the clock speed in the file
`/home/username/bin/arduino-1.8.9/hardware/arduino/avr/programmers.txt`, where `username` is your
username (on Linux, on Windows and Mac it might be different, see [here](https://www.arduino.cc/en/hacking/preferences)).
Search for the `usbasp.program.extra_params` line and add a parameter:

```
usbasp.program.extra_params=-Pusb -B 10
```

For flashing the firmware, configure the following settings (Board, Clock, Variant and Bootloader needs to be changed):

![Arduino settings](doc/tools-settings.png?raw=true)

The Arduino IDE doesn't flash the ATmega fuses, so initially you need to do this from the command line with avrdude like this:

```
avrdude -cusbasp -p m328 -B 10 -U lfuse:w:0xff:m
```
The output should look like this:

```
avrdude: set SCK frequency to 93750 Hz
avrdude: AVR device initialized and ready to accept instructions

Reading | ################################################## | 100% 0.00s

avrdude: Device signature = 0x1e9514 (probably m328)
avrdude: reading input file "0xff"
avrdude: writing lfuse (1 bytes):

Writing | ################################################## | 100% 0.00s

avrdude: 1 bytes of lfuse written
avrdude: verifying lfuse memory against 0xff:
avrdude: load data lfuse data from input file 0xff:
avrdude: input file 0xff contains 1 bytes
avrdude: reading on-chip lfuse data:

Reading | ################################################## | 100% 0.00s

avrdude: verifying ...
avrdude: 1 bytes of lfuse verified

avrdude: safemode: Fuses OK (E:FF, H:D9, L:FF)

avrdude done.  Thank you.
```

For debugging, there is a jumper, which when closed connects the serial output of the ATmega to MIDI out.
You can instead open it and connect a 5V UART receiver to it with a jumper wire:

![debug pin](doc/debug-pin.jpg?raw=true)

Connect GND of the UART receiver as well. There is another pin soldered to the middle pin of the rotary encoder.
