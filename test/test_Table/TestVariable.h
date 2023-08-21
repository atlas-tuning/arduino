#pragma once

#include <string>

#include "Value.h"

class TestVariable: public Value {
    public:
        TestVariable(std::string* name);
        TestVariable();
        
        double get();
        double set(double value);
        
        bool isStatic();
    private:
        double value;
};