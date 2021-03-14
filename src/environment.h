#ifndef __ENVIORMENT_H__
#define __ENVIORMENT_H__

#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include "object.h"
#include "token.h"
#include "error.h"

// TODO typo here, fix it eventually

namespace lasm {
    class Environment {
        public:
            Environment(std::shared_ptr<Environment> parent=std::shared_ptr<Environment>(nullptr)):
                parent(parent) {}
            void define(std::string name, LasmObject &value);

            std::shared_ptr<LasmObject> get(std::shared_ptr<Token> name);
            void assign(std::shared_ptr<Token> name, LasmObject &value);

            std::shared_ptr<Environment> getParent() { return parent; }
            void setParent(std::shared_ptr<Environment> parent) { this->parent = parent; }

            void clear();

            std::map<std::string, std::shared_ptr<LasmObject>>& getValues() { return values; }

            std::string getName() {
                return name;
            }

            void setName(std::string newName) {
                name = newName;
            }
        private:
            std::map<std::string, std::shared_ptr<LasmObject>> values;
            std::shared_ptr<Environment> parent = std::shared_ptr<Environment>(nullptr);

            // env's name. only used for label export
            std::string name = "";
    };
}

#endif
