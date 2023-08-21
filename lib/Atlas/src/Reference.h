#pragma once


#include <string>

#include <functional>
#include "Value.h"

class Reference: Value {
public:
    Reference(std::string* name, double (*callback)(void));
    Reference(double (*callback)(void));
      
    double get();
    bool isStatic();
private:
    double (*callback)(void);
};