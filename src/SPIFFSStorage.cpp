#include "SPIFFSStorage.h"

#include <SPIFFS.h>

SPIFFSStorage::SPIFFSStorage(const char* filename, int bufferSize): GenericStorage(bufferSize) {
    this->filename = filename;
}

void SPIFFSStorage::initialize() {
    SPIFFS.begin();
}

int SPIFFSStorage::readProgramData(char* buffer, int sz) {
    fs::File file = SPIFFS.open(this->filename, "r", false);
    if (sz < file.size()) {
        throw "Buffer too small to read file";
    }
    return (int) file.readBytes(buffer, min(sz, (int) file.size()));
}