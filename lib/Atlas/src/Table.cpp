#include <algorithm>
#include <math.h>
#include <cstdint>

#include "Table.h"
#include "Profiler.h"

/**
 * Assigns a value to indices that represents the index of the Table for a given low and high index pair
 * @param dimension the dimension to assign indices from
 * @param dimensionIndex the index of the dimension provided in the Table
 * @param indices the array to assign indices to
 * @param lowIndex the "low" index within the dimension that represents the lower bound to assign against
 * @param highIndex the "high" index within the dimension that represents the upper bound to assign against
*/
void assignIndices(Dimension* dimension, int dimensionIndex, v_int* indices, int lowIndex, int highIndex) {
    v_float* anchors = dimension->getAnchors();
    
    // Clamp indices to the bounds of the dimension provided
    lowIndex = std::max(0, std::min((int)anchors->size() - 1, lowIndex));
    highIndex = std::max(0, std::min((int)anchors->size() - 1, highIndex));

    if (lowIndex == highIndex) {
        // If the low and high indices are the same, then the answer is easy; pick either one.
        (*indices)[dimensionIndex] = highIndex;
    } else {
        float lowValue = anchors->at(lowIndex);
        float highValue = anchors->at(highIndex);

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
void findDimensionIndices_linear(Dimension* dimension, float coordinate,
                                int* lowIndex, int* highIndex) {
    v_float* anchors = dimension->getAnchors();
    for (int index = 0; index < anchors->size(); index++) {
        float anchor = anchors->at(index);

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
void findDimensionIndices_estimate(Dimension* dimension, float coordinate,
                                    int* lowIndex, int* highIndex) {
    v_float* anchors = dimension->getAnchors();
    if (anchors->size() == 1) {
        *lowIndex = *highIndex = 0;
        return;
    }

    float lowValue = anchors->front();
    float highValue = anchors->back();

    float gradient = (coordinate - lowValue) / (highValue - lowValue);
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

        float anchorLow = anchors->at(*lowIndex);
        float anchorHigh = anchors->at(*highIndex);

        if (anchorLow <= coordinate && anchorHigh >= coordinate) {
            break;
        } else if (anchorLow > coordinate) {
            searchDirection = -1;
        } else {
            searchDirection = 1;
        }
    }
}

/*
 * Continually divides a dimension in two by first estimating an index to divide at using the
 * provided coordinate to calculate a gradient between the active low and high anchors. This
 * repeats until the low and high anchor indices are either equal or are adjacent. This sets
 * this algorithm apart from the "estimate" algorithm as the estimation is continually repeated
 * until convergence on the desired dimension anchors, whereas the aforementioned algorithm
 * only performs estimation once and performs a linear scan from that point. This algorithm
 * should have better performance on larger tables with non-linear anchors, but has
 * shown to meet or exceed the performance of the linear algorithm in testing on linear tables.
 * 
 * Additionally, this function will remember the last low anchor index and reuse it for faster
 * future calculations. This is particularly useful for automotive applications where inputs
 * change gradually between table processing.
 * 
 * @param dimension the dimension to search
 * @param coordinate the coordinate to search for
 * @param lowIndex the low index to set for the given coordinate, when found in the dimension
 * @param highIndex the high index to set for the given coordinate, when found in the dimension.
*/
void findDimensionIndices_reestimate(Dimension* dimension, float coordinate,
                                    int* lowIndex, int* highIndex) {
    v_float* anchors = dimension->getAnchors();
    if (anchors->size() == 1) {
        *lowIndex = *highIndex = 0;
        return;
    }

    int lastIndex = anchors->size() - 1;
    int lowSearchIndex = 0;
    int highSearchIndex = lastIndex;
    float lowValue = anchors->at(lowSearchIndex);
    float highValue = anchors->at(highSearchIndex);

    if (lowValue >= coordinate) {
        *lowIndex = *highIndex = 0; // clamp to beginning of dimension
        return;
    } else if (highValue <= coordinate) {
        *lowIndex = *highIndex = lastIndex; // clamp to end of dimension
        return;
    }

    float coordinate_minus_low_value = coordinate - lowValue;
    int medianIndex = dimension->getLastLookupIndex();
    while (true) {
        float medianValue = anchors->at(medianIndex);
        if (medianValue == coordinate) {
            // Easily found by direct match
            *lowIndex = *highIndex = medianIndex;
            return;
        } else if (coordinate > medianValue) {
            lowSearchIndex = medianIndex;
            lowValue = anchors->at(lowSearchIndex);
            coordinate_minus_low_value = coordinate - lowValue;
        } else {
            highSearchIndex = medianIndex;
            highValue = anchors->at(highSearchIndex);
        }

        // Check to see if the search indices are adjacent or equal
        if (highSearchIndex - lowSearchIndex <= 1) {
            *lowIndex = lowSearchIndex;
            *highIndex = highSearchIndex;
            dimension->setLastLookupIndex(lowSearchIndex);
            return;
        }

        float gradient = coordinate_minus_low_value / (highValue - lowValue);
        medianIndex = lowSearchIndex + (gradient * (highSearchIndex - lowSearchIndex));
        if (medianIndex <= lowSearchIndex) {
            medianIndex = lowSearchIndex + 1;
        }
    }

    throw "Failed to find value";
}

v_int findDataIndex(Table* table, v_float const &coordinates) {
    int lowIndex = -1, highIndex = -1;
    float coordinate;
    Dimension* dimension;
    v_int indices;

    int numDimensions = table->numDimensions();
    indices.resize(numDimensions);
    for (int dimIndex = 0; dimIndex < numDimensions; dimIndex ++) {
        dimension = table->getDimension(dimIndex);
        coordinate = coordinates.at(dimIndex);
        findDimensionIndices_reestimate(dimension, coordinate, &lowIndex, &highIndex);
        assignIndices(dimension, dimIndex, &indices, lowIndex, highIndex);
    }
    return indices;
}

Table::Table(std::string* name, v_dimension *dimensions, std::vector<float> *data): Value(name) {
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
    int precedingDimensionLength = 1;

    for (int i = 0; i < dimensions; i ++) {
        if (i == 0) {
            index += coordinates[i];
        } else {
            index += precedingDimensionLength * coordinates[i];
        }

        precedingDimensionLength *= this->getDimension(i)->getSize();
    }
    return index;
}

float Table::getData(int index) {
    return this->data->at(index);
}

float Table::getData(v_int const &coordinates) {
    int index = this->getDataOffset(coordinates);
    return this->getData(index);
}

float Table::setData(int index, float value) {
    float old = this->data->at(index);
    this->data->at(index) = value;
    return old;
}

float Table::setData(v_int const &coordinates, float value) {
    int index = this->getDataOffset(coordinates);
    return this->setData(index, value);
}

v_int Table::getDataIndex(v_float const &coordinates) {
    return findDataIndex(this, coordinates);
}

float Table::setData(v_float const &coordinates, float value) {
    return this->setData(getDataIndex(coordinates), value);
}

int calculateCornerIndex(v_int *cornerIndices) {
    int index = 0;
    for (int i = 0; i < cornerIndices->size(); i ++) {
        index = index | (int8_t)((cornerIndices->at(i) & 0x1) << i);
    }
    return index;
}

void Table::getDataIndex(v_int const &cornerIndices, v_int const &lowIndices, v_int const &highIndices, v_int  &dataIndex) {
    for (int d = 0; d < numDimensions(); d ++) {
        int cornerIndex = cornerIndices[d];
        dataIndex.push_back(cornerIndex == 0 ? lowIndices[d] : highIndices[d]);
    }
}

v_float Table::fill(v_int const &lowIndices, v_int const &highIndices) {
    int cornerCount = pow(2, numDimensions());
    v_float corners;
    corners.resize(cornerCount);
    fill(corners, lowIndices, highIndices);
    return corners;
}

void Table::fill(v_float& corners, v_int const &lowIndices, v_int const &highIndices) {
    v_int cornerIndices;
    cornerIndices.resize(numDimensions());
    int cornerIndex;
    fill(0, cornerIndex, cornerIndices, corners, lowIndices, highIndices);
}

void Table::fill(int dimIndex, int cornerIndex, v_int &cornerIndices, v_float& corners,
                    v_int const &lowIndices, v_int const &highIndices) {
    int dimensions = numDimensions();
    for (int i = 0; i < 2; i++) {
        if (dimIndex == 0) {
            PROFILE_START("fill_reset");
            std::fill(cornerIndices.begin(), cornerIndices.end(), 0);
            cornerIndex = 0;
            PROFILE_STOP();
        }
        cornerIndices[dimIndex] = i;
        cornerIndex = cornerIndex | (i << dimIndex);
        if (dimIndex == dimensions - 1) {
            v_int dataIndex = v_int();
            PROFILE_START("getDataIndex");
            getDataIndex(cornerIndices, lowIndices, highIndices, dataIndex);
            PROFILE_STOP();
            PROFILE_START("getData");
            corners.at(cornerIndex) = getData(dataIndex);
            PROFILE_STOP();
        } else {
            fill(dimIndex + 1, cornerIndex, cornerIndices, corners, lowIndices, highIndices);
        }
    }
}

v_float Table::reduce(v_int const &lowIndices, v_int const &highIndices, v_float const &gradients) {

    PROFILE_START("table.reduce");
    
    PROFILE_START("fill");
    v_float corners = fill(lowIndices, highIndices);
    PROFILE_STOP();

    PROFILE_START("reduce");
    v_float reduced = reduce(corners, gradients, 0);
    PROFILE_STOP();

    PROFILE_STOP();
    return reduced;
}

v_float Table::reduce(v_float& corners, v_float const &gradients, int dimIndex) {
    Dimension* dimension = getDimension(dimIndex);
    Integration integration = *dimension->getIntegration();
    float g = gradients[dimIndex];

    v_float pairs;
    pairs.resize(corners.size() / 2);

    for (int i = 0; i < corners.size(); i += 2) {
        float a = corners[i];
        float b = corners[i+1];

        if (a == b) {
            pairs[i / 2] = a;
        } else {
            pairs[i / 2] = integration(a, b, g);
        }
    }

    v_float result;
    if (pairs.size() == 1) {
        result = pairs;
    } else {
        result = reduce(pairs, gradients, dimIndex + 1);
    }

    return result;
}

float Table::integrate(v_float const &coordinates) {
    PROFILE_START("table.integrate");
    int numDimensions = this->numDimensions();
    v_int lowIndices, highIndices;
    v_float gradients;
    lowIndices.resize(numDimensions);
    highIndices.resize(numDimensions);
    gradients.resize(numDimensions);

    // Identify the low and high portion of the cells
    for (int dimIndex = 0; dimIndex < numDimensions; dimIndex ++) {
        Dimension* dimension = getDimension(dimIndex);
        v_float* anchors = dimension->getAnchors();
        float coordinate = coordinates.at(dimIndex);

        int lowIndex = -1, highIndex = -1;
        PROFILE_START("findDimensionIndices");
        findDimensionIndices_reestimate(dimension, coordinate, &lowIndex, &highIndex);
        PROFILE_STOP();

        // Clamp to the bounds of the table
        lowIndex = std::max(0, std::min((int)anchors->size() - 1, lowIndex));
        highIndex = std::max(0, std::min((int)anchors->size() - 1, highIndex));

        if (lowIndex == highIndex) {
            gradients[dimIndex] = 1.0f;
            lowIndices[dimIndex] = lowIndex;
            highIndices[dimIndex] = highIndex;
        } else {
            float lowValue = anchors->at(lowIndex);
            float highValue = anchors->at(highIndex);
            lowIndices[dimIndex] = lowIndex;
            highIndices[dimIndex] = highIndex;

            if (lowValue == highValue) {
                gradients[dimIndex] = 1.0f;
            } else {
                gradients[dimIndex] = (coordinate - lowValue) / (highValue - lowValue);
            }
        }
    }

    v_float reduced = reduce(lowIndices, highIndices, gradients);
    PROFILE_STOP();
    return reduced[0];
}

float Table::get() {
    PROFILE_START("get");
    v_float coordinates;
    int numDimensions = this->numDimensions();
    coordinates.resize(numDimensions);
    for (int dim = 0; dim < numDimensions; dim ++) 
    {
        auto dimension = this->getDimension(dim);
        coordinates.at(dim) = dimension->getSource()->get();
    }

    float value = integrate(coordinates);
    PROFILE_STOP();

    return value;
}