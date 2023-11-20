#pragma once

#include <vector>
#include <string>

#include "Value.h"
#include "Dimension.h"
#include "Types.h"

#include "PermanentStorage.h"

class GenericStorage: public PermanentStorage {
public:
    GenericStorage();

    virtual int readProgramData(char** buffer) = 0;

    Program* readProgram();

private:
    int bufferSize;
    int size;
};