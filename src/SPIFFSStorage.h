#pragma once

#include <vector>
#include <string>

#include "Value.h"
#include "Dimension.h"
#include "Types.h"

#include "GenericStorage.h"

class SPIFFSStorage: public GenericStorage {
public:
    SPIFFSStorage(const char* filename);

    void initialize();

    int readProgramData(char** buffer);

private:
    const char* filename;
};