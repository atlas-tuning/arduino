#pragma once

#include <string>

#include "Value.h"

class Output {
    public:
        Output(std::string* name);
        Output();

        virtual int setup() = 0;

        virtual double send() = 0;

        double getLastSent();

        std::string* getName();
    protected:
      double sent;
    private:
      std::string* name;
};

typedef std::vector<Output*> v_output;