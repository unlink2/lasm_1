#include "callable.h"

namespace lasm {
    LasmObject NativeHi::call(Interpreter *interpreter, std::vector<LasmObject> arguments) {
        auto num = arguments[0];
        if (num.getType() != NUMBER_O) {
            return LasmObject(NIL_O, nullptr);
        }
        return LasmObject(NUMBER_O, (num.toNumber() >> 8) & 0xFF);
    }

    LasmObject NativeLo::call(Interpreter *interpreter, std::vector<LasmObject> arguments) {
        auto num = arguments[0];
        if (num.getType() != NUMBER_O) {
            return LasmObject(NIL_O, nullptr);
        }
        return LasmObject(NUMBER_O, num.toNumber() & 0xFF);
    }
}
