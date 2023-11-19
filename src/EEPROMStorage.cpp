#pragma once

#include "Arduino.h"
#include <EEPROM.h>

#include "EEPROMStorage.h"
#include "Table.h"

#include "GPIOInput.h"
#include "GPIOOutput.h"

EEPROMStorage::EEPROMStorage(int address, int size): GenericStorage(size) {
    this->address = address;;
    this->size = size;
}

void EEPROMStorage::initialize() {
    EEPROM.begin(this->size);
}

int EEPROMStorage::readProgramData(char* buffer, int sz) {
    EEPROM.readBytes(this->address, buffer, sz);
}