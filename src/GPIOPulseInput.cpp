#include <chrono>

#include "GPIOPulseInput.h"

class GPIOPulseInput_Frequency : public Value {
public:
      GPIOPulseInput_Frequency(std::string* name, GPIOPulseInput* input): Value(name) {
        this->input = input;
      };
      
      double get() {
        return input->readFrequency();
      };

      bool isStatic() {
        return false;
      };
private:
    GPIOPulseInput* input;
};

class GPIOPulseInput_HoldTime : public Value {
public:
      GPIOPulseInput_HoldTime(std::string* name, GPIOPulseInput* input): Value(name) {
        this->input = input;
      };
      
      double get() {
        return input->readHoldTime();
      };

      bool isStatic() {
        return false;
      };
private:
    GPIOPulseInput* input;
};

GPIOPulseInput::GPIOPulseInput(std::string* name, int pin, int resistorMode, int edge, 
    void (*ISR_callback)(void), int window):
 Input(name) {
    this->pin = pin;
    this->resistorMode = resistorMode;
    this->edge = edge;
    this->callback = ISR_callback;
}

GPIOPulseInput::GPIOPulseInput(int pin, int resistorMode, int edge,
    void (*ISR_callback)(void), int window):
 GPIOPulseInput(nullptr, pin, resistorMode, edge, ISR_callback, window) {
}

double GPIOPulseInput::readFrequency() {
    std::list<long> pulses = this->pulses;

    long widths, n;
    for (auto ptr = pulses.begin();;ptr++) {
        auto first = ptr;
        if (first == pulses.end()) {
            break;
        }

        auto second = ptr ++;
        if (second == pulses.end()) {
            break;
        }
        
        long width = *second - *first;
        widths += width;
        n++;
    }

    if (n == 0 || widths == 0) {
        return 0;
    }

    double usPerPulse = (double)widths / (double)n;
    return usPerPulse / 1,000,000.0;
}

void GPIOPulseInput::handleInterrupt() {
    long now;
    
    #if defined(ARDUINO)
    now = micros();
    #else
    now = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    #endif
    std::list<long> pulses = this->pulses;
    pulses.push_front(now);
    if (pulses.size() >= this->pulseLimit) {
        pulses.pop_back();
    }
}

int GPIOPulseInput::setup() {
    #if defined(ARDUINO)
    int _pinMode;
    switch (this->resistorMode) {
        case GPIOPULSEINPUT_RESISTOR_MODE_NONE:
            _pinMode = INPUT;
            break;
        case GPIOPULSEINPUT_RESISTOR_MODE_PULLDOWN:
            _pinMode = INPUT_PULLDOWN;
            break;
        case GPIOPULSEINPUT_RESISTOR_MODE_PULLUP:
            _pinMode = INPUT_PULLUP;
            break;
        default:
            throw "Unknown resistor mode";
    }

    int _edgeMode;
    switch (this->resistorMode) {
        case GPIOPULSEINPUT_EDGE_EITHER:
            _edgeMode = CHANGE;
            break;
        case GPIOPULSEINPUT_EDGE_RISING:
            _edgeMode = RISING;
            break;
        case GPIOPULSEINPUT_EDGE_FALLING:
            _edgeMode = FALLING;
            break;
        default:
            throw "Unknown resistor mode";
    }
    
    pinMode(this->pin,_pinMode);
    attachInterrupt(digitalPinToInterrupt(this->pin), callback, _edgeMode);

    GPIOPulseInput_Frequency* frequency = new GPIOPulseInput_Frequency(new std::string(*this->getName() + "_freq"), this);
    this->values.push_back(std::unique_ptr<Value>(frequency));

    GPIOPulseInput_HoldTime* holdTime = new GPIOPulseInput_HoldTime(new std::string(*this->getName() + "_holdTime"), this);
    this->values.push_back(std::unique_ptr<Value>(holdTime));
    #else
    throw "Arduino is not supported";
    #endif

    return 1;
}