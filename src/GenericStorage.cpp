#include <bitset>

#include "GenericStorage.h"

#include "GPIOPulseInput.h"
#include "GPIOInput.h"
#include "GPIOOutput.h"
#include "CANBus.h"

#include "MemoryStateTable.h"
#include "NVSStateTable.h"

#include "Variable.h"

#include "Profiler.h"

#define VERSION 2

typedef std::bitset<CHAR_BIT * sizeof(uint16_t)> ShortBits;

GenericStorage::GenericStorage() {
}

char read(char* buffer, int* offs) {
    #if defined(DEBUG)
    Serial.write("Reading char at offset ");
    Serial.write(std::to_string(*offs).c_str());
    Serial.write(": ");
    #endif

    uint8_t val = buffer[*offs];
    *offs += 1;

    #if defined(DEBUG)
    Serial.write(std::to_string((char) val).c_str());
    Serial.write(".\n");
    #endif

    return val;
}

uint8_t readByte(char* buffer, int* offs) {
    #if defined(DEBUG)
    Serial.write("Reading byte at offset ");
    Serial.write(std::to_string(*offs).c_str());
    Serial.write(": ");
    #endif

    uint8_t val = buffer[*offs];

    #if defined(DEBUG)
    Serial.write(std::to_string(val).c_str());
    Serial.write(".\n");
    #endif

    *offs += 1;
    return val;
}

uint16_t readShort(char* buffer, int* offs) {
    #if defined(DEBUG)
    Serial.write("Reading short at offset ");
    Serial.write(std::to_string(*offs).c_str());
    Serial.write(": ");
    #endif

    uint8_t val_high = buffer[*offs];
    uint8_t val_low = buffer[*offs+1];
    *offs += 2;
    uint16_t value = val_low | (val_high << 8);

    #if defined(DEBUG)
    Serial.write(std::to_string(value).c_str());
    Serial.write(".\n");
    #endif

    return value;
}

uint32_t readInt(char* buffer, int* offs) {
    #if defined(DEBUG)
    Serial.write("Reading int at offset ");
    Serial.write(std::to_string(*offs).c_str());
    Serial.write(": ");
    #endif

    uint8_t val_a = buffer[*offs];
    uint8_t val_b = buffer[*offs+1];
    uint8_t val_c = buffer[*offs+2];
    uint8_t val_d = buffer[*offs+3];
    *offs += 4;
    uint32_t value = val_d | (val_c << 8) | (val_b << 16) | (val_a << 24);

    #if defined(DEBUG)
    Serial.write(std::to_string(value).c_str());
    Serial.write(".\n");
    #endif

    return value;
}

float readFloat(char* buffer, int* offs) {
    #if defined(DEBUG)
    Serial.write("Reading float at offset ");
    Serial.write(std::to_string(*offs).c_str());
    Serial.write(": ");
    #endif

    float value = *(float*)(buffer + *offs);
    int ivalue = *(int*)(buffer + *offs);
    *offs += 4;

    #if defined(DEBUG)
    Serial.write(std::to_string(value).c_str());
    Serial.write(".\n");
    #endif

    return value;
}

v_float* readData(char* buffer, int* offs, int len) {
    #if defined(DEBUG)
    Serial.write("Reading floats of length ");
    Serial.write(std::to_string(len).c_str());
    Serial.write(" at offset ");
    Serial.write(std::to_string(*offs).c_str());
    Serial.write("...\n");
    #endif

    v_float* data = new v_float();
    for (int i = 0; i < len; i ++) {
        data->push_back(readFloat(buffer, offs));
    }
    return data;
}

int read(char* buffer, int* offs, char* data, int length) {
    #if defined(DEBUG)
    Serial.write("Reading binary data of length ");
    Serial.write(std::to_string(length).c_str());
    Serial.write(" at offset ");
    Serial.write(std::to_string(*offs).c_str());
    Serial.write("...\n");
    #endif

    for (int i = 0; i < length; i++) {
        data[i] = read(buffer, offs);
    }
    return length;
}

