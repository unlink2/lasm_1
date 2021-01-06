#include "astprinter.h"

namespace lasm {

    std::string AstPrinter::toString(Expr &expr) {
        return std::any_cast<std::string>(expr.accept(this));
    }

    std::any AstPrinter::visitBinary(BinaryExpr *expr) {
        std::vector<Expr*> v;
        v.push_back(expr->left.get());
        v.push_back(expr->right.get());

        return parenthesize(expr->op->getLexeme(), v);
    }

    std::any AstPrinter::visitGrouping(GroupingExpr *expr) {
        std::vector<Expr*> v;
        v.push_back(expr->expression.get());

        return parenthesize("Group", v);
    }

    std::any AstPrinter::visitLiteral(LiteralExpr *expr) {
        std::stringstream strstream;
        // cast based on what is it
        switch (expr->value.getType()) {
            case NUMBER_O:
                strstream << expr->value.castTo<lasmNumber>();
                break;
            case REAL_O:
                strstream << expr->value.castTo<lasmReal>();
                break;
            case STRING_O:
                strstream << expr->value.castTo<lasmString>();
                break;
            default:
                strstream << "nil";
                break;
        }

        return strstream.str();
    }

    std::any AstPrinter::visitUnary(UnaryExpr *expr) {
        std::vector<Expr*> v;
        v.push_back(expr->right.get());

        return parenthesize(expr->op->getLexeme(), v);
    }

    std::string AstPrinter::parenthesize(std::string name, std::vector<Expr*> exprs) {
        std::stringstream strstream;

        strstream << "(" << name;
        for (auto it = exprs.begin(); it != exprs.end(); it++) {
            strstream << " " << std::any_cast<std::string>((*it)->accept(this));
        }

        strstream << ")";

        return strstream.str();
    }
}
