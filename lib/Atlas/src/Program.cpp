#include "Program.h"

Program::Program(v_input *inputs, v_output *outputs, v_table *tables, v_bus *busses) {
    this->inputs = inputs;
    this->outputs = outputs;
    this->tables = tables;
    this->busses = busses;
}

v_input* Program::getInputs() {
    return this->inputs;;
}

v_output* Program::getOutputs() {
    return this->outputs;
}

v_table* Program::getTables() {
    return this->tables;
}

v_bus* Program::getBusses() {
    return this->busses;
}