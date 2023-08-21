#include "Input.h"

Input::Input(std::string* name) {
    this->name = name;
}

Input::Input() {
    
}

std::string* Input::getName() {
    return this->name;
}

const v_value* Input::getValues() {
    return &this->values;
}

Value* Input::getPrimaryValue() {
    return this->values.at(0).get();
}