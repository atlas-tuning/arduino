#pragma once

#if defined(ARDUINO)
#include <Arduino.h>
#endif

#include <functional>
#include <list>

#include "Input.h"
#include "Counter.h"

#define GPIOINPUT_TYPE_PULSE 0x03

#define GPIOPULSEINPUT_RESISTOR_MODE_NONE 0x1
#define GPIOPULSEINPUT_RESISTOR_MODE_PULLDOWN 0x2
#define GPIOPULSEINPUT_RESISTOR_MODE_PULLUP 0x3

#define GPIOPULSEINPUT_EDGE_RISING 0x01
#define GPIOPULSEINPUT_EDGE_FALLING 0x02
#define GPIOPULSEINPUT_EDGE_EITHER 0x03

class GPIOPulseInput : public Input {
public:
    GPIOPulseInput(std::string* name, int pin, int resistorMode, int edge, int window);
    GPIOPulseInput(int pin, int resistorMode, int edge, int window);

    int setup();
    int read();

    void handleInterrupt();

    float readFrequency();

private:
    int pin;
    int resistorMode;
    int edge;

    long lastPulse;
    Counter* counter;
};