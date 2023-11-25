#pragma once

#include <string>

#include "Value.h"

class TestVariable: public Value {
    public:
        TestVariable(std::string* name);
        TestVariable();
        
        float get();
        float set(float value);
        
        bool isStatic();
    private:
        float value;
};