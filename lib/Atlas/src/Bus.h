#pragma once

#include <string>
#include <vector>

class Bus {
public:
    Bus(std::string* name);

    std::string* getName();

    virtual int setup() = 0;
    virtual int begin() = 0;
    virtual int end() = 0;

private:
    std::string *name;
};

typedef std::vector<Bus*> v_bus;