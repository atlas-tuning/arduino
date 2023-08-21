#pragma once

#if defined(ARDUINO)
#include <Arduino.h>
#endif

#include <functional>
#include <list>

#include "Input.h"

#define GPIOPULSEINPUT_RESISTOR_MODE_NONE 0x1
#define GPIOPULSEINPUT_RESISTOR_MODE_PULLDOWN 0x2
#define GPIOPULSEINPUT_RESISTOR_MODE_PULLUP 0x3

#define GPIOPULSEINPUT_EDGE_RISING 0x01
#define GPIOPULSEINPUT_EDGE_FALLING 0x02
#define GPIOPULSEINPUT_EDGE_EITHER 0x03

class GPIOPulseInput : public Input {
public:
    GPIOPulseInput(std::string* name, int pin, int resistorMode, int edge, 
        void (*ISR_callback)(void), int window);
    GPIOPulseInput(int pin, int resistorMode, int edge,
        void (*ISR_callback)(void), int window);

    int setup();

    void handleInterrupt();

    double readFrequency();
    double readHoldTime();

private:
    int pin;
    int resistorMode;
    int edge;

    v_value values;

    int pulseLimit;
    std::list<long> pulses;

    void (*callback)(void);
};