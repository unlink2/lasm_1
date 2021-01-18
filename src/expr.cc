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

    std::shared_ptr<Enviorment> VariableExpr::getEnv(unsigned long address) {
        auto it = labels.find(address);
        if (it == labels.end()) {
            return std::shared_ptr<Enviorment>(nullptr);
        }
        return it->second;
    }

    void VariableExpr::setEnv(unsigned long address, std::shared_ptr<Enviorment> env) {
        this->labels[address] = env;
    }

    std::any AssignExpr::accept(ExprVisitor *visitor) {
        return visitor->visitAssign(this);
    }

    std::any LogicalExpr::accept(ExprVisitor *visitor) {
        return visitor->visitLogical(this);
    }

    std::any CallExpr::accept(ExprVisitor *visitor) {
        return visitor->visitCall(this);
    }

    std::any ListExpr::accept(ExprVisitor *visitor) {
        return visitor->visitList(this);
    }

    std::any IndexExpr::accept(ExprVisitor *visitor) {
        return visitor->visitIndex(this);
    }

    std::any IndexAssignExpr::accept(ExprVisitor *visitor) {
        return visitor->visitIndexAssign(this);
    }
}
