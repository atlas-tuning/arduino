#include "SPIFFSStorage.h"

#include <SPIFFS.h>

SPIFFSStorage::SPIFFSStorage(const char* filename): GenericStorage() {
    this->filename = filename;
}

void SPIFFSStorage::initialize() {
    SPIFFS.begin(true);
}

int SPIFFSStorage::readProgramData(char** buffer) {
    Serial.write("Opening file ");
    Serial.write(this->filename);
    Serial.write("...\n");
    fs::File file = SPIFFS.open(this->filename, "r", true);
    if (!file || file.available() <= 0) {
        return -ENODEV;
    }

    *buffer = new char[file.size()];

    Serial.write("Reading file data...\n");
    return (int) file.readBytes(*buffer, file.size());
}