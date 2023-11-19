#pragma once

#include <vector>
#include <string>

#include "Value.h"
#include "Dimension.h"
#include "Types.h"

#include "PermanentStorage.h"

class GenericStorage: public PermanentStorage {
public:
    GenericStorage(int bufferSize);

    virtual int readProgramData(char* buffer, int sz) = 0;

    Program* readProgram();

private:
    int bufferSize;
    int size;
};