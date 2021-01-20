#include "instruction6502.h"
#include "parser.h"
#include "stmt.h"
#include "interpreter.h"
#include "utility.h"

namespace lasm {
    /**
     * Immediate
     */
    InstructionParser6502Immediate::InstructionParser6502Immediate(char immediate,
            InstructionSet6502 *is):
        immediate(immediate),
        is(is) {}

    std::shared_ptr<Stmt> InstructionParser6502Immediate::parse(Parser *parser) {
        auto name = parser->previous();
        if (parser->match(std::vector<TokenType> {HASH})) {
            // immediate
            auto expr = parser->expression();
            std::vector<std::shared_ptr<Expr>> args;

            args.push_back(expr);

            auto info = std::make_shared<InstructionInfo>(InstructionInfo(is->immediate));
            info->addOpcode(immediate);

            parser->consume(SEMICOLON, MISSING_SEMICOLON);

            return std::make_shared<InstructionStmt>(InstructionStmt(name, info, args));
        }
        return std::shared_ptr<Stmt>(nullptr);
    }

    InstructionResult Immediate6502Generator::generate(Interpreter *interpreter,
            std::shared_ptr<InstructionInfo> info, InstructionStmt *stmt) {
        auto value = interpreter->evaluate(stmt->args[0]);

        const unsigned int size = 2;

        std::shared_ptr<char[]> data(new char[size]);
        data[0] = info->getOpcode();
        if (!value.isScalar()) {
            // handle first pass
            if (value.isNil() && interpreter->getPass() == 0) {
                value = LasmObject(NUMBER_O, (lasmNumber)0);
            } else {
                throw LasmException(TYPE_ERROR, stmt->name);
            }
        } else if (value.toNumber() > 0xFF) {
            throw LasmException(VALUE_OUT_OF_RANGE, stmt->name);
        }
        data[1] = value.toNumber() & 0xFF;
        interpreter->setAddress(interpreter->getAddress()+size);
        return InstructionResult(data, size, interpreter->getAddress()-size, stmt->name);
    }

    /**
     * Absolute
     */
    InstructionParser6502AbsoluteOrZp::InstructionParser6502AbsoluteOrZp(InstructionSet6502 *is): is(is) {}

    std::shared_ptr<Stmt> InstructionParser6502AbsoluteOrZp::parse(Parser *parser) {
        auto name = parser->previous();
        auto expr = parser->expression();
        std::vector<std::shared_ptr<Expr>> args;

        args.push_back(expr);

        auto info = std::make_shared<InstructionInfo>(InstructionInfo(is->absolute));

        if (parser->match(std::vector<TokenType> {COMMA})) {
            if (parser->match(std::vector<TokenType> {IDENTIFIER})) {
                auto reg = parser->previous();
                if (reg->getLexeme() == "x") {
                    if (enableAbsoluteX) {
                        info->addOpcode(absoluteX, "absolute");
                    }

                    if (enableZeropageX) {
                        info->addOpcode(zeropageX, "zeropage");
                    }
                } else if (reg->getLexeme() == "y") {
                    if (enableAbsoluteY) {
                        info->addOpcode(absoluteY, "absolute");
                    }

                    if (enableZeropageY) {
                        info->addOpcode(zeropageY, "zeropage");
                    }
                } else {
                    throw ParserException(parser->previous(), INVALID_INSTRUCTION);
                }
            } else {
                throw ParserException(parser->previous(), INVALID_INSTRUCTION);
            }
        } else {
            if (enableAbsolute) {
                info->addOpcode(absolute, "absolute");
            }

            if (enableZeropage) {
                info->addOpcode(zeropage, "zeropage");
            }
        }

        parser->consume(SEMICOLON, MISSING_SEMICOLON);

        return std::make_shared<InstructionStmt>(InstructionStmt(name, info, args));
    }

