#pragma once

#include <functional>

typedef std::function<double(double, double, double)> Integration;

static Integration LINEAR_INTEGRATION = [](double a, double b, double i) { return a + ((b - a) * i) ; };
static Integration FLOOR_INTEGRATION = [](double a, double b, double i) { return a; };
static Integration CEILING_INTEGRATION = [](double a, double b, double i) { return b; };