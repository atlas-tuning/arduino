#pragma once

#include <vector>
#include <string>

#include "Value.h"
#include "Dimension.h"
#include "Types.h"

#include "GenericStorage.h"

class EEPROMStorage: public GenericStorage {
public:
    EEPROMStorage(int address, int size);

    void initialize();

    int readProgramData(char* buffer, int sz);

private:
    int size;
    int address;
};