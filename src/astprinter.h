#ifndef __ASTPRINTER_H__
#define __ASTPRINTER_H__

#include "token.h"
#include "expr.h"
#include <any>
#include <iostream>
#include <sstream>
#include <vector>
#include <memory>

namespace lasm {
    class AstPrinter: public ExprVisitor {
        public:
            std::string toString(Expr &expr);

            std::any visitBinary(BinaryExpr *expr);
            std::any visitGrouping(GroupingExpr *expr);
            std::any visitLiteral(LiteralExpr *expr);
            std::any visitUnary(UnaryExpr *expr);
        private:
            std::string parenthesize(std::string name, std::vector<Expr*> exprs);
    };
}

#endif
