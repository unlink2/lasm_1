#include "instruction6502.h"
#include "parser.h"
#include "stmt.h"
#include "interpreter.h"

namespace lasm {
    InstructionParser6502Generic::InstructionParser6502Generic(char immediate, char zeropage, char zeropageX,
            char absolute, char absoluteX, char absoluteY, char indirectX, char indirectY,
            InstructionSet6502 *is):
        immediate(immediate), zeropage(zeropage), zeropageX(zeropageX), absolute(absolute),
        absoluteX(absoluteX), absoluteY(absoluteY), indirectX(indirectX), indirectY(indirectY),
        is(is) {}

    std::shared_ptr<Stmt> InstructionParser6502Generic::parse(Parser *parser) {
        auto name = parser->previous();
        if (parser->match(std::vector<TokenType> {HASH})) {
            // immediate
            return parseImmediate(parser, name);
        }
        return std::shared_ptr<Stmt>(nullptr);
    }

    std::shared_ptr<Stmt> InstructionParser6502Generic::parseImmediate(Parser *parser, std::shared_ptr<Token> name) {
        auto expr = parser->expression();
        std::vector<std::shared_ptr<Expr>> args;

        args.push_back(expr);

        auto info = std::make_shared<InstructionInfo>(InstructionInfo(immediate, 2,
                    is->immediate));

        parser->consume(SEMICOLON, MISSING_SEMICOLON);

        return std::make_shared<InstructionStmt>(InstructionStmt(name, info, args));
    }

    InstructionResult Immediate6502Generator::generate(Interpreter *interpreter,
            std::shared_ptr<InstructionInfo> info, InstructionStmt *stmt) {
        auto value = interpreter->evaluate(stmt->args[0]);

        std::shared_ptr<char[]> data(new char[2]);
        data[0] = info->getOpcode();
        if (!value.isScalar()) {
            throw LasmException(TYPE_ERROR, stmt->name);
        } else if (value.toNumber() > 0xFF) {
            throw LasmException(VALUE_OUT_OF_RANGE, stmt->name);
        }
        data[1] = value.toNumber() & 0xFF;
        interpreter->setAddress(interpreter->getAddress()+info->getSize());
        return InstructionResult(data, info->getSize(), interpreter->getAddress()-info->getSize(), stmt->name);
    }

    InstructionSet6502::InstructionSet6502() {
        // TODO add all instructions
        addInstruction("lda", new InstructionParser6502Generic(0x69, 0x65, 0x75, 0x6D, 0x7D, 0x79, 0x61, 0x71, this));
    }

    std::shared_ptr<Stmt> InstructionSet6502::parse(Parser *parser) {
        auto name = parser->previous()->getLexeme();
        return instructions.find(name)->second->parse(parser);
    }

    InstructionResult InstructionSet6502::generate(Interpreter *interpreter,
            std::shared_ptr<InstructionInfo> info,
            InstructionStmt *stmt) {
        return info->getGenerator()->generate(interpreter, info, stmt);
    }
};
