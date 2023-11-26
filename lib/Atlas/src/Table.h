#pragma once

#include <vector>
#include <string>

#include "Value.h"
#include "Dimension.h"
#include "Types.h"

class Table: public Value {
public:
    Table(std::string* name, v_dimension *dimensions, std::vector<float> *data);

    float get();
    float get_stateless();
    bool isStatic();
    int numDimensions();

    Dimension* getDimension(int index);
    v_dimension* getDimensions();
    
    float setData(int offset, float value);
    float setData(v_int const &coordinates, float value);
    float setData(v_float const &coordinates, float value);

    float getData(int offset);
    float getData(v_int const &coordinates);
    int getDataOffset(v_int const &coordinates);
    void getDataIndex(v_int const &cornerIndices, v_int const &lowIndices, v_int const &highIndices, v_int &dataIndex);
    v_int getDataIndex(v_float const &coordinates);

    v_float fill(v_int const &lowIndices, v_int const &highIndices);
    void fill(v_float& corners, v_int const &lowIndices, v_int const &highIndices);
    void fill(int dimIndex, int cornerIndex, v_int &cornerIndices, v_float& corners, v_int const &lowIndices, v_int const &highIndices);

    v_float reduce(v_int const &lowIndices, v_int const &highIndices, v_float const &gradients);
    v_float reduce(v_float& corners, v_float const &gradients, int dimIndex);

    virtual float integrate(v_float const &coordinates);
    virtual float integrate_stateless(v_float const &coordinates);

private:
    v_dimension *dimensions;
    v_float *data;
};

typedef std::vector<Table*> v_table;