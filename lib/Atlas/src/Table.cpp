#include <algorithm>
#include <math.h>
#include <cstdint>

#include "Table.h"

Table::Table(std::string* name, v_dimension *dimensions, std::vector<double> *data): Value(name) {
    this->dimensions = dimensions;
    this->data = data;
}

bool Table::isStatic() {
    return all_of(
        this->dimensions->begin(), 
        this->dimensions->end(),
        [](Dimension* d){
            return d->getSource()->isStatic();
        });
}

v_dimension* Table::getDimensions() {
    return this->dimensions;
}

Dimension* Table::getDimension(int index) {
    v_dimension dims = *this->dimensions;
    return dims.at(index);
}

int Table::numDimensions() {
    return this->dimensions->size();
}

double Table::getData(v_int const &coordinates) {
    int index = 0;
    int dimensions = this->dimensions->size();

    for (int i = 0; i < dimensions; i ++) {
        if (i == 0) {
            index += coordinates[i];
        } else {
            int precedingDimensionLength = 1;
            for (int d = 0; d < i; d ++) {
                precedingDimensionLength *= this->getDimension(d)->getSize();
            }
            index += precedingDimensionLength * coordinates[i];
        }
    }

    return this->data->at(index);
}

int calculateCornerIndex(v_int *cornerIndices) {
    int index = 0;
    for (int i = 0; i < cornerIndices->size(); i ++) {
        index = index | (int8_t)((cornerIndices->at(i) & 0x1) << i);
    }
    return index;
}

v_int Table::getDataIndex(v_int const &cornerIndices, v_int const &lowIndices, v_int const &highIndices) {
    v_int dataIndex;

    for (int d = 0; d < numDimensions(); d ++) {
        int cornerIndex = cornerIndices[d];
        dataIndex.push_back(cornerIndex == 0 ? lowIndices[d] : highIndices[d]);
    }

    return dataIndex;
}

v_double Table::fill(v_int const &lowIndices, v_int const &highIndices) {
    int cornerCount = pow(2, numDimensions());
    v_double corners;
    corners.resize(cornerCount);
    fill(corners, lowIndices, highIndices);
    return corners;
}

void Table::fill(v_double& corners, v_int const &lowIndices, v_int const &highIndices) {
    v_int cornerIndices;
    cornerIndices.resize(numDimensions());
    fill(0, cornerIndices, corners, lowIndices, highIndices);
}

void Table::fill(int dimIndex, v_int &cornerIndices, v_double& corners, v_int const &lowIndices, v_int const &highIndices) {
    for (int i = 0; i < 2; i++) {
        if (dimIndex == 0) {
            std::fill(cornerIndices.begin(), cornerIndices.end(), 0);
        }
        cornerIndices[dimIndex] = i;
        if (dimIndex == numDimensions() - 1) {
            v_int dataIndex = getDataIndex(cornerIndices, lowIndices, highIndices);
            int cornerIndex = calculateCornerIndex(&cornerIndices);
            corners.at(cornerIndex) = getData(dataIndex);
        } else {
            fill(dimIndex + 1, cornerIndices, corners, lowIndices, highIndices);
        }
    }
}

v_double Table::reduce(v_int const &lowIndices, v_int const &highIndices, v_double const &gradients) {
    v_double corners = fill(lowIndices, highIndices);
    return reduce(corners, gradients, 0);
}

v_double Table::reduce(v_double& corners, v_double const &gradients, int dimIndex) {
    Dimension* dimension = getDimension(dimIndex);

    v_double pairs;
    pairs.resize(corners.size() / 2);

    for (int i = 0; i < corners.size(); i += 2) {
        double a = corners[i];
        double b = corners[i+1];

        if (a == b) {
            pairs[i/2] = a;
        } else {
            double g = gradients[dimIndex];
            Integration integration = *dimension->getIntegration();
            pairs[i / 2] = integration(a, b, g);
        }
    }

    if (pairs.size() == 1) {
        return pairs;
    } else {
        return reduce(pairs, gradients, dimIndex + 1);
    }
}

double Table::integrate(v_double const &coordinates) {
    int numDimensions = this->numDimensions();
    v_int lowIndices, highIndices;
    v_double gradients;
    lowIndices.resize(numDimensions);
    highIndices.resize(numDimensions);
    gradients.resize(numDimensions);

    // Identify the low and high portion of the cells
    for (int dimIndex = 0; dimIndex < numDimensions; dimIndex ++) {
        Dimension* dimension = getDimension(dimIndex);
        v_double* anchors = dimension->getAnchors();
        double coordinate = coordinates.at(dimIndex);

        int lowIndex = -1, highIndex = -1;
        for (int index = 0; index < anchors->size(); index++) {
            double anchor = anchors->at(index);
            if (anchor <= coordinate) {
                lowIndex = index;
                highIndex = index;
            }
            if (anchor == coordinate) {
                break;
            }
            if (anchor > coordinate) {
                lowIndex = index -1;
                highIndex = index;
                break;
            }
        }

        // Clamp to the bounds of the table
        lowIndex = std::max(0, std::min((int)anchors->size() - 1, lowIndex));
        highIndex = std::max(0, std::min((int)anchors->size() - 1, highIndex));

        if (lowIndex == highIndex) {
            gradients[dimIndex] = 1.0;
            lowIndices[dimIndex] = lowIndex;
            highIndices[dimIndex] = highIndex;
        } else {
            double lowValue = anchors->at(lowIndex);
            double highValue = anchors->at(highIndex);
            lowIndices[dimIndex] = lowIndex;
            highIndices[dimIndex] = highIndex;

            if (lowValue == highValue) {
                gradients[dimIndex] = 1.0;
            } else {
                gradients[dimIndex] = (coordinate - lowValue) / (highValue - lowValue);
            }
        }
    }

    v_double reduced = reduce(lowIndices, highIndices, gradients);
    return reduced[0];
}

double Table::get() {
    v_double coordinates;
    int numDimensions = this->numDimensions();
    coordinates.resize(numDimensions);
    for (int dim = 0; dim < numDimensions; dim ++) 
    {
        auto dimension = this->getDimension(dim);
        coordinates.at(dim) = dimension->getSource()->get();
    }

    return integrate(coordinates);
}