#pragma once

#include "Table.h"
#include "Value.h"

class FeedbackTable: public Table {
public:
    FeedbackTable(std::string* name, v_dimension *dimensions, std::vector<float> *data,
                    Value* real, Value* target, std::vector<float> *feedback, int window);

    float integrate(v_float const &coordinates) override;

    Table* getCorrectionTable();
private:
    Value* real;
    Value* target;
    Table* correction;

    std::vector<float> *accumulator;
    std::vector<int> *samples;

    int window;
};