#include "Value.h"

#include <string>

Value::Value(std::string* name) {
    this->name = name;
}

Value::Value() {
}

std::string* Value::getName() { 
    return name;
}