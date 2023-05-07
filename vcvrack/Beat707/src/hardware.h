#ifndef HARDWARE_H
#define HARDWARE_H

// TODO:
// Data_save_load: flash.
// Interface_Menu: flash.

#include <stdint.h>
#include <queue>
#include <mutex>
#include <condition_variable>

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

#define bitSet(v, p) ((v) |= 1 << (p))
#define bitRead(v, p) (((v) & (1 << (p))) > 0 ? 1 : 0)
#define bitClear(v, p) ((v) &= ~(1 << (p)))

typedef uint8_t byte;

extern byte segments[3][16];
extern byte leds[3];
extern byte buttons[4];

extern volatile int OCR1A;

int random(int min, int max);

void midi_write(uint8_t data);
bool midi_available();
uint8_t midi_read();

void waitMs(int mstime);
void setup();
void loop();
void interrupt();

void logic_init();

void sendScreen();

class ByteQueue {
public:
    ByteQueue() {}

    void push(const uint8_t& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(value);
    }

    uint8_t pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
		uint8_t v = queue_.front();
        queue_.pop();
        return v;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

private:
    mutable std::mutex mutex_;
    std::queue<uint8_t> queue_;
};

extern ByteQueue midiInQueue;
extern ByteQueue midiOutQueue;

#endif
