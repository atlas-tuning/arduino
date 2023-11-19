#include "Bus.h"

Bus::Bus(std::string* name) {
    this->name = name;
    this->inputs = new v_input();
}

std::string* Bus::getName() {
    return this->name;
}

v_input* Bus::getInputs() {
    return this->inputs;
}
