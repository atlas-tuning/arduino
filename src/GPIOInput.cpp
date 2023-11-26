#include "GPIOInput.h"
#include "Reference.h"
#include "Profiler.h"
#include "PlatformTime.h"

class GPIOInput_Value : public Value {
public:
      GPIOInput_Value(std::string* name, GPIOInput* input): Value(name) {
        this->input = input;
      };

      GPIOInput_Value(const char* name, GPIOInput* input): Value(new std::string(name)) {
        this->input = input;
      };
      
      float get() {
        return input->get();
      };

      bool isStatic() {
        return false;
      };
private:
    GPIOInput* input;
};

class GPIOInput_HoldTime : public Value {
public:
      GPIOInput_HoldTime(std::string* name, GPIOInput* input): Value(name) {
        this->input = input;
      };

      GPIOInput_HoldTime(const char* name, GPIOInput* input): Value(new std::string(name)) {
        this->input = input;
      };
      
      float get() {
        return input->getHoldTime();
      };

      bool isStatic() {
        return false;
      };
private:
    GPIOInput* input;
};

class GPIOInput_Delta : public Value {
public:
      GPIOInput_Delta(std::string* name, GPIOInput* input): Value(name) {
        this->input = input;
      };

      GPIOInput_Delta(const char* name, GPIOInput* input): Value(new std::string(name)) {
        this->input = input;
      };
      
      float get() {
        return input->getDelta();
      };

      bool isStatic() {
        return false;
      };
private:
    GPIOInput* input;
};

GPIOInput::GPIOInput(std::string* name, int pin, int resistorMode, int type, Value* v_gnd, Value* v_ref): Input(name) {
    this->pin = pin;
    this->resistorMode = resistorMode;
    this->v_gnd = v_gnd;
    this->v_ref = v_ref;
    
    switch(type) {
        case GPIOINPUT_TYPE_DIGITAL:
            this->reader = GPIO_READER_DIGITAL;
            break;
        case GPIOINPUT_TYPE_ANALOG:
            this->reader = GPIO_READER_ANALOG;
            break;
        default:
            throw "Unknown GPIO input type";
    }

    GPIOReader reader = this->reader;

    GPIOInput_Value* value = new GPIOInput_Value(getName(), this);
    values.push_back(std::unique_ptr<Value>(value));
    
    GPIOInput_HoldTime* holdTime = new  GPIOInput_HoldTime(new std::string(*this->getName() + "_holdTime"), this);
    values.push_back(std::unique_ptr<Value>(holdTime));
    
    GPIOInput_Delta* delta = new GPIOInput_Delta(new std::string(*this->getName() + "_delta"), this);
    values.push_back(std::unique_ptr<Value>(delta));
}

GPIOInput::GPIOInput(int pin, int resistorMode, int type, Value* v_gnd, Value* v_ref) 
    : GPIOInput::GPIOInput(nullptr, pin, resistorMode, type, v_gnd, v_ref) {
}

int GPIOInput::read() {
    PROFILE_START("gpio.read");
    float value = reader(pin);

    if (v_gnd) {
        float v_gnd_value = v_gnd->get();
        value -= v_gnd_value;

        if (value < 0) {
            value = 0;
        }
    }

    if (v_ref) {
        float v_ref_value = v_ref->get();
        value = value / v_ref_value;
    }

    this->delta = this->last - value;

    if (last != value) {
        lastChangeMicros = platform_get_micros();
        this->last = value;
    }

    PROFILE_STOP();
    return 1;
}

float GPIOInput::get() {
    return last;
}

float GPIOInput::getHoldTime() {
    return (float)(platform_get_micros() - lastChangeMicros) / 1000000.0f;
}

float GPIOInput::getDelta() {
    return delta;
}

int GPIOInput::setup() {
    PROFILE_START("gpio.setup");
    #if defined(ARDUINO)
    analogReadResolution(12);

    int mode;
    switch (this->resistorMode) {
        case GPIOINPUT_RESISTOR_MODE_NONE:
            mode = INPUT;
            break;
        case GPIOINPUT_RESISTOR_MODE_PULLDOWN:
            mode = INPUT_PULLDOWN;
            break;
        case GPIOINPUT_RESISTOR_MODE_PULLUP:
            mode = INPUT_PULLUP;
            break;
        default:
            Serial.write("Invalid resistor mode: ");
            Serial.write(std::to_string(this->resistorMode).c_str());
            Serial.write("\n");
            return -EINVAL;
    }

    pinMode(pin, mode);
    lastChangeMicros = platform_get_micros();

    PROFILE_STOP();

    return 1;
    #else
    throw "Arduino is not supported";
    #endif
}