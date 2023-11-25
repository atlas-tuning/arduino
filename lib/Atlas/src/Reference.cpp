#include "Reference.h"

Reference::Reference(std::string* name, float (*callback)(void)): Value(name) {
    this->callback = callback;
}

Reference::Reference(float (*callback)(void)): Reference(nullptr, callback) {

}

bool Reference::isStatic() {
    return false;
}

float Reference::get() {
    return callback();
}