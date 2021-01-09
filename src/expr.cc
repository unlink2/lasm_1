#include "expr.h"

namespace lasm {
    std::any BinaryExpr::accept(ExprVisitor *visitor) {
        return visitor->visitBinary(this);
    }

    std::any GroupingExpr::accept(ExprVisitor *visitor) {
        return visitor->visitGrouping(this);
    }

    std::any LiteralExpr::accept(ExprVisitor *visitor) {
        return visitor->visitLiteral(this);
    }

    std::any UnaryExpr::accept(ExprVisitor *visitor) {
        return visitor->visitUnary(this);
    }

    std::any VariableExpr::accept(ExprVisitor *visitor) {
        return visitor->visitVariable(this);
    }

    std::any AssignExpr::accept(ExprVisitor *visitor) {
        return visitor->visitAssign(this);
    }

    std::any LogicalExpr::accept(ExprVisitor *visitor) {
        return visitor->visitLogical(this);
    }
}
