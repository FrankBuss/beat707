#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <thread>
#include <chrono>
#include "hardware.h"

using namespace std;

ByteQueue midiInQueue;
ByteQueue midiOutQueue;

int random(int min, int max) {
   return min + rand() % (max - min);
}

void midi_write(uint8_t data) {
    midiOutQueue.push(data);
}

bool midi_available() {
    return !midiInQueue.empty();
}

uint8_t midi_read() {
    return midiInQueue.pop();
}

void waitMs(int mstime)
{
     this_thread::sleep_for(chrono::milliseconds(mstime));
}

void setupAndLoopHandler() {
    // wait a bit until VCV Rack is all setup
    this_thread::sleep_for(chrono::milliseconds(1000));

    // now call the logic code
    setup();
    while (true) {
        loop();
        this_thread::sleep_for(chrono::milliseconds(1));
    }
}

#include <chrono>
#include <thread>

void interruptHandler() {
    using namespace std::chrono;
    while (true) {
        steady_clock::time_point now = steady_clock::now();
        int buffered_OCR1A = OCR1A;
        if (buffered_OCR1A > 0) {
            double desired_interval = 1.0 / buffered_OCR1A;
            interrupt();
            duration<double> elapsed_time = duration_cast<duration<double>>(steady_clock::now() - now);
            this_thread::sleep_for(duration<double>(desired_interval - elapsed_time.count()));
        } else {
            this_thread::sleep_for(milliseconds(10));
        }
    }
}

void logic_init() {
    new std::thread(setupAndLoopHandler);
    new std::thread(interruptHandler);
}
