#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <thread>
#include <chrono>
#include "hardware.h"

using namespace std;

int random(int min, int max) {
   return min + rand() % (max - min);
}

void midi_write(uint8_t data) {

}

bool midi_available() {
    return false;
}

uint8_t midi_read() {
    return 0;
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
        this_thread::sleep_for(chrono::milliseconds(100)); // Sleep for 10 ms (100 Hz)
    }
}

void interruptHandler() {
    while (true) {
        this_thread::sleep_for(chrono::milliseconds(100)); // Sleep for 10 ms (100 Hz)

        // Your interrupt code here
        {
            printf("i\n");
        }
    }
}

void logic_init() {
    new std::thread(setupAndLoopHandler);
    new std::thread(interruptHandler);
}
