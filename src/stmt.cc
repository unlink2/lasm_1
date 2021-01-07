#include "stmt.h"

namespace lasm {
    std::any ExpressionStmt::accept(StmtVisitor *visitor) {
        return visitor->visitExpression(this);
    }
}
