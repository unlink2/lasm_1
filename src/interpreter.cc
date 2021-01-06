#include "interpreter.h"

namespace lasm {
    Interpreter::Interpreter(BaseError &onError):
        onError(onError) {
    }

    LasmObject Interpreter::evaluate(std::shared_ptr<Expr> expr) {
        return std::any_cast<LasmObject>(expr->accept(this));
    }

    std::any Interpreter::visitBinary(BinaryExpr *expr) {
        auto left = evaluate(expr->left);
        auto right = evaluate(expr->right);

        switch (expr->op->getType()) {
            case MINUS:

            default:
                break;
        }

        // should be unreacbable
        return LasmObject(NIL_O, nullptr);
    }

    std::any Interpreter::visitUnary(UnaryExpr *expr) {
        auto right = evaluate(expr->right);

        switch (expr->op->getType()) {
            case MINUS:
                if (right.isReal()) {
                    return LasmObject(REAL_O, -right.castTo<lasmReal>());
                } else if (right.isNumber()) {
                    return LasmObject(NUMBER_O, -right.castTo<lasmNumber>());
                } else {
                    // type error!
                    throw LasmTypeError(std::vector<ObjectType> {NUMBER_O, REAL_O}, right.getType());
                }
                break;
            case BANG:
                return LasmObject(BOOLEAN_O, !right.isTruthy());
            default:
                break;
        }

        // should be unreacbable
        return LasmObject(NIL_O, nullptr);
    }

    std::any Interpreter::visitLiteral(LiteralExpr *expr) {
        return expr->value;
    }

    std::any Interpreter::visitGrouping(GroupingExpr *expr) {
        return evaluate(expr->expression);
    }
}
