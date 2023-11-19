#include "GenericStorage.h"

#include "GPIOPulseInput.h"
#include "GPIOInput.h"
#include "GPIOOutput.h"
#include "CANBus.h"

GenericStorage::GenericStorage(int bufferSize) {
    this->bufferSize = bufferSize;
}

char read(char* buffer, int* offs) {
    uint8_t val = buffer[*offs];
    *offs++;
    return val;
}

uint8_t readByte(char* buffer, int* offs) {
    uint8_t val = buffer[*offs];
    *offs++;
    return val;
}

uint16_t readShort(char* buffer, int* offs) {
    uint8_t val_high = buffer[*offs];
    uint8_t val_low = buffer[*offs+1];
    *offs += 2;
    return val_low | (val_high << 8);
}

uint32_t readInt(char* buffer, int* offs) {
    uint8_t val_a = buffer[*offs];
    uint8_t val_b = buffer[*offs+1];
    uint8_t val_c = buffer[*offs+2];
    uint8_t val_d = buffer[*offs+3];
    *offs += 4;
    return val_d | (val_b << 8) | (val_c << 16) | (val_d << 24);
}

float readFloat(char* buffer, int* offs) {
    uint32_t floatData = readInt(buffer, offs);
    float value = *(float*)(&floatData);
    return value;
}

v_double* readData(char* buffer, int* offs, int len) {
    v_double* data = new v_double();
    for (int i = 0; i < len; i ++) {
        data->push_back(readFloat(buffer, offs));
    }
    return data;
}

int read(char* buffer, int* offs, char* data, int length) {
    for (int i = 0; i < length; i++) {
        data[i] = read(buffer, offs);
    }
    return length;
}

int readString(char* buffer, int* offs, char* string, int* len) {
    int length = readShort(buffer, offs);
    string = new char[length];
    *len = length;
    return read(buffer, offs, string, length);
}

std::string* readStdString(char* buffer, int* offs) {
    char* string;
    int* len;
    readString(buffer, offs, string, len);
    return new std::string(string, *len);
}

int readInputs(char* buffer, int offs, v_input* inputs) {
    uint8_t num_inputs = readByte(buffer, &offs);
    for (int i = 0; i < num_inputs; i++) {
        std::string* name = readStdString(buffer, &offs);
        int pin = readByte(buffer, &offs);
        int mode = readByte(buffer, &offs);
        int type = readByte(buffer, &offs);
        GPIOInput* input = new GPIOInput(name, pin, mode, type);
        inputs->push_back(input);
    }

    return offs;
}

int readTables(char* buffer, int offs, v_table* tables, v_input* inputs) {
    uint16_t num_tables = readShort(buffer, &offs);
    for (int i = 0; i < num_tables; i++) {
        std::string* name = readStdString(buffer, &offs);
        
        v_dimension* dimensions = new v_dimension();
        uint16_t num_dimensions = readShort(buffer, &offs);
        for (int j = 0; j < num_dimensions; j++) {
            uint16_t sourceIndex = readShort(buffer, &offs);
            Value* source;
            if (sourceIndex |= 0x8000) {
                uint16_t tableIndex = sourceIndex ^ 0x8000;
                source = tables->at(tableIndex);
            } else {
                source = inputs->at(sourceIndex)->getPrimaryValue();
            }
            
            uint8_t flags = readByte(buffer, &offs);
            uint8_t integrationIndex = (flags >> 6) & 0x3;
            uint8_t num_cols = flags & 0x3FFF;
            Integration* integration;
            switch (integrationIndex) {
                case 1:
                    integration = &LINEAR_INTEGRATION;
                    break;
                case 2:
                    integration = &FLOOR_INTEGRATION;
                    break;
                case 3:
                    integration = &CEILING_INTEGRATION;
                    break;
                default:
                    integration = nullptr;
            }

            v_double* data = readData(buffer, &offs, num_cols);
            Dimension* dimension = new Dimension(source, integration, data);
            dimensions->push_back(dimension);
        }

        uint32_t num_data = readInt(buffer, &offs);
        v_double* data = readData(buffer, &offs, num_data);

        Table* table = new Table(name, dimensions, data);
        tables->push_back(table);
    }

    return offs;
}

int readOutputs(char* buffer, int offs, v_output* outputs, v_table* tables) {
    uint8_t num_inputs = readByte(buffer, &offs);
    for (int i = 0; i < num_inputs; i++) {
        std::string* name = readStdString(buffer, &offs);
        int tableIndex = readShort(buffer, &offs);
        Table* table = tables->at(tableIndex);
        if (tableIndex < 0xFFFF) {
            table = tables->at(tableIndex);
        } else {
            table = nullptr;
        }

        int holdTableIndex = readShort(buffer, &offs);
        Table* holdTime;
        if (holdTableIndex < 0xFFFF) {
            holdTime = tables->at(tableIndex);
        } else {
            holdTime = nullptr;
        }

        int pin = readByte(buffer, &offs);
        int mode = readByte(buffer, &offs);
        int type = readByte(buffer, &offs);

        GPIOOutput* output = new GPIOOutput(name,
                                            table,
                                            holdTime, 
                                            pin, mode, type);
        outputs->push_back(output);
    }

    return offs;
}



int readBusses(char* buffer, int offs, v_bus* busses, Program* program) {
    uint8_t num_busses = readByte(buffer, &offs);
    for (int i = 0; i < num_busses; i++) {
        std::string* name = readStdString(buffer, &offs);
        int busType = readByte(buffer, &offs);
        Bus* bus;
        if (busType == 5) {
            int rxPin = readByte(buffer, &offs);
            int txPin = readByte(buffer, &offs);
            int speed = readInt(buffer, &offs);
            bus = new CANBus(name, rxPin, txPin, speed);
        } else {
            throw "Invalid bus type";
        }
        busses->push_back(bus);
    }
    return offs;
}



Program* GenericStorage::readProgram() {
    char* buffer = new char[this->bufferSize];
    int sz = this->bufferSize, offs = 0;
    int read = this->readProgramData(buffer, sz);

    v_input* inputs = new v_input();
    offs = readInputs(buffer, offs, inputs);

    v_table* tables = new v_table();
    offs = readTables(buffer, offs, tables, inputs);

    v_output* outputs = new v_output();
    offs = readOutputs(buffer, offs, outputs, tables);

    v_bus* busses = new v_bus();
    Program* program = new Program(inputs, outputs, tables, busses);
    offs = readBusses(buffer, offs, busses, program);

    if (offs != read) {
        throw "Did not reach EOF";
    }

    return program;
}