int readString(char* buffer, int* offs, char** string, int* len) {
    int length = readShort(buffer, offs);

    #if defined(DEBUG)
    Serial.write("Reading string data of length ");
    Serial.write(std::to_string(length).c_str());
    Serial.write(" at offset ");
    Serial.write(std::to_string(*offs).c_str());
    Serial.write("...\n");
    #endif

    *string = new char[length];
    *len = length;
    return read(buffer, offs, *string, length);
}

std::string* readStdString(char* buffer, int* offs) {
    char* string;
    int len;
    readString(buffer, offs, &string, &len);
    std::string* stdstring = new std::string();
    stdstring->assign(string, len);
    delete string;
    return stdstring;
}

int readInputs(char* buffer, int* offs, v_input* inputs) {
    uint8_t num_inputs = readByte(buffer, offs);
    for (int i = 0; i < num_inputs; i++) {
        std::string* name = readStdString(buffer, offs);
        int pin = readByte(buffer, offs);
        int mode = readByte(buffer, offs);
        int type = readByte(buffer, offs);
    
        Input* input;
        if (type == GPIOINPUT_TYPE_PULSE) {
            int edge = readByte(buffer, offs);
            int window = readShort(buffer, offs);

            #if defined(DEBUG)
            Serial.write("Adding pulse input ");
            Serial.write(name->c_str());
            Serial.write(", pin=");
            Serial.write(std::to_string(pin).c_str());
            Serial.write(", mode=");
            Serial.write(std::to_string(mode).c_str());
            Serial.write(", edge=");
            Serial.write(std::to_string(edge).c_str());
            Serial.write(", window=");
            Serial.write(std::to_string(window).c_str());
            Serial.write("...\n");
            #endif

            input = new GPIOPulseInput(name, pin, mode, edge, window);
        } else {
            uint8_t v_gnd, v_ref;
            v_gnd = readByte(buffer, offs);
            v_ref = readByte(buffer, offs);

            #if defined(DEBUG)
            Serial.write("Adding input ");
            Serial.write(name->c_str());
            Serial.write(", pin=");
            Serial.write(std::to_string(pin).c_str());
            Serial.write(", mode=");
            Serial.write(std::to_string(mode).c_str());
            Serial.write(", type=");
            Serial.write(std::to_string(type).c_str());
            Serial.write(", v_gnd=");
            Serial.write(std::to_string(v_gnd).c_str());
            Serial.write(", v_ref=");
            Serial.write(std::to_string(v_ref).c_str());
            Serial.write("...\n");
            #endif

            Value* v_gnd_input = nullptr;
            if (v_gnd < i) {
                v_gnd_input = inputs->at(v_gnd)->getPrimaryValue();
            } else if (v_gnd < 0xFF) {
                Serial.write("Referenced invalid input. There will be no ground reference on this input.\n");
            }

            Value* v_ref_input = nullptr;
            if (v_ref < i) {
                v_ref_input = inputs->at(v_ref)->getPrimaryValue();
            } else if (v_ref < 0xFF) {
                Serial.write("Referenced invalid input. There will be no voltage reference on this input.\n");
            }

            input = new GPIOInput(name, pin, mode, type, v_gnd_input, v_ref_input);
        }

        inputs->push_back(input);
    }

    return *offs;
}

