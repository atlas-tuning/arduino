#include "Arduino.h"
#include "Profiler.h"

#include "EEPROMStorage.h"
#include "SPIFFSStorage.h"
 
#define PROGRAM_FILE "/program.bin"
#define DEBUG_PAUSE 500
#define DEBUG 1

Program* program;
 
void setup() {
    PROFILE_START("setup");
    Serial.begin(115200);
    delay(1000);

    Serial.write("Initializing storage...\n");
    PermanentStorage* storage = new SPIFFSStorage(PROGRAM_FILE);
    storage->initialize();

    Serial.write("Reading program from storage...\n");
    program = storage->readProgram();
    if (!program) {
      Serial.write("Program read failed!\n");
      abort();
    }

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
    PROFILE_STOP();
 }

 void loop() {
    PROFILE_START("loop");
    #if defined(DEBUG)
    Serial.write("[=======FRAME=======]\n");
    #endif

    // Update all inputs
    PROFILE_START("inputs");
    for (auto& i : *program->getInputs()) {
      i->read();

      #if defined(DEBUG)
      Serial.write("SENS  ");
      Serial.write(i->getName()->c_str());
      Serial.write(": ");
      Value* value = i->getPrimaryValue();
      if (value) {
        float i_val = value->get();
        Serial.write(std::to_string(i_val).c_str());
      } else {
        Serial.write("nullptr");
      }
      Serial.write("\n");
      #endif
    }
    PROFILE_STOP();

    PROFILE_START("busses");
    // Update all busses and their corresponding inputs
    for (auto& b : *program->getBusses()) {
        for (auto& i : *b->getInputs()) {
            i->read();
        }

        b->update();
    }
    PROFILE_STOP();

    #if defined(DEBUG)
    PROFILE_START("tables");
    for (auto& t : *program->getTables()) {
      float t_val = t->get();

      Serial.write("TABLE ");
      Serial.write(t->getName()->c_str());
      Serial.write(": ");
      Serial.write(std::to_string(t_val).c_str());
      Serial.write("\n");
    }
    PROFILE_STOP();
    #endif

    // Update all outputs (also calls tables)
    PROFILE_START("outputs");
    for (auto& o : *program->getOutputs()){
      float sent = o->send();

      #if defined(DEBUG)
      Serial.write("OUT   ");
      Serial.write(o->getName()->c_str());
      Serial.write(": ");
      Serial.write(std::to_string(sent).c_str());
      Serial.write("\n");
      #endif
    }
    PROFILE_STOP();

    #if defined(DEBUG)
    delay(DEBUG_PAUSE);
    #endif
    PROFILE_STOP();
 }