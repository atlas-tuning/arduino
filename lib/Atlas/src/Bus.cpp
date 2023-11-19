#include "Bus.h"

Bus::Bus(std::string* name) {
    this->name = name;
}

std::string* Bus::getName() {
    return this->name;
}