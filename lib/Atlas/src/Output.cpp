#include "Output.h"

Output::Output(std::string* name) {
    this->name = name;
}

Output::Output() {

}

float Output::getLastSent() {
    return this->sent;
}

std::string* Output::getName() {
    return this->name;
}