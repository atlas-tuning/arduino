#include "Variable.h"
#include <string>

Variable::Variable(std::string* name, float value): Value(name) {
    this->value = value;
}

Variable::Variable(float value): Value() {
    this->value = value;
}

bool Variable::isStatic() {
    return true;
}

float Variable::get() {
    return this->value;
}