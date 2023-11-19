#include "Arduino.h"

#include "EEPROMStorage.h"
#include "SPIFFSStorage.h"
 
#define PROGRAM_FILE "/program.bin"
#define BUFFER_SIZE 0xF4240 

Program* program;
 
void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.write("Reading program from storage...\n");
    PermanentStorage* storage = new SPIFFSStorage(PROGRAM_FILE, BUFFER_SIZE);
    storage->initialize();

    program = storage->readProgram();

    Serial.write("Setting up inputs...\n");
    for (auto& i : *program->getInputs()) {
        i->setup();
    }

    Serial.write("Setting up outputs...\n");
    for (auto& o : *program->getOutputs()) {
        o->setup();
    }

    Serial.write("Setting up busses...\n");
    for (auto& b : *program->getBusses()) {
        b->setup();
        b->begin();
    }
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