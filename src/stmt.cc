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

    std::any WhileStmt::accept(StmtVisitor *visitor) {
        return visitor->visitWhile(this);
    }

    std::any FunctionStmt::accept(StmtVisitor *visitor) {
        return visitor->visitFunction(this);
    }

    std::any ReturnStmt::accept(StmtVisitor *visitor) {
        return visitor->visitReturn(this);
    }

    std::any LabelStmt::accept(StmtVisitor *visitor) {
        return visitor->visitLabel(this);
    }


    std::any InstructionStmt::accept(StmtVisitor *visitor) {
        return visitor->visitInstruction(this);
    }

    std::any AlignStmt::accept(StmtVisitor *visitor) {
        return visitor->visitAlign(this);
    }

    std::any OrgStmt::accept(StmtVisitor *visitor) {
        return visitor->visitOrg(this);
    }

    std::any FillStmt::accept(StmtVisitor *visitor) {
        return visitor->visitFill(this);
    }
}
