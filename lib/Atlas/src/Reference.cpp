#include "Reference.h"

Reference::Reference(std::string* name, double (*callback)(void)): Value(name) {
    this->callback = callback;
}

Reference::Reference(double (*callback)(void)): Reference(nullptr, callback) {

}

bool Reference::isStatic() {
    return false;
}

double Reference::get() {
    return callback();
}