#pragma once

#include "StateTable.h"

class NVSStateTable: public StateTable {
public:
    NVSStateTable(std::string* name, v_dimension *dimensions, std::vector<float> *data);
    float setStateValue(float newValue);
    float getStateValue();
private:
    float stateValue;
};