#include "callable.h"
#include "enviorment.h"
#include "interpreter.h"

namespace lasm {
    LasmObject LasmFunction::call(Interpreter *interpreter, std::vector<LasmObject> arguments) {
        std::shared_ptr<Enviorment> env = std::make_shared<Enviorment>(Enviorment(interpreter->getEnv()));

        for (unsigned int i = 0; i < stmt->params.size(); i++) {
            env->define(stmt->params[i]->getLexeme(), arguments[i]);
        }
        try {
            interpreter->executeBlock(stmt->body, env);
            return LasmObject(NIL_O, nullptr);
        } catch (Return &e) {
            return e.value;
        }
    }

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

    LasmObject NativeAddress::call(Interpreter *interpreter, std::vector<LasmObject> arguments) {
        return LasmObject(NUMBER_O, lasmNumber(interpreter->getAddress()));
    }
}
