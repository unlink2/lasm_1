#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <iostream>
#include <any>
#include "types.h"

// TODO test

namespace lasm {
    typedef long lasmNumber;
    typedef double lasmReal;
    typedef std::string lasmString;
    typedef char lasmChar;

    enum ObjectType {
        NIL_O,
        NUMBER_O,
        REAL_O,
        STRING_O
    };

    class LasmLiteral {
        public:
            LasmLiteral(ObjectType type, std::any value);
            std::string toString();

            template<typename T>
            T castTo() {
                return std::any_cast<T>(value);
            }

            ObjectType getType() {
                return type;
            }
        private:
            ObjectType type;
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
