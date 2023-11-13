#pragma once

#include "Value.h"

class Input {
public:
    Input(std::string* name);
    Input();

    virtual int setup() = 0;
    virtual int read() = 0;

    const v_value* getValues();
    Value* getPrimaryValue();

    std::string* getName();
protected:
    v_value values;
private:
    std::string* name;
};

typedef std::vector<Input*> v_input;