#pragma once

#include <vector>
#include <string>

#include "Value.h"
#include "Dimension.h"
#include "Types.h"

#include "PermanentStorage.h"

class FlashStorage: public PermanentStorage {
public:
    FlashStorage(int size);

    void initialize();
    Program* readProgram();

private:
    int size;
};