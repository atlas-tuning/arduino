#pragma once

#include "Bus.h"

#include <Arduino.h>
#include <driver/can.h>

class CANBus : public Bus {
public:
    CANBus(std::string* name, int txPin, int rxPin, int kbits);

    std::string* getName();

    /**
     * Sets up the bus
    */
    int setup();

    /**
     * Starts the bus
    */
    int begin();

    /**
     * @returns 0 if no messages were handled, positive for number of messages 
     *            handled, and negative on error.
    */
    int update();

    /**
     * Ends the bus
    */
    int end();
private:
    int txPin;
    int rxPin;
    int kbits;
};