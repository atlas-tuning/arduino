#include "MemoryStateTable.h"

MemoryStateTable::MemoryStateTable(std::string* name, v_dimension *dimensions, std::vector<float> *data):
    StateTable(name, dimensions, data) {
    this->stateValue = 0.0f;
}

float MemoryStateTable::setStateValue(float value) {
    float oldValue = stateValue;
    if (oldValue != value) {
        stateValue = value;
    }
    return oldValue;
}

float MemoryStateTable::getStateValue() {
    return stateValue;
}