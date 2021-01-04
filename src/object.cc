#include "object.h"

namespace lasm {
    LasmLiteral::LasmLiteral(ObjectType type, std::any value):
        type(type), value(value) {}

    std::string LasmLiteral::toString() {
        return ""; // TODO
    }

    LasmObject::LasmObject(std::any value):
        value(value) {
    }

    std::string LasmObject::toString() {
        return ""; // TODO
    }
}

