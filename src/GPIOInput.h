#pragma once

#if defined(ARDUINO)
#include <Arduino.h>
#endif

#include <functional>

#include "Input.h"

typedef std::function<double(int& pin)> GPIOReader;

static double MAX_12 = (double)(uint32_t)0xFFF;

static GPIOReader GPIO_READER_ANALOG = [](int& pin) { 
#if defined(ARDUINO)
    return ((double)analogReadMilliVolts(pin) / (double)1000);
#else
    throw "Arduino is not supported";
    return -1;
#endif
};

static GPIOReader GPIO_READER_DIGITAL = [](int& pin) { 
#if defined(ARDUINO)
    return digitalRead(pin);
#else
    throw "Arduino is not supported";
    return -1;
#endif
};

#define GPIOINPUT_RESISTOR_MODE_NONE 0x1
#define GPIOINPUT_RESISTOR_MODE_PULLDOWN 0x2
#define GPIOINPUT_RESISTOR_MODE_PULLUP 0x3

#define GPIOINPUT_TYPE_DIGITAL 0x01
#define GPIOINPUT_TYPE_ANALOG 0x02

class GPIOInput : public Input {
public:
    GPIOInput(std::string* name, int pin, int resistorMode, int type, Value* v_gnd, Value* v_ref);
    GPIOInput(int pin, int resistorMode, int type, Value* v_gnd, Value* v_ref);

    int setup();

    int read();
    double get();

private:
    int pin;
    int resistorMode;
    GPIOReader reader;
    double last;
    Value* v_gnd;
    Value* v_ref;
};