int readDimensions(char* buffer, int* offs, v_table* tables, v_input* inputs, v_dimension* dimensions) {
    uint16_t num_dimensions = readShort(buffer, offs);
    for (int j = 0; j < num_dimensions; j++) {
        uint16_t sourceIndex = readShort(buffer, offs);
        Value* source;
        if (ShortBits(sourceIndex).test(15)) {
            uint16_t tableIndex = sourceIndex & 0x7FFF;
            if (tables->size() <= tableIndex) {
                Serial.write("Referenced unknown table with index=");
                Serial.write(std::to_string(tableIndex).c_str());
                Serial.write(".\n");
                return -1;
            }
            source = tables->at(tableIndex);
        } else {
            if (inputs->size() <= sourceIndex) {
                Serial.write("Referenced unknown input with index=");
                Serial.write(std::to_string(sourceIndex).c_str());
                Serial.write(".\n");
                return -1;
            }
            Input* input = inputs->at(sourceIndex);
            
            uint8_t subValueIndex = readByte(buffer, offs);
            if (input->getValues()->size() <= subValueIndex) {
                Serial.write("Referenced unknown subinput with index=");
                Serial.write(std::to_string(subValueIndex).c_str());
                Serial.write(".\n");
                return -1;
            }
            source = input->getValues()->at(subValueIndex).get();
        }

        uint8_t flags = readByte(buffer, offs);
        uint8_t integrationIndex = (flags >> 6) & 0x3;
        uint8_t num_cols = flags & 0x3F;
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

        v_float* data = readData(buffer, offs, num_cols);
        Dimension* dimension = new Dimension(source, integration, data);

        #if defined(DEBUG)
        Serial.write("Adding dimension ");
        Serial.write(std::to_string(j).c_str());
        Serial.write(", source=");
        Serial.write(source->getName()->c_str());
        Serial.write(", integration=");
        Serial.write(std::to_string(integrationIndex).c_str());
        Serial.write(", cols=");
        Serial.write(std::to_string(num_cols).c_str());
        Serial.write("...\n");
        #endif

        dimensions->push_back(dimension);
    }

    return *offs;
}

int readTables(char* buffer, int* offs, v_table* tables, v_input* inputs) {
    uint16_t num_tables = readShort(buffer, offs);
    for (int i = 0; i < num_tables; i++) {
        std::string* name = readStdString(buffer, offs);
        uint8_t type = readByte(buffer, offs);
        
        v_dimension* dimensions = new v_dimension();
        readDimensions(buffer, offs, tables, inputs, dimensions);

        uint32_t num_data = readInt(buffer, offs);
        v_float* data = readData(buffer, offs, num_data);

        #if defined(DEBUG)
        Serial.write("Adding table ");
        Serial.write(name->c_str());
        Serial.write(", type=");
        Serial.write(std::to_string(type).c_str());
        Serial.write(", data=");
        Serial.write(std::to_string(num_data).c_str());
        Serial.write("...\n");
        #endif

        Table* table;
        switch (type) {
            case 0x1:
                table = new Table(name, dimensions, data);
                break;
            case 0x2:
                table = new MemoryStateTable(name, dimensions, data);
                break;
            case 0x3:
                table = new NVSStateTable(name, dimensions, data);
                break;
            default:
                throw "Unknown table type";
        }
        tables->push_back(table);
    }

    return *offs;
}

