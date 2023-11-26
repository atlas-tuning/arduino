#include "GPIOOutput.h"
#include "Profiler.h"

#include <chrono>

#if defined(ESP32)

#include "esp32-hal.h"
#ifdef SOC_LEDC_SUPPORT_HS_MODE
#define LEDC_CHANNELS           (SOC_LEDC_CHANNEL_NUM<<1)
#else
#define LEDC_CHANNELS           (SOC_LEDC_CHANNEL_NUM)
#endif

typedef struct  {
    bool bound = false;
    uint8_t channel = -1;
    uint32_t frequency = -1;
    uint32_t resolution = -1;
    uint32_t max_duty = 0;
    uint32_t duty = -1;
} esp32_ledc_channel_config;

static esp32_ledc_channel_config pin_to_channel[SOC_GPIO_PIN_COUNT];
static uint32_t esp32_ledc_default_frequency = 1000;

/**
 * Selects the ideal bit depth for the ESP32 LEDC channel's resolution based on the PWM frequency desired.
 */
uint32_t ledc_select_resolution(uint32_t frequency) {
    // See: https://www.reddit.com/r/esp32/comments/cnpy5f/question_about_pwm_frequency_vs_bit_depth/
    if (frequency < 1220) {
        return 16;
    } else if (frequency < 2441) {
        return 15;
    } else if (frequency < 4882) {
        return 14;
    } else if (frequency < 9765) {
        return 13;
    } else if (frequency < 19531) {
        return 12;
    } else if (frequency < 39062) {
        return 11;
    } else if (frequency < 78125) {
        return 10;
    } else if (frequency < 156250) {
        return 9;
    } else if (frequency < 312500) {
        return 8;
    } else {
        return 8;
    }
}

uint8_t ledc_next_channel() {
    uint8_t selected = 0;

    for (uint8_t selected = 0; selected < LEDC_CHANNELS; selected ++) {
        bool in_use = false;

        for (int pin = 0; pin < SOC_GPIO_PIN_COUNT; pin++) {
            esp32_ledc_channel_config* channel = &pin_to_channel[pin];
            if (channel->channel >= 0 && channel->channel == selected) { 
                in_use = true;
                break;
            }
        }

        if (!in_use) {
            return selected;
        }
    }

    return -1;
}
#endif

static GPIOWriter GPIO_WRITER_ANALOG = [](int& pin, float& value, float& frequency) { 
    value = max(min(value, 1.0f), 0.0f);
#if defined(ESP32)
    // TODO
#elif defined(ARDUINO)
    analogWrite(pin, (int)(value * (2^ANALOG_RESOLUTION)));
    analogGetChannel(pin);
#else
    throw "Board is not supported";
#endif
};

