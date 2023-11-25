#pragma once

#include "Value.h"
#include "Functions.h"
#include "Types.h"

class Dimension {
public:
    Dimension(Value* source, Integration* integration, v_float* anchors);

    Value* getSource();
    Integration* getIntegration();
    v_float* getAnchors();
    int getSize();
private:
    Value *source;
    Integration *integration;
    v_float *anchors;
};

typedef std::vector<Dimension*> v_dimension;