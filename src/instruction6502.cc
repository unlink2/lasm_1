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
                } else if (enableIndirect) {
                    // this is only for (jmp). 16 bit indirect
                    info->addOpcode(indirect, "absolute");
                }
            }

            parser->consume(SEMICOLON, MISSING_SEMICOLON);

            return std::make_shared<InstructionStmt>(InstructionStmt(name, info, args));
        }

        return std::shared_ptr<Stmt>(nullptr);
    }

    /**
     * Implicit parser
     */
    InstructionParser6502Implicit::InstructionParser6502Implicit(char opcode, InstructionSet6502 *is, bool allowAccumulator):
        opcode(opcode), is(is), allowAccumulator(allowAccumulator) {}

    std::shared_ptr<Stmt> InstructionParser6502Implicit::parse(Parser *parser) {
        auto name = parser->previous();
        std::vector<std::shared_ptr<Expr>> args;
        auto info = std::make_shared<InstructionInfo>(InstructionInfo(is->implicit));
        info->addOpcode(opcode);

        // allow accumulator name
        if (allowAccumulator && parser->match(std::vector<TokenType> {IDENTIFIER})) {
            auto reg = parser->previous();
            if (reg->getLexeme() != "a") {
                throw ParserException(name, INVALID_INSTRUCTION);
            }
        }

        parser->consume(SEMICOLON, MISSING_SEMICOLON);

        return std::make_shared<InstructionStmt>(InstructionStmt(name, info, args));
    }

    InstructionResult Implicit6502Generator::generate(Interpreter *interpreter,
            std::shared_ptr<InstructionInfo> info, InstructionStmt *stmt) {
        const unsigned int size = 1;

        std::shared_ptr<char[]> data(new char[size]);
        data[0] = info->getOpcode();
        interpreter->setAddress(interpreter->getAddress()+size);
        return InstructionResult(data, size, interpreter->getAddress()-size, stmt->name);
    }

    /**
     * Relative parser (branch)
     */

    InstructionParser6502Relative::InstructionParser6502Relative(char opcode, InstructionSet6502 *is):
        opcode(opcode), is(is) {}

    std::shared_ptr<Stmt> InstructionParser6502Relative::parse(Parser *parser) {
        auto name = parser->previous();
        // immediate
        auto expr = parser->expression();
        std::vector<std::shared_ptr<Expr>> args;

        args.push_back(expr);

        auto info = std::make_shared<InstructionInfo>(InstructionInfo(is->relative));
        info->addOpcode(opcode);

        parser->consume(SEMICOLON, MISSING_SEMICOLON);

        return std::make_shared<InstructionStmt>(InstructionStmt(name, info, args));
    }

    InstructionResult Relative6502Generator::generate(Interpreter *interpreter,
            std::shared_ptr<InstructionInfo> info,
            InstructionStmt *stmt) {
        auto value = interpreter->evaluate(stmt->args[0]);

        const unsigned int size = 2;

        if (!value.isScalar()) {
            // handle first pass
            if (value.isNil() && interpreter->getPass() == 0) {
                value = LasmObject(NUMBER_O, (lasmNumber)0);
            } else {
                throw LasmException(TYPE_ERROR, stmt->name);
            }
        }

        short offset = value.toNumber() - interpreter->getAddress() - 2;
        if (interpreter->getPass() != 0 && (offset > 127 || offset < -128)) {
            throw LasmException(VALUE_OUT_OF_RANGE, stmt->name);
        } else if (interpreter->getPass() == 0) {
            offset = 0;
        }
        std::shared_ptr<char[]> data(new char[size]);
        data[0] = info->getOpcode();
        data[1] = (char)offset;
        interpreter->setAddress(interpreter->getAddress()+size);
        return InstructionResult(data, size, interpreter->getAddress()-size, stmt->name);
    }

    /**
     * Instruction set
     */
    void InstructionSet6502::addFullInstruction(std::string name, char immediate, char zeropage, char zeropageX,
            char absolute, char absoluteX, char absoluteY, char indirectX, char indirectY) {
        addInstruction(name, std::make_shared<InstructionParser6502Immediate>(InstructionParser6502Immediate(immediate, this)));

        auto ldaIndirect = std::make_shared<InstructionParser6502Indirect>(
                InstructionParser6502Indirect(this));
        ldaIndirect->withIndirectX(indirectX)->withIndirectY(indirectY);
        addInstruction(name, ldaIndirect);

        auto ldaAbsoluteOrZp = std::make_shared<InstructionParser6502AbsoluteOrZp>(
                InstructionParser6502AbsoluteOrZp(this));
        ldaAbsoluteOrZp->withAbsolute(absolute)->withAbsoluteX(absoluteX)->withAbsoluteY(absoluteY)
            ->withZeropage(zeropage)->withZeropageX(zeropageX);
        addInstruction(name, ldaAbsoluteOrZp);
    }

    InstructionSet6502::InstructionSet6502() {
        // TODO add all instructions

        addFullInstruction("adc", 0x69, 0x65, 0x75, 0x6D, 0x7D, 0x79, 0x61, 0x71);
        addFullInstruction("and", 0x29, 0x25, 0x35, 0x2D, 0x3D, 0x39, 0x21, 0x31);

        // asl
        {
            addInstruction("asl", std::make_shared<InstructionParser6502Implicit>(
                        InstructionParser6502Implicit(0x0A, this, true)));

            auto absoluteOrZp = std::make_shared<InstructionParser6502AbsoluteOrZp>(
                    InstructionParser6502AbsoluteOrZp(this));
            absoluteOrZp->withAbsolute(0x0E)->withAbsoluteX(0x1E)
                ->withZeropage(0x06)->withZeropageX(0x16);
            addInstruction("asl", absoluteOrZp);
        }

        // bit
        {
            auto absoluteOrZp = std::make_shared<InstructionParser6502AbsoluteOrZp>(InstructionParser6502AbsoluteOrZp(this));
            absoluteOrZp->withAbsolute(0x24)
                ->withZeropage(0x2C);
            addInstruction("bit", absoluteOrZp);
        }

        // branches
        {
            addInstruction("bpl",
                    std::make_shared<InstructionParser6502Relative>(InstructionParser6502Relative(0x10, this)));
            addInstruction("bmi",
                    std::make_shared<InstructionParser6502Relative>(InstructionParser6502Relative(0x30, this)));
            addInstruction("bvc",
                    std::make_shared<InstructionParser6502Relative>(InstructionParser6502Relative(0x50, this)));
            addInstruction("bvs",
                    std::make_shared<InstructionParser6502Relative>(InstructionParser6502Relative(0x70, this)));
            addInstruction("bcc",
                    std::make_shared<InstructionParser6502Relative>(InstructionParser6502Relative(0x90, this)));
            addInstruction("bcs",
                    std::make_shared<InstructionParser6502Relative>(InstructionParser6502Relative(0xB0, this)));
            addInstruction("bne",
                    std::make_shared<InstructionParser6502Relative>(InstructionParser6502Relative(0xD0, this)));
            addInstruction("beq",
                    std::make_shared<InstructionParser6502Relative>(InstructionParser6502Relative(0xF0, this)));
        }

        // brk
        addInstruction("brk", std::make_shared<InstructionParser6502Implicit>(InstructionParser6502Implicit(0x00, this)));

        // cmp
        addFullInstruction("cmp", 0xC9, 0xC5, 0xD5, 0xCD, 0xDD, 0xD9, 0xC1, 0xD1);

        // cpx
        {
            auto name = "cpx";
            addInstruction(name, std::make_shared<InstructionParser6502Immediate>(InstructionParser6502Immediate(0xE0, this)));

            auto absoluteOrZp = std::make_shared<InstructionParser6502AbsoluteOrZp>(
                    InstructionParser6502AbsoluteOrZp(this));
            absoluteOrZp->withAbsolute(0xE4)
                ->withZeropage(0xEC);
            addInstruction(name, absoluteOrZp);
        }

        // cpy
        {
            auto name = "cpx";
            addInstruction(name, std::make_shared<InstructionParser6502Immediate>(InstructionParser6502Immediate(0xC0, this)));

            auto absoluteOrZp = std::make_shared<InstructionParser6502AbsoluteOrZp>(
                    InstructionParser6502AbsoluteOrZp(this));
            absoluteOrZp->withAbsolute(0xC4)
                ->withZeropage(0xCC);
            addInstruction(name, absoluteOrZp);
        }

        // dec
        {
            auto name = "dec";

            auto absoluteOrZp = std::make_shared<InstructionParser6502AbsoluteOrZp>(
                    InstructionParser6502AbsoluteOrZp(this));
            absoluteOrZp->withAbsolute(0xCE)->withAbsoluteX(0xDE)
                ->withZeropage(0xC6)->withZeropageX(0xD6);
            addInstruction(name, absoluteOrZp);
        }

        // eor
        addFullInstruction("eor", 0x49, 0x45, 0x55, 0x4D, 0x5D, 0x59, 0x41, 0x51);

        // flags
        {
            addInstruction("clc",
                    std::make_shared<InstructionParser6502Implicit>(InstructionParser6502Implicit(0x18, this)));
            addInstruction("sec",
                    std::make_shared<InstructionParser6502Implicit>(InstructionParser6502Implicit(0x38, this)));
            addInstruction("cli",
                    std::make_shared<InstructionParser6502Implicit>(InstructionParser6502Implicit(0x58, this)));
            addInstruction("sei",
                    std::make_shared<InstructionParser6502Implicit>(InstructionParser6502Implicit(0x78, this)));
            addInstruction("clv",
                    std::make_shared<InstructionParser6502Implicit>(InstructionParser6502Implicit(0xB8, this)));
            addInstruction("cld",
                    std::make_shared<InstructionParser6502Implicit>(InstructionParser6502Implicit(0xD8, this)));
            addInstruction("sed",
                    std::make_shared<InstructionParser6502Implicit>(InstructionParser6502Implicit(0xF8, this)));
        }

        // inc
        {
            auto name = "inc";

            auto absoluteOrZp = std::make_shared<InstructionParser6502AbsoluteOrZp>(
                    InstructionParser6502AbsoluteOrZp(this));
            absoluteOrZp->withAbsolute(0xEE)->withAbsoluteX(0xFE)
                ->withZeropage(0xE6)->withZeropageX(0xF6);
            addInstruction(name, absoluteOrZp);
        }

        // jmp
        {
            auto name = "jmp";

            auto indirect = std::make_shared<InstructionParser6502Indirect>(
                    InstructionParser6502Indirect(this));
            indirect->withIndirect(0x6C);
            addInstruction(name, indirect);

            auto absoluteOrZp = std::make_shared<InstructionParser6502AbsoluteOrZp>(
                    InstructionParser6502AbsoluteOrZp(this));
            absoluteOrZp->withAbsolute(0x4C);
            addInstruction(name, absoluteOrZp);
        }

        // nop
        addInstruction("nop", std::make_shared<InstructionParser6502Implicit>(InstructionParser6502Implicit(0xEA, this)));
    }

    InstructionResult InstructionSet6502::generate(Interpreter *interpreter,
            std::shared_ptr<InstructionInfo> info,
            InstructionStmt *stmt) {
        return info->getGenerator()->generate(interpreter, info, stmt);
    }
};
