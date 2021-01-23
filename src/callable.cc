#include "callable.h"
#include "enviorment.h"
#include "interpreter.h"
#include "utility.h"

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
        return LasmObject(NUMBER_O, LO(num.toNumber()));
    }

    LasmObject NativeLo::call(Interpreter *interpreter, std::vector<LasmObject> arguments) {
        auto num = arguments[0];
        if (num.getType() != NUMBER_O) {
            return LasmObject(NIL_O, nullptr);
        }
        return LasmObject(NUMBER_O, HI(num.toNumber()));
    }

    LasmObject NativeAddress::call(Interpreter *interpreter, std::vector<LasmObject> arguments) {
        return LasmObject(NUMBER_O, lasmNumber(interpreter->getAddress()));
    }

    LasmObject NativeOrd::call(Interpreter *interpreter, std::vector<LasmObject> arguments) {
        auto c = arguments[0];
        if (!c.isString() || c.toString().length() != 1) {
            return LasmObject(NIL_O, nullptr);
        }
        return LasmObject(NUMBER_O, lasmNumber(c.toString()[0]));
    }
}
