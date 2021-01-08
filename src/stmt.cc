#include "stmt.h"

namespace lasm {
    std::any ExpressionStmt::accept(StmtVisitor *visitor) {
        return visitor->visitExpression(this);
    }

    std::any LetStmt::accept(StmtVisitor *visitor) {
        return visitor->visitLet(this);
    }
}
