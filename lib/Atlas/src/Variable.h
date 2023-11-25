#pragma once

#include <string>

#include "Value.h"

class Variable: public Value {
    public:
        Variable(std::string* name, float value);
        Variable(float value);
        
        float get();
        bool isStatic();
    private:
        float value;
};