#pragma once


#include <string>

#include <functional>
#include "Value.h"

class Reference: Value {
public:
    Reference(std::string* name, float (*callback)(void));
    Reference(float (*callback)(void));
      
    float get();
    bool isStatic();
private:
    float (*callback)(void);
};