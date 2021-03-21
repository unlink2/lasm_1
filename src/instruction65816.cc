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
        InstructionSet6502() {
            addDirective("m8", std::make_shared<Set8BitDirective85816>(Set8BitDirective85816()));
            addDirective("m16", std::make_shared<Set16BitDirective65816>(Set16BitDirective65816()));
    }
}
