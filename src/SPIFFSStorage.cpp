#include "SPIFFSStorage.h"

#ifdef ESP32
#include <SPIFFS.h>
#endif

SPIFFSStorage::SPIFFSStorage(const char* filename): GenericStorage() {
    this->filename = filename;
}

void SPIFFSStorage::initialize() {
    #ifdef ESP32
    SPIFFS.begin(true);
    #endif
}

int SPIFFSStorage::readProgramData(char** buffer) {
    #ifdef ESP32
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
    #else
    return -ENODEV;
    #endif
}