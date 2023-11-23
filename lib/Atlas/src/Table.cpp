#include <algorithm>
#include <math.h>
#include <cstdint>

#include "Table.h"

/**
 * Assigns a value to indices that represents the index of the Table for a given low and high index pair
 * @param dimension the dimension to assign indices from
 * @param dimensionIndex the index of the dimension provided in the Table
 * @param indices the array to assign indices to
 * @param lowIndex the "low" index within the dimension that represents the lower bound to assign against
 * @param highIndex the "high" index within the dimension that represents the upper bound to assign against
*/
void assignIndices(Dimension* dimension, int dimensionIndex, v_int* indices, int lowIndex, int highIndex) {
    v_double* anchors = dimension->getAnchors();
    
    // Clamp indices to the bounds of the dimension provided
    lowIndex = std::max(0, std::min((int)anchors->size() - 1, lowIndex));
    highIndex = std::max(0, std::min((int)anchors->size() - 1, highIndex));

    if (lowIndex == highIndex) {
        // If the low and high indices are the same, then the answer is easy; pick either one.
        (*indices)[dimensionIndex] = highIndex;
    } else {
        double lowValue = anchors->at(lowIndex);
        double highValue = anchors->at(highIndex);

        if (lowValue == highValue) {
            // If the values stored in the anchors are the same, this is an easy answer; pick either one.
            (*indices)[dimensionIndex] = lowIndex;
        } else {
            // Otherwise, pick the higher index.
            (*indices)[dimensionIndex] = highIndex;
        }
    }
}

/**
 * Finds the quantized data indices in a Table, but does so in a linear manner.
 * This function is intended to be a reference implementation for tables and
 * should not be used in production code as optimizations will improve performance.
 * 
 * One issue with this implementation is that it does not deliver a constant, or
 * near-constant lookup time.  For values deeper in the table (to the lower or right directions),
 * lookup time increases linearly in terms of clock cycles.
 * 
 * @param dimension the dimension to search
 * @param coordinate the coordinate to search for
 * @param lowIndex the low index to set for the given coordinate, when found in the dimension
 * @param highIndex the high index to set for the given coordinate, when found in the dimension.
*/
void findDimensionIndices_linear(Dimension* dimension, double coordinate,
                                int* lowIndex, int* highIndex) {
    v_double* anchors = dimension->getAnchors();
    for (int index = 0; index < anchors->size(); index++) {
        double anchor = anchors->at(index);

        if (anchor <= coordinate) {
            *lowIndex = index;
            *highIndex = index;
        }
        
        if (anchor == coordinate) {
            break;
        }

        if (anchor > coordinate) {
            *lowIndex = index -1;
            *highIndex = index;
            break;
        }
    }
}

/**
 * Finds the quantized data incies for a given Table by estimating where the data is likely
 * to be located. This function should be faster for larger tables. When a rough estimate
 * start location on a given dimension is decided, this function searches the dimension
 * dimension in the direction indicated by the linear nature of the anchor vector.
 * 
 * There are also checks for out-of-bounds coordinates to further speed up the search. In this case,
 * this function sets indices such that lowIndex == highIndex (0 or n-1 where n is the size of the
 * dimension's anchor vector). In any other case, lowIndex < highIndex, where lowIndex >= 0 and
 * highIndex <= n-1.
 * 
 * This function has a relatively more stable response time in terms of CPU cycles than a linear
 * scan.
 * 
 * @param dimension the dimension to search
 * @param coordinate the coordinate to search for
 * @param lowIndex the low index to set for the given coordinate, when found in the dimension
 * @param highIndex the high index to set for the given coordinate, when found in the dimension.
*/
void findDimensionIndices_estimate(Dimension* dimension, double coordinate,
                                    int* lowIndex, int* highIndex) {
    v_double* anchors = dimension->getAnchors();
    if (anchors->size() == 1) {
        *lowIndex = *highIndex = 0;
        return;
    }

    double lowValue = anchors->front();
    double highValue = anchors->back();

    double gradient = (coordinate - lowValue) / (highValue - lowValue);
    int lastIndex = anchors->size() - 1;

    if (gradient <= 0) {
        *lowIndex = *highIndex = 0; // clamp to beginning of dimension
        return;
    } else if (gradient >= 1) {
        *lowIndex = *highIndex = lastIndex; // clamp to end of dimension
        return;
    }

    int startIndex = (int) (gradient * lastIndex);
    startIndex = std::max(std::min(startIndex, lastIndex), 0);

    int index;
    int searchDirection = 1;
    int searchWidth = std::max(startIndex, lastIndex - startIndex);
    for (int searched = 0; searched <= searchWidth; searched++) {
        index = startIndex + (searchDirection * searched);
        
        if (searchDirection == -1) {
            *lowIndex = std::max(index - 1, 0);
            *highIndex = index;
        } else {
            *lowIndex = index;
            *highIndex = std::min(index + 1, lastIndex);
        }

        double anchorLow = anchors->at(*lowIndex);
        double anchorHigh = anchors->at(*highIndex);

        if (anchorLow <= coordinate && anchorHigh >= coordinate) {
            break;
        } else if (anchorLow > coordinate) {
            searchDirection = -1;
        } else {
            searchDirection = 1;
        }
    }
}

v_int findDataIndex(Table* table, v_double const &coordinates) {
    int lowIndex = -1, highIndex = -1;
    double coordinate;
    Dimension* dimension;
    v_int indices;

    int numDimensions = table->numDimensions();
    indices.resize(numDimensions);
    for (int dimIndex = 0; dimIndex < numDimensions; dimIndex ++) {
        dimension = table->getDimension(dimIndex);
        coordinate = coordinates.at(dimIndex);
        findDimensionIndices_estimate(dimension, coordinate, &lowIndex, &highIndex);
        assignIndices(dimension, dimIndex, &indices, lowIndex, highIndex);
    }
    return indices;
}

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

int Table::getDataOffset(v_int const &coordinates) {
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

    return index;
}

double Table::getData(int index) {
    return this->data->at(index);
}

double Table::getData(v_int const &coordinates) {
    int index = this->getDataOffset(coordinates);
    return this->getData(index);
}

double Table::setData(int index, double value) {
    double old = this->data->at(index);
    this->data->at(index) = value;
    return old;
}

double Table::setData(v_int const &coordinates, double value) {
    int index = this->getDataOffset(coordinates);
    return this->setData(index, value);
}

v_int Table::getDataIndex(v_double const &coordinates) {
    return findDataIndex(this, coordinates);
}

double Table::setData(v_double const &coordinates, double value) {
    return this->setData(getDataIndex(coordinates), value);
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
            pairs[i / 2] = a;
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
        findDimensionIndices_estimate(dimension, coordinate, &lowIndex, &highIndex);

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