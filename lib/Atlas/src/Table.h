#pragma once

#include <vector>
#include <string>

#include "Value.h"
#include "Dimension.h"
#include "Types.h"

class Table: public Value {
public:
    Table(std::string* name, v_dimension *dimensions, std::vector<double> *data);

    double get();
    bool isStatic();
    int numDimensions();

    Dimension* getDimension(int index);
    v_dimension* getDimensions();
    
    double getData(v_int const &coordinates);
    v_int getDataIndex(v_int const &cornerIndices, v_int const &lowIndices, v_int const &highIndices);

    v_double fill(v_int const &lowIndices, v_int const &highIndices);
    void fill(v_double& corners, v_int const &lowIndices, v_int const &highIndices);
    void fill(int dimIndex, v_int &cornerIndices, v_double& corners, v_int const &lowIndices, v_int const &highIndices);

    v_double reduce(v_int const &lowIndices, v_int const &highIndices, v_double const &gradients);
    v_double reduce(v_double& corners, v_double const &gradients, int dimIndex);

    double integrate(v_double const &coordinates);

private:
    v_dimension *dimensions;
    v_double *data;
};