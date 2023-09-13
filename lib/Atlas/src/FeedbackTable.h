#pragma once

#include "Table.h"
#include "Value.h"

class FeedbackTable: public Table {
public:
    FeedbackTable(std::string* name, v_dimension *dimensions, std::vector<double> *data,
                    Value* real, Value* target, std::vector<double> *feedback, int window);

    double integrate(v_double const &coordinates) override;

    Table* getCorrectionTable();
private:
    Value* real;
    Value* target;
    Table* correction;

    std::vector<double> *accumulator;
    std::vector<int> *samples;

    int window;
};