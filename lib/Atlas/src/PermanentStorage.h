#pragma once

#include "Program.h"

class PermanentStorage {
public:
    virtual void initialize() = 0;
    virtual Program* readProgram() = 0;
};