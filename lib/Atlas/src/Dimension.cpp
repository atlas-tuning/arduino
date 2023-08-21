#include "Dimension.h"

Dimension::Dimension(Value* source, Integration* integration, v_double* anchors) {
    this->source = source;
    this->integration = integration;
    this->anchors = anchors;
}

Value* Dimension::getSource() {
    return this->source;
}

v_double* Dimension::getAnchors() {
    return this->anchors;
}

Integration* Dimension::getIntegration() {
    return this->integration;
}

int Dimension::getSize() {
    return getAnchors()->size();
}