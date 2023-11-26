#pragma once

#include "StateTable.h"

class MemoryStateTable: public StateTable {
public:
    MemoryStateTable(std::string* name, v_dimension *dimensions, std::vector<float> *data);
    float setStateValue(float newValue);
    float getStateValue();
private:
float stateValue;
    
};