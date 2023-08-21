#include "Variable.h"
#include <string>

Variable::Variable(std::string* name, double value): Value(name) {
    this->value = value;
}

Variable::Variable(double value): Value() {
    this->value = value;
}

bool Variable::isStatic() {
    return true;
}

double Variable::get() {
    return this->value;
}