int readOutputs(char* buffer, int* offs, v_output* outputs, v_table* tables) {
    uint8_t num_inputs = readByte(buffer, offs);
    for (int i = 0; i < num_inputs; i++) {
        std::string* name = readStdString(buffer, offs);

        int pin = readByte(buffer, offs);
        int mode = readByte(buffer, offs);
        int type = readByte(buffer, offs);
        int tableIndex = readShort(buffer, offs);
        int holdTableIndex = readShort(buffer, offs);

        int updateFrequencyTableIndex = readShort(buffer, offs);
        Value* updateFrequency = nullptr;
        if (updateFrequencyTableIndex < 0xFFFF) {
            updateFrequency = tables->at(updateFrequencyTableIndex);
        } else {
            float frequency_value = readFloat(buffer, offs);
            if (frequency_value > 0.0f) {
                updateFrequency = new Variable(frequency_value);
            }
        }

        Value* pwmFrequency = nullptr;
        int frequencyTableIndex = 0xFFFF;
        if (type == GPIOOUTPUT_TYPE_PWM) {
            frequencyTableIndex = readShort(buffer, offs);
            if (frequencyTableIndex < 0xFFFF) {
                pwmFrequency = tables->at(frequencyTableIndex);
            } else {
                uint32_t frequency_value = readInt(buffer, offs);
                pwmFrequency = new Variable((float) frequency_value);
            }
        }

        Table* table;
        if (tableIndex < 0xFFFF) {
            table = tables->at(tableIndex);
        } else {
            table = nullptr;
        }

        Value* holdTime;
        if (holdTableIndex < 0xFFFF) {
            holdTime = tables->at(holdTableIndex);
        } else {
            holdTime = nullptr;
        }

        #if defined(DEBUG)
        Serial.write("Adding output ");
        Serial.write(name->c_str());
        Serial.write(", pin=");
        Serial.write(std::to_string(pin).c_str());
        Serial.write(", mode=");
        Serial.write(std::to_string(mode).c_str());
        Serial.write(", type=");
        Serial.write(std::to_string(type).c_str());
        Serial.write(", table=");
        Serial.write(std::to_string(tableIndex).c_str());
        Serial.write(", holdTable=");
        Serial.write(std::to_string(holdTableIndex).c_str());
        Serial.write(", updateFreqTable=");
        Serial.write(std::to_string(updateFrequencyTableIndex).c_str());
        Serial.write(", pwmFreqTable=");
        Serial.write(std::to_string(frequencyTableIndex).c_str());
        Serial.write("...\n");
        #endif

        GPIOOutput* output = new GPIOOutput(name,
                                            table,
                                            holdTime,
                                            pwmFrequency,
                                            updateFrequency,
                                            pin, mode, type);
        outputs->push_back(output);
    }

    return *offs;
}

int readBusses(char* buffer, int* offs, v_bus* busses, Program* program) {
    uint8_t num_busses = readByte(buffer, offs);
    for (int i = 0; i < num_busses; i++) {
        std::string* name = readStdString(buffer, offs);
        int busType = readByte(buffer, offs);
        Bus* bus;
        if (busType == 5) {
            int rxPin = readByte(buffer, offs);
            int txPin = readByte(buffer, offs);
            int speed = readInt(buffer, offs);
            bus = new CANBus(name, rxPin, txPin, speed);
        } else {
            throw "Invalid bus type";
        }
        busses->push_back(bus);
    }
    return *offs;
}



Program* GenericStorage::readProgram() {
    PROFILE_START("readProgram");
    Serial.write("Reading program data...\n");
    char* buffer;
    int read = this->readProgramData(&buffer);

    if (read <= 0) {
        Serial.write("Error reading program data: ");
        Serial.write(std::to_string(read).c_str());
        Serial.write("\n");
        PROFILE_STOP();
        return nullptr;
    }

    if (!buffer) {
        Serial.write("Buffer wasn't instantiated!\n");
        PROFILE_STOP();
        return nullptr;
    }

    Serial.write("Read ");
    Serial.write(std::to_string(read).c_str());
    Serial.write(" program bytes.\n");

    int offs = 0x00;
    short version = readShort(buffer, &offs);
    if (version != VERSION) {
        Serial.write("Unsupported program version ");
        Serial.write(std::to_string(version).c_str());
        Serial.write(".\n");
        PROFILE_STOP();
        return nullptr;
    }

    Serial.write("Reading inputs...\n");
    v_input* inputs = new v_input();
    readInputs(buffer, &offs, inputs);

    Serial.write("Reading tables...\n");
    v_table* tables = new v_table();
    readTables(buffer, &offs, tables, inputs);

    Serial.write("Reading outputs...\n");
    v_output* outputs = new v_output();
    readOutputs(buffer, &offs, outputs, tables);

    Serial.write("Reading busses...\n");
    v_bus* busses = new v_bus();
    Program* program = new Program(inputs, outputs, tables, busses);
    readBusses(buffer, &offs, busses, program);

    PROFILE_STOP();

    if (offs != read) {
        Serial.write("Did not reach end of file!\n");
        return nullptr;
    }

    Serial.write("Program read successfully.\n");

    return program;
}