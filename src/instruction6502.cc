#include "instruction6502.h"

namespace lasm {
    InstructionParser6502Generic::InstructionParser6502Generic(char immediate, char zeropage, char zeropageX,
            char absolute, char absoluteX, char absoluteY, char indirectX, char indirectY):
        immediate(immediate), zeropage(zeropage), zeropageX(zeropageX), absolute(absolute),
        absoluteX(absoluteX), absoluteY(absoluteY), indirectX(indirectX), indirectY(indirectY) {}

    std::shared_ptr<Stmt> InstructionParser6502Generic::parse(Parser *parse) {
    }


    InstructionSet6502::InstructionSet6502() {
        // TODO add all instructions
        addInstruction("lda", new InstructionParser6502Generic(0x69, 0x65, 0x75, 0x6D, 0x7D, 0x79, 0x61, 0x71));
    }

    std::shared_ptr<Stmt> InstructionSet6502::parse(Parser *parse) {
    }

    InstructionResult InstructionSet6502::generate(Interpreter *interpreter, std::shared_ptr<InstructionStmt> stmt) {
    }
};
