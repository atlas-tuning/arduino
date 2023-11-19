#pragma once

#include <vector>
#include <string>

#include "Value.h"
#include "Dimension.h"
#include "Types.h"

#include "GenericStorage.h"

class SPIFFSStorage: public GenericStorage {
public:
    SPIFFSStorage(const char* filename, int bufferSize);

    void initialize();

    int readProgramData(char* buffer, int sz);

private:
    const char* filename;
};