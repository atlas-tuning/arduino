#include "GPIOInput.h"
#include "Reference.h"
#include "Profiler.h"

class GPIOInput_Value : public Value {
public:
      GPIOInput_Value(std::string* name, GPIOInput* input): Value(name) {
        this->input = input;
      };
      
      double get() {
        return input->get();
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
}

GPIOInput::GPIOInput(int pin, int resistorMode, int type, Value* v_gnd, Value* v_ref) 
    : GPIOInput::GPIOInput(nullptr, pin, resistorMode, type, v_gnd, v_ref) {
}

int GPIOInput::read() {
    PROFILE_START("gpio.read");
    double value = reader(pin);

    if (v_gnd) {
        double v_gnd_value = v_gnd->get();
        value -= v_gnd_value;

        if (value < 0) {
            value = 0;
        }
    }

    if (v_ref) {
        double v_ref_value = v_ref->get();
        value = value / v_ref_value;
    }

    last = value;
    PROFILE_STOP();
    return 1;
}

double GPIOInput::get() {
    return last;
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
    PROFILE_STOP();

    return 1;
    #else
    throw "Arduino is not supported";
    #endif
}