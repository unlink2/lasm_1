#ifndef __CALLABLE_H__
#define __CALLABLE_H__

#include <iostream>
#include <any>
#include <vector>
#include <memory>
#include "object.h"
#include "stmt.h"

namespace lasm {
    class Interpreter;

    class Return {
        public:
            Return(LasmObject value):
                value(value) {}

            LasmObject value;
    };

    class Callable {
        public:
            Callable(unsigned short arity=0):
                arity(arity) {}
            virtual ~Callable() {}
            virtual LasmObject call(Interpreter *interpreter, std::vector<LasmObject> arguments, CallExpr *expr) {
                return LasmObject(NIL_O, nullptr);
            }

            unsigned short getArity() { return arity; }
        private:
            unsigned short arity = 0;
    };

    class LasmFunction: public Callable {
        public:
            LasmFunction(FunctionStmt *stmt):
                Callable::Callable(stmt->params.size()), stmt(stmt) {}

            virtual LasmObject call(Interpreter *interpreter, std::vector<LasmObject> arguments, CallExpr *expr);
        private:
            FunctionStmt *stmt;
    };

    class NativeHi: public Callable {
        public:
            NativeHi():
                Callable::Callable(1) {}
            ~NativeHi() {}

            virtual LasmObject call(Interpreter *interpreter, std::vector<LasmObject> arguments, CallExpr *expr);
    };

    class NativeLo: public Callable {
        public:
            NativeLo():
                Callable::Callable(1) {}
            ~NativeLo() {}

            virtual LasmObject call(Interpreter *interpreter, std::vector<LasmObject> arguments, CallExpr *expr);
    };

    class NativeAddress: public Callable {
        public:
            NativeAddress():
                Callable::Callable(0) {}
            ~NativeAddress() {}

            virtual LasmObject call(Interpreter *interpreter, std::vector<LasmObject> arguments, CallExpr *expr);
    };

    class NativeOrd: public Callable {
        public:
            NativeOrd():
                Callable::Callable(1) {}
            ~NativeOrd() {}

            virtual LasmObject call(Interpreter *interpreter, std::vector<LasmObject> arguments, CallExpr *expr);
    };

    class NativeLen: public Callable {
        public:
            NativeLen():
                Callable::Callable(1) {}
            ~NativeLen() {}

            virtual LasmObject call(Interpreter *interpreter, std::vector<LasmObject> arguments, CallExpr *expr);
    };

    class NativeSetEnvName: public Callable {
        public:
            NativeSetEnvName():
                Callable::Callable(1) {}
            ~NativeSetEnvName() {}

            virtual LasmObject call(Interpreter *interpreter, std::vector<LasmObject> arguments, CallExpr *expr);
    };
}

#endif
