#include "FeedbackTable.h"

FeedbackTable::FeedbackTable(std::string* name, v_dimension *dimensions, std::vector<float> *data,
    Value* real, Value* target, std::vector<float> *feedback, int window): Table(name, dimensions, data) {
    this->real = real;
    this->target = target;
    this->correction = new Table(nullptr, dimensions, feedback);

    this->accumulator = new std::vector<float>(feedback->size());
    this->accumulator->assign(feedback->begin(), feedback->end());

    this->samples = new std::vector<int>(feedback->size());
    this->samples->assign(feedback->size(), 1);

    this->window = window;
}

Table* FeedbackTable::getCorrectionTable() {
    return this->correction;
}

float FeedbackTable::integrate(v_float const &coordinates) {
    // Get the real and target values (sensor or otherwise)
    float real = this->real->get();
    float target = this->target->get();

    // Do base integration of the table output
    float output = Table::integrate(coordinates);

    // Calculate the error value
    float error = (target - real) / target;
    
    // Update the buffer
    v_int index = Table::getDataIndex(coordinates);
    int offset = Table::getDataOffset(index);
    float& accumulated = this->accumulator->at(offset);
    int& samples = this->samples->at(offset);
    accumulated += error;

    // Calculate the error correction value and update the table accordingly
    if (samples >= this->window) {
        float average = accumulated / samples;
        accumulated -= average;
        this->correction->setData(offset, accumulated);
    } else {
        samples ++;
    }

    float correction = this->correction->get();

    // Multiply the output of this table with the correction value
    return output * correction;
}