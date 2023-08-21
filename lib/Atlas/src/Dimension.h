#pragma once

#include "Value.h"
#include "Functions.h"
#include "Types.h"

class Dimension {
public:
    Dimension(Value* source, Integration* integration, v_double* anchors);

    Value* getSource();
    Integration* getIntegration();
    v_double* getAnchors();
    int getSize();
private:
    Value *source;
    Integration *integration;
    v_double *anchors;
};

typedef std::vector<Dimension*> v_dimension;