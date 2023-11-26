#include "Dimension.h"

Dimension::Dimension(Value* source, Integration* integration, v_float* anchors) {
    this->source = source;
    this->integration = integration;
    this->anchors = anchors;
    this->lastLookupIndex = anchors->size() / 2;
}

Value* Dimension::getSource() {
    return this->source;
}

v_float* Dimension::getAnchors() {
    return this->anchors;
}

Integration* Dimension::getIntegration() {
    return this->integration;
}

int Dimension::getSize() {
    return getAnchors()->size();
}

int Dimension::getLastLookupIndex() {
    return lastLookupIndex;
}

void Dimension::setLastLookupIndex(int lastLookupIndex) {
    this->lastLookupIndex = lastLookupIndex;
}