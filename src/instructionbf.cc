#include "instructionbf.h"
#include "parser.h"
#include "stmt.h"
#include "interpreter.h"
#include "utility.h"

namespace lasm {

    /**
     * Implicit parser
     */
    InstructionParserBfImplicit::InstructionParserBfImplicit(char opcode, InstructionSetBf *is):
        opcode(opcode), is(is) {}

    std::shared_ptr<Stmt> InstructionParserBfImplicit::parse(Parser *parser) {
        auto name = parser->previous();
        std::vector<std::shared_ptr<Expr>> args;
        auto info = std::make_shared<InstructionInfo>(InstructionInfo(is->implicit));
        info->addOpcode(opcode);
        // else just check for ; if not presetn return null
        if (parser->peek()->getType() == SEMICOLON) {
            parser->consume(SEMICOLON, MISSING_SEMICOLON);
        } else {
            return std::shared_ptr<InstructionStmt>(nullptr);
        }

        return std::make_shared<InstructionStmt>(InstructionStmt(name, info, args));
    }

    InstructionResult ImplicitBfGenerator::generate(Interpreter *interpreter,
            std::shared_ptr<InstructionInfo> info, InstructionStmt *stmt) {
        const unsigned int size = 1;

        std::shared_ptr<char[]> data(new char[size]);
        data[0] = info->getOpcode();
        interpreter->setAddress(interpreter->getAddress()+size);
        return InstructionResult(data, size, interpreter->getAddress()-size, stmt->name);
    }

    InstructionSetBf::InstructionSetBf() {
        addInstruction("incp", std::make_shared<InstructionParserBfImplicit>(
                    InstructionParserBfImplicit('>', this)));

        addInstruction("decp", std::make_shared<InstructionParserBfImplicit>(
                    InstructionParserBfImplicit('<', this)));

        addInstruction("incv", std::make_shared<InstructionParserBfImplicit>(
                    InstructionParserBfImplicit('+', this)));

        addInstruction("decv", std::make_shared<InstructionParserBfImplicit>(
                    InstructionParserBfImplicit('-', this)));

        addInstruction("out", std::make_shared<InstructionParserBfImplicit>(
                    InstructionParserBfImplicit('.', this)));

        addInstruction("inp", std::make_shared<InstructionParserBfImplicit>(
                    InstructionParserBfImplicit(',', this)));

        addInstruction("jmpf", std::make_shared<InstructionParserBfImplicit>(
                    InstructionParserBfImplicit('[', this)));

        addInstruction("jmpb", std::make_shared<InstructionParserBfImplicit>(
                    InstructionParserBfImplicit(']', this)));
    }

    InstructionResult InstructionSetBf::generate(Interpreter *interpreter,
            std::shared_ptr<InstructionInfo> info,
            InstructionStmt *stmt) {
        return info->getGenerator()->generate(interpreter, info, stmt);
    }
}
