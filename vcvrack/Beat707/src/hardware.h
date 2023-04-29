#ifndef HARDWARE_H
#define HARDWARE_H

// TODO:
// Data_save_load: flash.
// Interface_Menu: flash.
// TM1638: ports
// Sequencer.cpp: ISR, TCCR1A, OCR1A, TCCR1B, TIMSK1

#include <stdint.h>

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

#define bitSet(v, p) ((v) |= 1 << (p))
#define bitRead(v, p) (((v) & (1 << (p))) > 0 ? 1 : 0)
#define bitClear(v, p) ((v) &= ~(1 << (p)))

typedef uint8_t byte;

extern byte segments[3][16];
extern byte leds[3];

int random(int min, int max);

void midi_write(uint8_t data);
bool midi_available();
uint8_t midi_read();

void waitMs(int mstime);
void setup();
void loop();

void logic_init();

void sendScreen();

#endif
