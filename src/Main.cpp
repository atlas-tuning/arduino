#include "Arduino.h"

#include "FlashStorage.h"
 
#define FLASH_MEMORY_SIZE 0x2C00 

Program* program;
 
void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.write("Reading flash memory...\n");
    FlashStorage* storage = new FlashStorage(FLASH_MEMORY_SIZE);
    storage->initialize();

    program = storage->readProgram();
 }

 void loop() {
    #if defined(DEBUG)
    Serial.write("[=======FRAME=======]\n");
    #endif

    // Update all inputs
    for (auto& i : *program->getInputs()) {
      i->read();

      #if defined(DEBUG)
      Serial.write("SENS  ");
      double i_val = i->getPrimaryValue()->get();
      Serial.write(i->getName()->c_str());
      Serial.write(": ");
      Serial.write(std::to_string(i_val).c_str());
      Serial.write("\n");
      #endif
    }


    #if defined(DEBUG)
    for (auto& t : *program->getTables()) {
      double t_val = t->get();

      Serial.write("TABLE ");
      Serial.write(t->getName()->c_str());
      Serial.write(": ");
      Serial.write(std::to_string(t_val).c_str());
      Serial.write("\n");
    }
    #endif

    // Update all outputs (also calls tables)
    for (auto& o : *program->getOutputs()){
      double sent = o->send();

      #if defined(DEBUG)
      Serial.write("OUT   ");
      Serial.write(o->getName()->c_str());
      Serial.write(": ");
      Serial.write(std::to_string(sent).c_str());
      Serial.write("\n");
      #endif
    }

    #if defined(DEBUG)
    delay(500);
    #endif
 }