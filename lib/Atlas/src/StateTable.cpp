#include "StateTable.h"
#include <math.h>
#include <chrono>


float integrate_state(StateTable* table, v_float const &coordinates, float stateValue, void(*set)(StateTable*, float)) {
    float cell = table->Table::integrate(coordinates);
    if (cell >= 0) {
        stateValue = cell;
        set(table, stateValue);
        return cell;
    }
    
    cell = std::floor(cell);
    if (cell == -2) {
        stateValue += 1.0f;
        set(table, stateValue);
    } else if (cell == -3) {
        stateValue -= 1.0f;
        set(table, stateValue);
    } else if (cell == -4) {
        stateValue = stateValue * stateValue;
        set(table, stateValue);
    } else if (cell == -5) {
        stateValue = 1.0f / stateValue;
        set(table, stateValue);
    } else if (cell == -6) {
        stateValue = std::sqrt(stateValue);
        set(table, stateValue);
    } else if (cell == -7) {
        long now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        float seconds = now / 1000000000.0f;
        stateValue = seconds;
        set(table, stateValue);
    } else if (cell == -8) {
        long now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        float seconds = now / 1000000000.0f;
        stateValue = seconds - stateValue;
    } else {
        // do nothing
    }

    return stateValue;
}

StateTable::StateTable(std::string* name, v_dimension *dimensions, std::vector<float> *data): Table(name, dimensions, data) {
        
}

float StateTable::integrate(v_float const &coordinates) {
    return integrate_state(this, coordinates, getStateValue(), [](StateTable* table, float value){
        table->setStateValue(value);
    });
}

float StateTable::integrate_stateless(v_float const &coordinates) {
    return integrate_state(this, coordinates, getStateValue(), [](StateTable* table, float value){});
}