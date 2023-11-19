#pragma once

#include <string>
#include <vector>

#include "Input.h"

class Bus {
public:
    Bus(std::string* name);

    std::string* getName();

    /**
     * Performs one-time setup of the bus
    */
    virtual int setup() = 0;
    
    /**
     * Begins bus communication
    */
    virtual int begin() = 0;

    /**
     * @returns 0 if no messages were handled, positive for number of messages 
     *            handled, and negative on error.
    */
    virtual int update() = 0;

    /**
     * Ends bus communication
    */
    virtual int end() = 0;

    /**
     * Gets all declared inputs by this bus
    */
    v_input* getInputs();

private:
    std::string *name;
    v_input *inputs;
};

typedef std::vector<Bus*> v_bus;