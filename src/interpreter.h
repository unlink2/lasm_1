#ifndef __INTERPRETER_H__
#define __INTERPRETER_H__

#include <iostream>
#include <memory>
#include <vector>
#include <any>
#include "expr.h"
#include "object.h"
#include "error.h"

namespace lasm {
    class Interpreter: public ExprVisitor {
        public:
            Interpreter(BaseError &onError);

            LasmObject evaluate(std::shared_ptr<Expr> expr);

            std::any visitBinary(BinaryExpr *expr);
            std::any visitUnary(UnaryExpr *expr);
            std::any visitLiteral(LiteralExpr *expr);
            std::any visitGrouping(GroupingExpr *expr);

        private:
            BaseError &onError;
    };
}

#endif
