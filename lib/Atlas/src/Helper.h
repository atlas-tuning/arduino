#pragma once

#include "Dimension.h"
#include "Table.h"

#if defined(ARDUINO)
#include "Arduino.h"
#endif

 Table* newTable(std::string* name, v_dimension* dimensions, v_float* dataVector) {
    #if defined(ARDUINO)
    if (name) {
        Serial.write("Configuring table ");
        Serial.write(name->c_str());
        Serial.write("...\n");
    }
    #endif

    Table* table = new Table(name, dimensions, dataVector);
    return table;
}

v_dimension* newDimensionVec(Dimension* x, Dimension* y, Dimension* z) {
    v_dimension* dimensions = new v_dimension();
    dimensions->push_back(x);
    dimensions->push_back(y);
    dimensions->push_back(z);
    return dimensions;
}

v_dimension* newDimensionVec(Dimension* x, Dimension* y) {
    v_dimension* dimensions = new v_dimension();
    dimensions->push_back(x);
    dimensions->push_back(y);
    return dimensions;
}

v_dimension* newDimensionVec(Dimension* x) {
    v_dimension* dimensions = new v_dimension();
    dimensions->push_back(x);
    return dimensions;
}

template <size_t N> v_float* newDataVec(const float (&arr)[N]) {
    v_float* data = new v_float();
    for (int i = 0; i < N; ++i) {
        data->push_back(arr[i]);
    }
    return data;
}

template <size_t N> Dimension* newDimension(Value* source, Integration* integration, const float (&anchors)[N]) {
    return new Dimension(source, integration, newDataVec(anchors));
}