#pragma once

#include "Table.h"

/**
 * Describes a table which stores a single state variable that updates based on the
 * output cell of the table.
 * 
 * For a given output cell value,
 * 
 *  CONDITION         OPERATION
 *   n (positive)     the state variable will be updated to the value in the cell
 *   0                the state variable will be updated to zero (0)
 *  -1                the state variable will not be updated; no-op
 *  -2                the state variable will be incremented by one
 *  -3                the state variable will be decremented by one
 *  -4                the state variable will be squared v = (v ^ 2)
 *  -5                the state variable will be divided: v = 1 / v
 *  -6                the state variable will be the square root: v = sqrt(v)
 *  -7                the state variable will be the time, in seconds, since startup with high resolution
 *  -8                the output will be the difference in time between the stored variable and the current time
 * 
 * After an operation is complete, the state variable will be returned from the table unless otherwise specified
*/
class StateTable: public Table {
public:
    StateTable(std::string* name, v_dimension *dimensions, std::vector<float> *data);

    virtual float setStateValue(float newValue) = 0;
    virtual float getStateValue() = 0;

    float integrate(v_float const &coordinates) override;
    float integrate_stateless(v_float const &coordinates) override;
};