#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <iostream>
#include <any>
#include "types.h"
#include <vector>
#include <memory>

namespace lasm {
    typedef long lasmNumber;
    typedef double lasmReal;
    typedef std::string lasmString;
    typedef char lasmChar;
    typedef bool lasmBool;
    typedef nullptr_t lasmNil;
    class Callable;

    enum ObjectType {
        NIL_O,
        NUMBER_O,
        REAL_O,
        STRING_O,
        BOOLEAN_O,
        CALLABLE_O
    };

    class LasmObject {
        public:
            LasmObject(ObjectType type, std::any value);

            /**
             * Copy constructor
             */
            LasmObject(LasmObject *original);

            template<typename T>
            T castTo() {
                return std::any_cast<T>(value);
            }

            lasmReal toReal();
            lasmNumber toNumber();
            lasmString toString();
            lasmBool toBool();
            lasmNil toNil();
            std::shared_ptr<Callable> toCallable();

            bool isTruthy() {
                if (isNil()) {
                    return false;
                } else if (isBool()) {
                    return castTo<bool>();
                }
                return true;
            }

            bool isEqual(LasmObject &second) {
                if (type != second.getType()) {
                    return false; // not same type?
                }

                // same type comparse
                switch (type) {
                    case NIL_O:
                        return true;
                    case NUMBER_O:
                        return toNumber() == second.toNumber();
                    case REAL_O:
                        return toReal() == second.toReal();
                    case STRING_O:
                        return toString() == second.toString();
                    case BOOLEAN_O:
                        return toBool() == second.toBool();
                    case CALLABLE_O:
                        return false;
                }

                // should be unreacbable
                return false;
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

            bool isScalar();
        private:
            ObjectType type;
            std::any value;
    };

}

#endif