static GPIOWriter GPIO_WRITER_PWM = [](int& pin, float& value, float& frequency) {
    if (pin < 0 || pin > SOC_GPIO_PIN_COUNT) {
        return;
    }

    value = max(min(value, 1.0f), 0.0f);

#if defined(ESP32)
    esp32_ledc_channel_config* channel = &pin_to_channel[pin];
    if (!channel->bound) {
        uint8_t channel_number = ledc_next_channel();
        if (channel_number < 0) { // failure
            return;
        }

        channel->channel = channel_number;
        channel->bound = true;

        #ifdef DEBUG
        Serial.write("Attached LEDC channel #");
        Serial.write(std::to_string(channel_number).c_str());
        Serial.write(" to pin #");
        Serial.write(std::to_string(pin).c_str());
        Serial.write(".\n");
        #endif
    }

    if (channel->resolution <= 0 || channel->frequency != (uint32_t) frequency) {
        uint32_t ledc_frequency = esp32_ledc_default_frequency;
        if (frequency > 0) {
            ledc_frequency = (uint32_t) frequency;
        }

        uint32_t resolution = ledc_select_resolution(ledc_frequency);
        ledc_frequency = ledcSetup(channel->channel, ledc_frequency, resolution);
        if (ledc_frequency <= 0) {
            // failure
            return;
        }

        ledcAttachPin(pin, channel->channel);

        channel->frequency = ledc_frequency;
            channel->resolution = resolution;
            channel->max_duty = (0x1 << resolution) - 1;

            #ifdef DEBUG
            Serial.write("Changed LEDC setup ch #");
            Serial.write(std::to_string(channel->channel).c_str());
            Serial.write(", pin #");
            Serial.write(std::to_string(pin).c_str());
            Serial.write(", freq=");
            Serial.write(std::to_string(ledc_frequency).c_str());
            Serial.write("Hz, resolution=");
            Serial.write(std::to_string(resolution).c_str());
            Serial.write(" bits, max_duty=");
            Serial.write(std::to_string(channel->max_duty).c_str());
            Serial.write(".\n");
            #endif
    }

    uint32_t duty = (uint32_t) (value * channel->max_duty);
    if (channel->duty != duty) {
        ledcWrite(channel->channel, duty);
        channel->duty = duty;

        #ifdef DEBUG
        Serial.write("Changed LEDC duty ch #");
        Serial.write(std::to_string(channel->channel).c_str());
        Serial.write(", pin #");
        Serial.write(std::to_string(pin).c_str());
        Serial.write(", duty=");
        Serial.write(std::to_string(duty).c_str());
        Serial.write("/");
        Serial.write(std::to_string(channel->max_duty).c_str());
        Serial.write(".\n");
        #endif
    }
#elif defined(ARDUINO)
    analogWrite(pin, (int)(value * (2^ANALOG_RESOLUTION)));
#else
    throw "Arduino is not supported";
#endif
};

static GPIOWriter GPIO_WRITER_DIGITAL = [](int& pin, float& value, float& frequency) { 
#if defined(ARDUINO)
    value = max(min(value, 1.0f), 0.0f);
    if (value == 1.0) {
        digitalWrite(pin, 1);
    } else {
        digitalWrite(pin, 0);
    }
#else
    throw "Arduino is not supported";
#endif
};

GPIOOutput::GPIOOutput(std::string *name, Value* value, Value* holdTime, Value* frequency,
                     Value* updateFrequency,
                     int pin, int resistorMode, int type): Output(name) {
    this->value = value;
    this->pin = pin;
    this->resistorMode = resistorMode;
    this->frequency = frequency;
    this->updateFrequency = updateFrequency;
    this->lastUpdate = 0;

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
                float holdMs = this->holdTime->get();
                long holdMicros = (int)(holdMs * 1000.0);
                if (now > this->holdBegin + holdMicros) {
                    this->holdBegin = -1;
                    float value = 0;
                    this->sendToHw(value);
                } else {
                    // We just continue holding
                }
            } else {
                float value = this->value->get();
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
            if (this->updateFrequency != nullptr) {
                long now = platform_get_micros();
                float frequency = this->updateFrequency->get();
                float secondsPerUpdate = 1.0f / frequency;
                long wait = (long)(secondsPerUpdate * 1000000.0);
                if (now - this->lastUpdate < wait) {
                    return this->sent;
                }
            }

            float value = this->value->get();
            this->sendToHw(value);
            return this->sent;
        };
    }
}

GPIOOutput::GPIOOutput(Value* value, Value* holdTime, Value* frequency, 
                     Value* updateFrequency, 
                     int pin, int resistorMode, int type):
    GPIOOutput(nullptr, value, holdTime, frequency, updateFrequency, pin, resistorMode, type) {
}

int GPIOOutput::setup() {
    PROFILE_START("gpio.seup");
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
    PROFILE_STOP();

    return 1;
    #else
    throw "Arduino is not supported";
    #endif
}

float GPIOOutput::send() {
    PROFILE_START("gpio.send");
    float sent = this->sendMethod();
    PROFILE_STOP();
    return sent;
}