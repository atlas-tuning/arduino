#include "GPIOOutput.h"

#include <chrono>

GPIOOutput::GPIOOutput(std::string *name, Value* value, Value* holdTime, int pin,
                        int resistorMode, int type): Output(name) {
    this->value = value;
    this->pin = pin;
    this->resistorMode = resistorMode;

    switch(type) {
        case GPIOOUTPUT_TYPE_ANALOG:
            this->writer = GPIO_WRITER_ANALOG;
            break;
        case GPIOOUTPUT_TYPE_DIGITAL:
            this->writer = GPIO_WRITER_DIGITAL;
            break;
        case GPIOOUTPUT_TYPE_PWM:
            this->writer = GPIO_WRITER_PWM;
            break;
        default:
            throw "Unknown GPIO input type";
    }

    if (holdTime) {
        this->sendMethod = [this]() {
            long now;

            #if defined(ARDUINO)
            now = micros();
            #else
            now = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            #endif
            
            if (this->holdBegin > -1) {
                double holdMs = this->holdTime->get();
                long holdMicros = (int)(holdMs * 1000.0);
                if (now > this->holdBegin + holdMicros) {
                    this->holdBegin = -1;
                    double value = 0;
                    this->sendToHw(value);
                } else {
                    // We just continue holding
                }
            } else {
                double value = this->value->get();
                if (value > 0) {
                    this->holdBegin = now;
                    this->sendToHw(value);
                }
            }
            
            // We always tell the caller the value we are sending to hardware
            return this->sent;
        };
    } else {
        this->sendMethod = [this]() {
            double value = this->value->get();
            this->sendToHw(value);
            return this->sent;
        };
    }
}

GPIOOutput::GPIOOutput(Value* value, Value* holdTime, int pin, int resistorMode, int type):
    GPIOOutput(nullptr, value, holdTime, pin, resistorMode, type) {
}

int GPIOOutput::setup() {
    #if defined(ARDUINO)
    int mode;
    switch (this->resistorMode) {
        case GPIOOUTPUT_RESISTOR_MODE_NONE:
            mode = OUTPUT;
            break;
        case GPIOOUTPUT_RESISTOR_MODE_PULLDOWN:
            mode = PULLDOWN;
            break;
        case GPIOOUTPUT_RESISTOR_MODE_PULLUP:
            mode = PULLUP;
            break;
        default:
            throw "Unknown resistor mode";
    }

    pinMode(this->pin, mode);

    return 1;
    #else
    throw "Arduino is not supported";
    #endif
}

double GPIOOutput::send() {
    return this->sendMethod();
}