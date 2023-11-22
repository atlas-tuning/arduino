#pragma once

#if defined(ARDUINO)
#include <Arduino.h>
#endif

#include <functional>
#include <string>

#include "Output.h"

#define ANALOG_RESOLUTION 8

typedef std::function<void(int& pin, double& value)> GPIOWriter;

static GPIOWriter GPIO_WRITER_ANALOG = [](int& pin, double& value) { 
#if defined(ARDUINO)
    value = max(min(value, 1.0), 0.0);
    analogWrite(pin, (int)(value * (2^ANALOG_RESOLUTION)));
#else
    throw "Arduino is not supported";
#endif
};

static GPIOWriter GPIO_WRITER_PWM = [](int& pin, double& value) { 
#if defined(ARDUINO)
    value = max(min(value, 1.0), 0.0);
    analogWrite(pin, (int)(value * (2^ANALOG_RESOLUTION)));
#else
    throw "Arduino is not supported";
#endif
};

static GPIOWriter GPIO_WRITER_DIGITAL = [](int& pin, double& value) { 
#if defined(ARDUINO)
    value = max(min(value, 1.0), 0.0);
    if (value == 1.0) {
        digitalWrite(pin, 1);
    } else {
        digitalWrite(pin, 0);
    }
#else
    throw "Arduino is not supported";
#endif
};

#define GPIOOUTPUT_RESISTOR_MODE_NONE 0x1
#define GPIOOUTPUT_RESISTOR_MODE_PULLDOWN 0x2
#define GPIOOUTPUT_RESISTOR_MODE_PULLUP 0x3

#define GPIOOUTPUT_TYPE_DIGITAL 0x01
#define GPIOOUTPUT_TYPE_ANALOG 0x02
#define GPIOOUTPUT_TYPE_PWM 0x03

typedef std::function<double()> GPIOSendMethod;

class GPIOOutput : public Output {
public:
    GPIOOutput(std::string* name, Value* value, Value* holdTime, int pin, int resistorMode, int type);
    GPIOOutput(Value* value, Value* holdTime, int pin, int resistorMode, int type);

    int setup();

    double send();

private:
    int pin;
    int resistorMode;

    Value* value;
    Value* holdTime;
    GPIOWriter writer;

    GPIOSendMethod sendMethod;
    long holdBegin;

    void sendToHw(double& value) {
        this->writer(this->pin, value);
        this->sent = value;
    };

};