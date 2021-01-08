#ifndef __ENVIORMENT_H__
#define __ENVIORMENT_H__

#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include "object.h"
#include "token.h"
#include "error.h"

namespace lasm {
    class Enviorment {
        public:
            void define(std::string name, LasmObject &value);

            std::shared_ptr<LasmObject> get(std::shared_ptr<Token> name);
        private:
            std::map<std::string, std::shared_ptr<LasmObject>> values;
    };
}

#endif 
