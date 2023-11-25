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
    int getLastLookupIndex();
    void setLastLookupIndex(int lastLookupIndex);
private:
    Value *source;
    Integration *integration;
    v_float *anchors;
    int lastLookupIndex;
};

typedef std::vector<Dimension*> v_dimension;