#include "object.h"
#include "error.h"

namespace lasm {
    LasmObject::LasmObject(ObjectType type, std::any value):
        type(type), value(value) {}

    LasmObject::LasmObject(LasmObject *original) {
        type = original->type;
        value = original->value;
    }

    bool LasmObject::isScalar() {
        return type == NUMBER_O || type == REAL_O;
    }

    lasmReal LasmObject::toReal() {
        if (isNumber()) {
            return (lasmReal)castTo<lasmNumber>();
        } else if (isReal()) {
            return (lasmReal)castTo<lasmReal>();
        }

        // this exception should be caught by checking with isScalar to throw a token
        throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, type, std::shared_ptr<Token>(nullptr));
    }

    lasmNumber LasmObject::toNumber() {
        if (isNumber()) {
            return (lasmNumber)castTo<lasmNumber>();
        } else if (isReal()) {
            return (lasmNumber)castTo<lasmReal>();
        }

        // this exception should be caught by checking with isScalar to throw a token
        throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, type, std::shared_ptr<Token>(nullptr));
    }

    lasmString LasmObject::toString() {
        return castTo<lasmString>();
    }

    lasmBool LasmObject::toBool() {
        return castTo<bool>();
    }

    lasmNil LasmObject::toNil() {
        return castTo<nullptr_t>();
    }
}

