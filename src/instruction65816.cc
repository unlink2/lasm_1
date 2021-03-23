#include "instruction65816.h"

#include "parser.h"
#include "stmt.h"
#include "interpreter.h"
#include "utility.h"


namespace lasm {
    std::shared_ptr<Stmt> Set16BitDirective65816::parse(Parser *parser) {
        parser->consume(SEMICOLON, MISSING_SEMICOLON);
        return std::make_shared<DirectiveStmt>(
                DirectiveStmt(parser->previous(), std::vector<std::shared_ptr<Expr>>(),
                    this));
    }

    std::any Set16BitDirective65816::execute(Interpreter *interpreter, DirectiveStmt *stmt) {
        interpreter->getInstructions().setBits(16);
        return std::any();
    }

    std::shared_ptr<Stmt> Set8BitDirective85816::parse(Parser *parser) {
        parser->consume(SEMICOLON, MISSING_SEMICOLON);
        return std::make_shared<DirectiveStmt>(
                DirectiveStmt(parser->previous(), std::vector<std::shared_ptr<Expr>>(),
                    this));
    }

    std::any Set8BitDirective85816::execute(Interpreter *interpreter, DirectiveStmt *stmt) {
        interpreter->getInstructions().setBits(8);
        return std::any();
    }

    InstructionSet65816::InstructionSet65816():
        InstructionSet6502(false) {
            addDirective("m8", std::make_shared<Set8BitDirective85816>(Set8BitDirective85816()));
            addDirective("m16", std::make_shared<Set16BitDirective65816>(Set16BitDirective65816()));
        addOfficialInstructions();
    }

    void InstructionSet65816::addFullInstruction(std::string name, char immediate, char zeropage, char zeropageX,
            char absolute, char absoluteX, char absoluteY, char indirectX, char indirectY,
            char absoluteLong, char absoluteLongX) {
        addInstruction(name, std::make_shared<InstructionParser6502Immediate>(InstructionParser6502Immediate(immediate, this)));

        auto ldaIndirect = std::make_shared<InstructionParser6502Indirect>(
                InstructionParser6502Indirect(this));
        ldaIndirect->withIndirectX(indirectX)->withIndirectY(indirectY);
        addInstruction(name, ldaIndirect);

        auto ldaAbsoluteOrZp = std::make_shared<InstructionParser6502AbsoluteOrZp>(
                InstructionParser6502AbsoluteOrZp(this));
        ldaAbsoluteOrZp->withAbsolute(absolute)->withAbsoluteX(absoluteX)->withAbsoluteY(absoluteY)
            ->withZeropage(zeropage)->withZeropageX(zeropageX)
            ->withAbsoluteLong(absoluteLong)->withAbsoluteLongX(absoluteLongX);
        addInstruction(name, ldaAbsoluteOrZp);
    }

    void InstructionSet65816::addOfficialInstructions() {
        addFullInstruction("adc", 0x69, 0x65, 0x75, 0x6D, 0x7D, 0x79, 0x61, 0x71, 0x6F, 0x7F);
    }
}
