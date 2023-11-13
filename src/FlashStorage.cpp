#pragma once

#include "Arduino.h"
#include <EEPROM.h>

#include "FlashStorage.h"
#include "Table.h"

#include "GPIOInput.h"
#include "GPIOOutput.h"

FlashStorage::FlashStorage(int size) {
    this->size = size;
}

void FlashStorage::initialize() {
    EEPROM.begin(this->size);
}

v_input* readInputs(int* offs) {
    v_input* inputs = new v_input();

    uint8_t num_inputs = EEPROM.readByte(*offs); *offs ++;;
    for (int i = 0; i < num_inputs; i++) {
        String name_value = EEPROM.readString(*offs);
        *offs += name_value.length() + 1;

        std::string* name = new std::string(name_value.c_str(), name_value.length());

        int pin = EEPROM.readByte(*offs); *offs ++;
        int mode = EEPROM.readByte(*offs); *offs ++;
        int type = EEPROM.readByte(*offs); *offs ++;
        GPIOInput* input = new GPIOInput(name, pin, mode, type);
        inputs->push_back(input);
    }

    return inputs;
}

v_output* readOutputs(int* offs, v_table* tables) {
    v_output* outputs = new v_output();

    uint8_t num_inputs = EEPROM.readByte(*offs); *offs ++;
    for (int i = 0; i < num_inputs; i++) {
        String name_value = EEPROM.readString(*offs);
        *offs += name_value.length() + 1;
        std::string* name = new std::string(name_value.c_str(), name_value.length());

        int tableIndex = EEPROM.readShort(*offs); *offs += 2;
        Table* table = tables->at(tableIndex);
        if (tableIndex < 0xFFFF) {
            table = tables->at(tableIndex);
        } else {
            table = nullptr;
        }

        int holdTableIndex = EEPROM.readShort(*offs); *offs += 2;
        Table* holdTime;
        if (holdTableIndex < 0xFFFF) {
            holdTime = tables->at(tableIndex);
        } else {
            holdTime = nullptr;
        }

        int pin = EEPROM.readByte(*offs); *offs ++;
        int mode = EEPROM.readByte(*offs); *offs ++;
        int type = EEPROM.readByte(*offs); *offs ++;

        GPIOOutput* output = new GPIOOutput(name,
                                            table,
                                            holdTime, 
                                            pin, mode, type);
        outputs->push_back(output);
    }

    return outputs;
}

v_double* readData(int sz) {

}

v_table* readTables(int* offs, v_input* inputs) {
    v_table* tables = new v_table();

    uint16_t num_tables = EEPROM.readShort(*offs); *offs += 2;
    for (int i = 0; i < num_tables; i++) {
        String name_value = EEPROM.readString(*offs);
        *offs += name_value.length() + 1;
        std::string* name = new std::string(name_value.c_str(), name_value.length());
        
        v_dimension* dimensions = new v_dimension();
        uint16_t num_dimensions = EEPROM.read(*offs); *offs ++;
        for (int j = 0; j < num_dimensions; j++) {
            uint16_t sourceIndex = EEPROM.readShort(*offs); *offs += 2;
            Value* source;
            if (sourceIndex |= 0x8000) {
                uint16_t tableIndex = sourceIndex ^ 0x8000;
                source = tables->at(tableIndex);
            } else {
                source = inputs->at(sourceIndex)->getPrimaryValue();
            }
            
            uint8_t flags = EEPROM.read(*offs); *offs ++;
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

            v_double* data = readData(num_cols);
            Dimension* dimension = new Dimension(source, integration, data);
            dimensions->push_back(dimension);
        }

        uint32_t num_data = EEPROM.readInt(*offs); *offs += 4;
        v_double* data = readData(num_data);

        Table* table = new Table(name, dimensions, data);
        tables->push_back(table);
    }

    return tables;
}

Program* FlashStorage::readProgram() {
    int offs = 0x00000000;

    v_input* inputs = readInputs(&offs);
    v_table* tables = readTables(&offs, inputs);
    v_output* outputs = readOutputs(&offs, tables);
    
    Program* program = new Program(inputs, outputs, tables);
}