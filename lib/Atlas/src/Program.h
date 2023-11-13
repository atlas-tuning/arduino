#pragma once

#include "Input.h"
#include "Output.h"
#include "Table.h"
#include "Types.h"

class Program {
public:
    Program(v_input *inputs, v_output *outputs, v_table *tables);

    v_input* getInputs();
    v_output* getOutputs();
    v_table* getTables();

private:
    v_input *inputs;
    v_output *outputs;
    v_table *tables;
};