    InstructionResult AbsoluteOrZp6502Generator::generate(Interpreter *interpreter,
            std::shared_ptr<InstructionInfo> info,
            InstructionStmt *stmt) {

        unsigned int size = 3;

        // no opcode at all? bad instruction!
        if (!info->hasOpcode("zeropage") && !info->hasOpcode("absolute")) {
            throw LasmException(INVALID_INSTRUCTION, stmt->name);
        }

        auto value = interpreter->evaluate(stmt->args[0]);
        std::shared_ptr<char[]> data;
        if (!value.isScalar()) {
            // handle first pass
            if (value.isNil() && interpreter->getPass() == 0) {
                value = LasmObject(NUMBER_O, (lasmNumber)0);
            } else {
                throw LasmException(TYPE_ERROR, stmt->name);
            }
        } else if (value.toNumber() > 0xFFFF) {
            throw LasmException(VALUE_OUT_OF_RANGE, stmt->name);
        } else if ((value.toNumber() > 0xFF || !info->hasOpcode("zeropage")) && info->hasOpcode("absolute")) {
            size = 3;
            data = std::shared_ptr<char[]>(new char[size]);
            data[0] = info->getOpcode("absolute");
            data[1] = HI(value.toNumber());
            data[2] = LO(value.toNumber());
        } else {
            if (value.toNumber() > 0xFF) {
                throw LasmException(VALUE_OUT_OF_RANGE, stmt->name);
            }
            size = 2;
            data = std::shared_ptr<char[]>(new char[size]);
            data[0] = info->getOpcode("zeropage");
            data[1] = value.toNumber();
        }

        interpreter->setAddress(interpreter->getAddress()+size);
        return InstructionResult(data, size, interpreter->getAddress()-size, stmt->name);
    }


    /**
     * Indirect parser
     */

    InstructionParser6502Indirect::InstructionParser6502Indirect(InstructionSet6502 *is): is(is) {}

    std::shared_ptr<Stmt> InstructionParser6502Indirect::parse(Parser *parser) {
        auto name = parser->previous();
        if (parser->match(std::vector<TokenType> {LEFT_PAREN})) {
            auto expr = parser->expression();
            std::vector<std::shared_ptr<Expr>> args;

            args.push_back(expr);

            auto info = std::make_shared<InstructionInfo>(InstructionInfo(is->absolute));
            if (parser->match(std::vector<TokenType> {COMMA})) {
                if (parser->match(std::vector<TokenType> {IDENTIFIER})) {
                    auto reg = parser->previous();
                    if (reg->getLexeme() == "x") {
                        if (enableIndirectX) {
                            info->addOpcode(indirectX, "zeropage");
                        }
                    }
                    parser->consume(RIGHT_PAREN, MISSING_RIHGT_PAREN);
                }
            } else if (parser->match(std::vector<TokenType> {RIGHT_PAREN})) {
                if (parser->match(std::vector<TokenType> {COMMA})) {
                    if (parser->match(std::vector<TokenType> {IDENTIFIER})) {
                        auto reg = parser->previous();
                        if (reg->getLexeme() == "y") {
                            if (enableIndirectY) {
                                info->addOpcode(indirectY, "zeropage");
                            }
                        }
                    }
                }
            }

            parser->consume(SEMICOLON, MISSING_SEMICOLON);

            return std::make_shared<InstructionStmt>(InstructionStmt(name, info, args));
        }

        return std::shared_ptr<Stmt>(nullptr);
    }

    /**
     * Instruction set
     */

    InstructionSet6502::InstructionSet6502() {
        // TODO add all instructions

        // lda
        {
            addInstruction("lda", std::make_shared<InstructionParser6502Immediate>(InstructionParser6502Immediate(0x69, this)));

            auto ldaIndirect = std::make_shared<InstructionParser6502Indirect>(
                    InstructionParser6502Indirect(this));
            ldaIndirect->withIndirectX(0x61)->withIndirectY(0x71);
            addInstruction("lda", ldaIndirect);

            auto ldaAbsoluteOrZp = std::make_shared<InstructionParser6502AbsoluteOrZp>(
                    InstructionParser6502AbsoluteOrZp(this));
            ldaAbsoluteOrZp->withAbsolute(0x6D)->withAbsoluteX(0x7D)->withAbsoluteY(0x79)
                ->withZeropage(0x65)->withZeropageX(0x75);
            addInstruction("lda", ldaAbsoluteOrZp);
        }
    }

    InstructionResult InstructionSet6502::generate(Interpreter *interpreter,
            std::shared_ptr<InstructionInfo> info,
            InstructionStmt *stmt) {
        return info->getGenerator()->generate(interpreter, info, stmt);
    }
};
