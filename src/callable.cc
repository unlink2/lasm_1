#include "callable.h"
#include "environment.h"
#include "interpreter.h"
#include "utility.h"

namespace lasm {
    LasmObject LasmFunction::call(Interpreter *interpreter, std::vector<LasmObject> arguments, CallExpr *expr) {
        std::shared_ptr<Environment> env = std::make_shared<Environment>(Environment(interpreter->getEnv()));

        for (unsigned int i = 0; i < stmt->params.size(); i++) {
            env->define(stmt->params[i]->getLexeme(), arguments[i]);
        }
        try {
            interpreter->executeBlock(stmt->body, env);
            return LasmObject(NIL_O, nullptr);
        } catch (Return &e) {
            return e.value;
        } catch (LasmException &e) {
            // wrap any exception inside a function in another esception to
            // represent the call stack
            throw CallStackUnwind(expr->paren, &e);
        }
    }

    LasmObject NativeHi::call(Interpreter *interpreter, std::vector<LasmObject> arguments, CallExpr *expr) {
        auto num = arguments[0];
        if (num.getType() != NUMBER_O) {
            return LasmObject(NIL_O, nullptr);
        }
        return LasmObject(NUMBER_O, LO(num.toNumber()));
    }

    LasmObject NativeLo::call(Interpreter *interpreter, std::vector<LasmObject> arguments, CallExpr *expr) {
        auto num = arguments[0];
        if (num.getType() != NUMBER_O) {
            return LasmObject(NIL_O, nullptr);
        }
        return LasmObject(NUMBER_O, HI(num.toNumber()));
    }

    LasmObject NativeAddress::call(Interpreter *interpreter, std::vector<LasmObject> arguments, CallExpr *expr) {
        return LasmObject(NUMBER_O, lasmNumber(interpreter->getAddress()));
    }

    LasmObject NativeOrd::call(Interpreter *interpreter, std::vector<LasmObject> arguments, CallExpr *expr) {
        auto c = arguments[0];
        if (!c.isString() || c.toString().length() != 1) {
            return LasmObject(NIL_O, nullptr);
        }
        return LasmObject(NUMBER_O, lasmNumber(c.toString()[0]));
    }

    LasmObject NativeLen::call(Interpreter *interpreter, std::vector<LasmObject> arguments, CallExpr *expr) {
        auto c = arguments[0];
        if (c.isString()) {
            return LasmObject(NUMBER_O, lasmNumber(c.toString().length()));
        } else if (c.isList()) {
            return LasmObject(NUMBER_O, lasmNumber(c.toList()->size()));
        } else {
            return LasmObject(NIL_O, nullptr);
        }
    }

    LasmObject NativeSetEnvName::call(Interpreter *interpreter, std::vector<LasmObject> arguments, CallExpr *expr) {
        auto c = arguments[0];
        interpreter->getEnv()->setName(c.toString());
        interpreter->getLabels()->setName(c.toString());
        return LasmObject(NIL_O, nullptr);
    }
}
