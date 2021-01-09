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
            Enviorment(std::shared_ptr<Enviorment> parent=std::shared_ptr<Enviorment>(nullptr)):
                parent(parent) {}
            void define(std::string name, LasmObject &value);

            std::shared_ptr<LasmObject> get(std::shared_ptr<Token> name);
            void assign(std::shared_ptr<Token> name, LasmObject &value);

            std::shared_ptr<Enviorment> getParent() { return parent; }
            void setParent(std::shared_ptr<Enviorment> parent) { this->parent = parent; }
        private:
            std::map<std::string, std::shared_ptr<LasmObject>> values;
            std::shared_ptr<Enviorment> parent = std::shared_ptr<Enviorment>(nullptr);
    };
}

#endif 
