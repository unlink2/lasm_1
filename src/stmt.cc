#include "stmt.h"

namespace lasm {
    std::any ExpressionStmt::accept(StmtVisitor *visitor) {
        return visitor->visitExpression(this);
    }

    std::any LetStmt::accept(StmtVisitor *visitor) {
        return visitor->visitLet(this);
    }

    std::any BlockStmt::accept(StmtVisitor *visitor) {
        return visitor->visitBlock(this);
    }

    std::any IfStmt::accept(StmtVisitor *visitor) {
        return visitor->visitIf(this);
    }
}
