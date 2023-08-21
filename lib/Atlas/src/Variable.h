#pragma once

#include <string>

#include "Value.h"

class Variable: public Value {
    public:
        Variable(std::string* name, double value);
        Variable(double value);
        
        double get();
        bool isStatic();
    private:
        double value;
};