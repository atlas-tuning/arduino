#pragma once

#include <string>
#include <vector>
#include <memory>

class Value {
   public:

      Value(std::string* name);
      Value();
   
      virtual bool isStatic() = 0;
      virtual float get() = 0;

      std::string* getName();
      
   private:
      std::string* name;
};

typedef std::vector<std::unique_ptr<Value>> v_value;