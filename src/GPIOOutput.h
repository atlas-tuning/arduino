#pragma once

#include <functional>
#include <string>

#include "Output.h"

#if defined(ARDUINO)
#include <Arduino.h>
#endif

#define GPIOOUTPUT_RESISTOR_MODE_NONE 0x1
#define GPIOOUTPUT_RESISTOR_MODE_PULLDOWN 0x2
#define GPIOOUTPUT_RESISTOR_MODE_PULLUP 0x3

#define GPIOOUTPUT_TYPE_DIGITAL 0x01
#define GPIOOUTPUT_TYPE_ANALOG 0x02
#define GPIOOUTPUT_TYPE_PWM 0x03

typedef std::function<double()> GPIOSendMethod;
typedef std::function<void(int& pin, double& valu, double& frequency)> GPIOWriter;

class GPIOOutput : public Output {
public:
    GPIOOutput(std::string* name, Value* value, Value* holdTime, Value* frequency, int pin, int resistorMode, int type);
    GPIOOutput(Value* value, Value* holdTime, Value* frequency, int pin, int resistorMode, int type);

    int setup();

    double send();

private:
    int pin;
    int resistorMode;

    Value* value;
    Value* holdTime;
    Value* frequency;
    GPIOWriter writer;

    GPIOSendMethod sendMethod;
    long holdBegin;

    void sendToHw(double& value) {
        double frequency = -1;
        if (this->frequency) {
            frequency = this->frequency->get();
        }
        this->writer(this->pin, value, frequency);
        this->sent = value;
    };

};