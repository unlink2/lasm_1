#include "object.h"
#include "error.h"

namespace lasm {
    LasmObject::LasmObject(ObjectType type, std::any value):
        type(type), value(value) {}

    lasmReal LasmObject::toReal() {
        if (isNumber()) {
            return (lasmReal)castTo<lasmNumber>();
        } else if (isReal()) {
            return (lasmReal)castTo<lasmReal>();
        }
        throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, type);
    }

    lasmNumber LasmObject::toNumber() {
        if (isNumber()) {
            return (lasmNumber)castTo<lasmNumber>();
        } else if (isReal()) {
            return (lasmNumber)castTo<lasmReal>();
        }
        throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, type);
    }

    std::string LasmObject::toString() {
        return ""; // TODO
    }
}

