#include "TestVariable.h"
#include <string>

TestVariable::TestVariable(std::string* name): Value(name) {
    this->value = value;
}

TestVariable::TestVariable(): Value() {
    this->value = value;
}

bool TestVariable::isStatic() {
    return true; // Not really
}

float TestVariable::get() {
    return this->value;
}

float TestVariable::set(float value) {
    float old = this->value;
    this->value = value;
    return old;
}