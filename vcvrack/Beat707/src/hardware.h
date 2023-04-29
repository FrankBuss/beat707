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

int random(int min, int max);

typedef uint8_t byte;

#define bitSet(v, p) ((v) |= 1 << (p))
#define bitRead(v, p) (((v) & (1 << (p))) > 0 ? 1 : 0)
#define bitClear(v, p) ((v) &= ~(1 << (p)))

void midi_write(uint8_t data);
bool midi_available();
uint8_t midi_read();

#endif
