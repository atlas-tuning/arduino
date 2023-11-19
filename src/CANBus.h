#pragma once

#include "Bus.h"

#include <Arduino.h>
#include <driver/can.h>

class CANBus : public Bus {
public:
    CANBus(std::string* name, int txPin, int rxPin, int kbits);

    std::string* getName();

    int setup();
    int begin();
    int end();

private:
    int txPin;
    int rxPin;
    int kbits;
};