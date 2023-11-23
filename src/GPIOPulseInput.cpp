#include <chrono>

#include "GPIOPulseInput.h"

#define DEBUG 1

typedef struct {
    bool enabled = false;
    GPIOPulseInput* input;
} pulse_input_lane_t;

#define MAX_LANES 8
static pulse_input_lane_t lanes[MAX_LANES] = {};

void IRAM_ATTR tick(void* input) {
    ((GPIOPulseInput*)input)->handleInterrupt();
}

void tick_lane(int laneNumber) {
    #if defined(DEBUG)
    Serial.write("Ticking pulse on lane #");
    Serial.write(std::to_string(laneNumber).c_str());
    Serial.write("...\n");
    #endif

    pulse_input_lane_t* lane = &lanes[laneNumber];
    if (lane->enabled && lane->input) {
        tick(lane->input);
    }
}

void intr0_callback() {
    tick_lane(0);
}
void intr1_callback() {
    tick_lane(1);
}
void intr2_callback() {
    tick_lane(2);
}
void intr3_callback() {
    tick_lane(3);
}
void intr4_callback() {
    tick_lane(4);
}
void intr5_callback() {
    tick_lane(5);
}
void intr6_callback() {
    tick_lane(6);
}
void intr7_callback() {
    tick_lane(7);
}

typedef void (*fptr)();
fptr get_callback(int lane) {
    switch (lane) {
        case 0:
            return &intr0_callback;
        case 1:
            return &intr1_callback;
        case 2:
            return &intr2_callback;
        case 3:
            return &intr3_callback;
        case 4:
            return &intr4_callback;
        case 5:
            return &intr5_callback;
        case 6:
            return &intr6_callback;
        case 7:
            return &intr7_callback;
        default:
            return nullptr;
    }
}

int attach(int pin, int edgeMode, GPIOPulseInput* input) {
    for (int i = 0; i < MAX_LANES; i ++) {
        pulse_input_lane_t* lane = &lanes[i];
        if (!lane->enabled) {
            lane->input = input;
            lane->enabled = true;

            #if defined(DEBUG)
            Serial.write("Attaching pulsed input pin #");
            Serial.write(std::to_string(pin).c_str());
            Serial.write(", edge=");
            Serial.write(std::to_string(edgeMode).c_str());
            Serial.write(" -> interrupt lane=");
            Serial.write(std::to_string(i).c_str());
            Serial.write(".\n");
            #endif

            attachInterrupt(digitalPinToInterrupt(pin), get_callback(i), edgeMode);

            return i;
        }
    }
    return -1;
}

long platform_get_micros() {
    #if defined(ARDUINO)
    return micros();
    #else
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    #endif
}

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

GPIOPulseInput::GPIOPulseInput(std::string* name, int pin, int resistorMode, int edge, int window):
 Input(name) {
    this->pin = pin;
    this->resistorMode = resistorMode;
    this->edge = edge;
    this->counter = new Counter(window);
    this->lastPulse = 0;

    GPIOPulseInput_Frequency* frequency = new GPIOPulseInput_Frequency(new std::string(*this->getName() + "_freq"), this);
    this->values.push_back(std::unique_ptr<Value>(frequency));
}

GPIOPulseInput::GPIOPulseInput(int pin, int resistorMode, int edge, int window):
    GPIOPulseInput(nullptr, pin, resistorMode, edge, window) {
}

double GPIOPulseInput::readFrequency() {
    long now = platform_get_micros();
    long delta = now - lastPulse;
    double average = this->counter->avg();
    if (average == 0.0) {
        return 0.0;
    } else if (delta >= this->counter->size() * average * 1000000.0) {
        return 0.0;
    } else {
        return 1.0 / average;
    }
}

void GPIOPulseInput::handleInterrupt() {
    long now = platform_get_micros();
    long delta = now - lastPulse;
    double value = (double)delta / 1000000.0; // 1uS
    if (lastPulse <= 0) {
        goto track; // skip adding this increment
    }
    this->counter->increment(value);
track:
    lastPulse = now;
}

int GPIOPulseInput::read() {
    return this->counter->size();
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
    
    pinMode(this->pin, _pinMode);

    int _edgeMode;
    switch (this->edge) {
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

    attachInterruptArg(digitalPinToInterrupt(pin), tick, this, _edgeMode);

    #else
    throw "Arduino is not supported";
    #endif

    return 1;
}