#include "CANBus.h"

CANBus::CANBus(std::string* name, int txPin, int rxPin, int kbits) : Bus(name) {
    this->txPin = txPin;
    this->rxPin = rxPin;
    this->kbits = kbits;
}

int CANBus::setup() {
    #ifdef ARDUINO
    // See: https://docs.espressif.com/projects/esp-idf/en/release-v3.3/api-reference/peripherals/can.html
    // Using macros here, for more configurability we could make structs manually.

    can_general_config_t g_config = CAN_GENERAL_CONFIG_DEFAULT(
            (gpio_num_t)this->txPin,
            (gpio_num_t)this->rxPin,
            CAN_MODE_NORMAL
    );

    can_timing_config_t t_config;
    switch(this->kbits) {
        case 25:
            t_config = CAN_TIMING_CONFIG_25KBITS();
            break;
        case 50:
            t_config = CAN_TIMING_CONFIG_50KBITS();
            break;
        case 100:
            t_config = CAN_TIMING_CONFIG_100KBITS();
            break;
        case 125:
            t_config = CAN_TIMING_CONFIG_125KBITS();
            break;
        case 250:
            t_config = CAN_TIMING_CONFIG_250KBITS();
            break;
        case 500:
            t_config = CAN_TIMING_CONFIG_500KBITS();
            break;
        case 800:
            t_config = CAN_TIMING_CONFIG_800KBITS();
            break;
        case 1000:
            t_config = CAN_TIMING_CONFIG_1MBITS();
            break;
        default:
            throw "Unsupported timing configuration";
    }

    can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();

    int res = can_driver_install(&g_config, &t_config, &f_config);

    // TODO setup inputs?

    return res;
    #else
    return -ENODEV;
    #endif
}

int CANBus::begin() {
    #ifdef ARDUINO
    return can_start();
    #else
    return -ENODEV;
    #endif
}

int CANBus::update() {
    #ifdef ARDUINO
    int num_recv = 0;
    can_message_t message;
    int res;

    while ((res = can_receive(&message, pdMS_TO_TICKS(0))) == ESP_OK) {
        // TODO handle canbus messages
        num_recv++;
    }

    if (res < 0 && num_recv <= 0) {
        return res;
    } else {
        return num_recv;
    }
    #else
    return -ENODEV;
    #endif
}

int CANBus::end() {
    #ifdef ARDUINO
    return can_stop();
    #else
    return -ENODEV;
    #endif
}