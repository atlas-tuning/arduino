#include "Counter.h"

Counter::Counter(int window) {
    this->window = window;
    this->buffer = 0.0;
    this->count = 0;
}

void Counter::increment(double value) {
    if (this->count >= this->window) {
        this->buffer -= this->avg();
        this->buffer += value;
    } else {
        this->buffer += value;
        this->count += 1;
    }
}

void Counter::clear() {
    this->buffer = 0.0;
    this->count = 0;
}

double Counter::avg() {
    if (this->count <= 0) {
        return 0;
    }

    return this->buffer / (double)this->count;
}

double Counter::sum() {
    return this->buffer;
}

int Counter::size() {
    return this->count;
}

int Counter::getWindow() {
    return this->window;
}