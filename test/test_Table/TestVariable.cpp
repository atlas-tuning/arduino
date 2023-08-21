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

double TestVariable::get() {
    return this->value;
}

double TestVariable::set(double value) {
    double old = this->value;
    this->value = value;
    return old;
}