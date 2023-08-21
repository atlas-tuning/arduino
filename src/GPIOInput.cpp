#include "GPIOInput.h"
#include "Reference.h"

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

GPIOInput::GPIOInput(std::string* name, int pin, int resistorMode, int type): Input(name) {
    this->pin = pin;
    this->resistorMode = resistorMode;
    
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
}

GPIOInput::GPIOInput(int pin, int resistorMode, int type): GPIOInput::GPIOInput(nullptr, pin, resistorMode, type) {
}

int GPIOInput::read() {
    this->last = this->reader(this->pin);
    return 1;
}

double GPIOInput::get() {
    return this->last;
}

int GPIOInput::setup() {
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
            throw "Unknown resistor mode";
    }

    pinMode(this->pin, mode);

    GPIOReader reader = this->reader;
    GPIOInput_Value* value = new GPIOInput_Value(this->getName(), this);

    this->values.push_back(std::unique_ptr<Value>(value));

    return 1;
    #else
    throw "Arduino is not supported";
    #endif
}