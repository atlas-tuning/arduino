#pragma once

#include "Input.h"
#include "Output.h"
#include "Table.h"
#include "Types.h"
#include "Bus.h"

class Program {
public:
    Program(v_input *inputs, v_output *outputs, v_table *tables, v_bus *busses);

    v_input* getInputs();
    v_output* getOutputs();
    v_table* getTables();
    v_bus* getBusses();

private:
    v_input *inputs;
    v_output *outputs;
    v_table *tables;
    v_bus *busses;
};

