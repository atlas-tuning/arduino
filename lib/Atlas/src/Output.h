#pragma once

#include <string>

#include "Value.h"

class Output {
    public:
        Output(std::string* name);
        Output();

        virtual int setup() = 0;

        virtual float send() = 0;

        float getLastSent();

        std::string* getName();
    protected:
      float sent;
    private:
      std::string* name;
};

typedef std::vector<Output*> v_output;