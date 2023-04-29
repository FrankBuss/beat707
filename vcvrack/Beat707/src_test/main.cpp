#include <thread>
#include <chrono>
#include <stdio.h>
#include "hardware.h"

using namespace std;

void sendScreen() {
    printf("send screen\n");
}

int main() {
    logic_init();

    while (true) {
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
    return 0;
}
