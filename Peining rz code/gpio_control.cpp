#include "gpio_control.h"
#include <wiringPi.h>
#include <thread>

void setupGPIO() {
    wiringPiSetup();
    pinMode(LOCK_PIN, OUTPUT);
    digitalWrite(LOCK_PIN, LOW);
}

void unlockDoor() {
    digitalWrite(LOCK_PIN, HIGH);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    digitalWrite(LOCK_PIN, LOW);
}
