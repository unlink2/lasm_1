#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <iostream>
#include <any>
#include "types.h"
#include <vector>

// TODO test

namespace lasm {
    typedef long lasmNumber;
    typedef double lasmReal;
    typedef std::string lasmString;
    typedef char lasmChar;
    typedef bool lasmBool;

    enum ObjectType {
        NIL_O,
        NUMBER_O,
        REAL_O,
        STRING_O,
        BOOLEAN_O
    };

    class LasmObject {
        public:
            LasmObject(ObjectType type, std::any value);
            std::string toString();

            template<typename T>
            T castTo() {
                return std::any_cast<T>(value);
            }

            lasmReal toReal();
            lasmNumber toNumber();

            bool isTruthy() {
                if (isNil()) {
                    return false;
                } else if (isBool()) {
                    return castTo<bool>();
                }
                return true;
            }

            ObjectType getType() {
                return type;
            }

            bool isNumber() {
                return type == NUMBER_O;
            }

            bool isReal() {
                return type == REAL_O;
            }

            bool isBool() {
                return type == BOOLEAN_O;
            }

            bool isNil() {
                return type == NIL_O;
            }

            bool isString() {
                return type == STRING_O;
            }
        private:
            ObjectType type;
            std::any value;
    };

}

#endif
