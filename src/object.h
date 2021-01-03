#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <iostream>
#include <any>
#include "types.h"

// TODO test

namespace lasm {
    class LasmLiteral {
        public:
            LasmLiteral(TokenType type, std::any value);
            std::string toString();

            template<typename T>
            T castTo() {
                return std::any_cast<T>(value);
            }

            TokenType getType() {
                return type;
            }
        private:
            TokenType type;
            std::any value;
    };

    class LasmObject {
        public:
            LasmObject(std::any value);
            std::string toString();
        private:
            std::any value;
    };
}

#endif
