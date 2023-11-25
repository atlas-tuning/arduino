#pragma once

#include <functional>

typedef std::function<float(float, float, float)> Integration;

static Integration LINEAR_INTEGRATION = [](float a, float b, float i) { return a + ((b - a) * i) ; };
static Integration FLOOR_INTEGRATION = [](float a, float b, float i) { return a; };
static Integration CEILING_INTEGRATION = [](float a, float b, float i) { return b; };