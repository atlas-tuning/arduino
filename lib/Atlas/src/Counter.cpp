#include "Counter.h"

Counter::Counter(int window) {
    this->window = window;
    this->buffer = 0.0;
    this->count = 0;
}

void Counter::increment(float value) {
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

float Counter::avg() {
    if (this->count <= 0) {
        return 0;
    }

    return this->buffer / (float)this->count;
}

float Counter::sum() {
    return this->buffer;
}

int Counter::size() {
    return this->count;
}

int Counter::getWindow() {
    return this->window;
}