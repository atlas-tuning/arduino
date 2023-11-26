#include "NVSStateTable.h"

#include "Preferences.h"

Preferences preferences;

NVSStateTable::NVSStateTable(std::string* name, v_dimension *dimensions, std::vector<float> *data):
    StateTable(name, dimensions, data) {
    preferences.begin("atlas");
    this->stateValue = preferences.getFloat(name->c_str(), 0.0f);
    preferences.end();
}

float NVSStateTable::setStateValue(float value) {
    float oldValue = stateValue;
    if (stateValue != value) {
        stateValue = value;
        int res;
        preferences.begin("atlas");
        if ((res = preferences.putFloat(getName()->c_str(), value)) <= 0) {
            Serial.write("Failed to persist state value: ");
            Serial.write(getName()->c_str());
            Serial.write("=");
            Serial.write(std::to_string(value).c_str());
            Serial.write(": ");
            Serial.write(std::to_string(res).c_str());
            Serial.write("\n");
        }
        preferences.end();
    }
    return oldValue;
}

float NVSStateTable::getStateValue() {
    return stateValue;
}