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
        LasmObject value = LasmObject(NIL_O, nullptr);
        try {
            value = interpreter->evaluate(stmt->args[0]);
        } catch (LasmTypeError &e) {
            if (interpreter->getPass() != 0) {
                throw e;
            }
        }

        unsigned int size = 2;
        unsigned int maxValue = 0xFF;
        // in 16 size is 3
        if (interpreter->getInstructions().getBits() == 16) {
            size = 3; 
            maxValue = 0xFFFF; 
        }

        std::shared_ptr<char[]> data(new char[size]);
        data[0] = info->getOpcode();
        if (!value.isScalar()) {
            // handle first pass
            if (value.isNil() && interpreter->getPass() == 0) {
                stmt->fullyResolved = false;
                value = LasmObject(NUMBER_O, (lasmNumber)0);
            } else {
                throw LasmException(TYPE_ERROR, stmt->name);
            }
        } else if (value.toNumber() > maxValue) {
            throw LasmException(VALUE_OUT_OF_RANGE, stmt->name);
        }

        if (interpreter->getInstructions().getBits() == 16) {
            data[1] = HI(value.toNumber(), 8);
            data[2] = LO(value.toNumber(), 8);
        } else {
            data[1] = value.toNumber() & maxValue;
        }

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

                    if (enableAbsoluteLongX) {
                        info->addOpcode(absoluteLongX, "absolutelong");
                    }
                } else if (reg->getLexeme() == "y") {
                    if (enableAbsoluteY) {
                        info->addOpcode(absoluteY, "absolute");
                    }

                    if (enableZeropageY) {
                        info->addOpcode(zeropageY, "zeropage");
                    }
                } else if (reg->getLexeme() == "s") {
                    if (enableStackRelative) {
                        // we tread stack relative like zero page since it is a 1-byte constant
                        info->addOpcode(stackRelative, "zeropage");
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

            if (enableAbsoluteLong) {
                info->addOpcode(absoluteLong, "absolutelong");
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
        if (!info->hasOpcode("zeropage") && !info->hasOpcode("absolute") && !info->hasOpcode("absolutelong")) {
            throw LasmException(INVALID_INSTRUCTION, stmt->name);
        }

        LasmObject value = LasmObject(NIL_O, nullptr);
        try {
            value = interpreter->evaluate(stmt->args[0]);
        } catch (LasmTypeError &e) {
            if (interpreter->getPass() != 0) {
                throw e;
            }
        }

        // out of range value
        unsigned int outOfRange = 0xFFFF;
        if (info->hasOpcode("absolutelong")) {
            outOfRange = 0xFFFFFF;
        }

        std::shared_ptr<char[]> data;
        if (!value.isScalar()) {
            // handle first pass
            if (value.isNil() && interpreter->getPass() == 0) {
                stmt->fullyResolved = false;
                value = LasmObject(NUMBER_O, (lasmNumber)0);
            } else {
                throw LasmException(TYPE_ERROR, stmt->name);
            }
        } else if (value.toNumber() > outOfRange) {
            throw LasmException(VALUE_OUT_OF_RANGE, stmt->name);
        } else if ((value.toNumber() > 0xFFFF || !stmt->fullyResolved || !info->hasOpcode("absolute"))
                && info->hasOpcode("absolutelong")) {
            // TODO implement case for absolutelong
            size = 4;
            data = std::shared_ptr<char[]>(new char[size]);
            data[0] = info->getOpcode("absolutelong");
            data[1] = RDBYTE(value.toNumber(), 0, 8);
            data[2] = RDBYTE(value.toNumber(), 1, 8);
            data[3] = RDBYTE(value.toNumber(), 2, 8);
        } else if ((value.toNumber() > 0xFF || !stmt->fullyResolved || !info->hasOpcode("zeropage"))
                && info->hasOpcode("absolute")) {
            size = 3;
            data = std::shared_ptr<char[]>(new char[size]);
            data[0] = info->getOpcode("absolute");
            data[1] = HI(value.toNumber(), 8);
            data[2] = LO(value.toNumber(), 8);
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
                        parser->consume(RIGHT_PAREN, MISSING_RIHGT_PAREN);
                    } else if (reg->getLexeme() == "s") {
                        // stack relative indirect, y
                        parser->consume(RIGHT_PAREN, MISSING_RIHGT_PAREN);
                        parser->consume(COMMA, MISSING_COMMA);
                        if (parser->match(std::vector<TokenType> {IDENTIFIER})
                                && parser->previous()->getLexeme() == "y") {
                            info->addOpcode(stackY, "zeropage");
                        } else {
                            parser->consume(RIGHT_PAREN, MISSING_RIHGT_PAREN);
                        }
                    } else {
                        // invalid instruction
                        parser->consume(RIGHT_PAREN, MISSING_RIHGT_PAREN);
                    }
                } else {
                    // invalid instruction
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
            // if accumulator we need to consume ;
            parser->consume(SEMICOLON, MISSING_SEMICOLON);
        } else {
            // else just check for ; if not presetn return null
            if (parser->peek()->getType() == SEMICOLON) {
                parser->consume(SEMICOLON, MISSING_SEMICOLON);
            } else {
                return std::shared_ptr<InstructionStmt>(nullptr);
            }
        }

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
        LasmObject value = LasmObject(NIL_O, nullptr);
        try {
            value = interpreter->evaluate(stmt->args[0]);
        } catch (LasmTypeError &e) {
            if (interpreter->getPass() != 0) {
                throw e;
            }
        }

        const unsigned int size = 2;

        if (!value.isScalar()) {
            // handle first pass
            if (value.isNil() && interpreter->getPass() == 0) {
                stmt->fullyResolved = false;
                value = LasmObject(NUMBER_O, (lasmNumber)0);
            } else {
                throw LasmException(TYPE_ERROR, stmt->name);
            }
        }

        short offset = value.toNumber() - interpreter->getAddress() - 2;
        if (interpreter->getPass() != 0 && (offset > 127 || offset < -128)) {
            throw LasmException(VALUE_OUT_OF_RANGE, stmt->name);
        } else if (interpreter->getPass() == 0) {
            stmt->fullyResolved = false;
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

    void InstructionSet6502::addHalfInstruction(std::string name, char zeropage, char zeropageX,
                    char absolute, char absoluteX) {
        auto absoluteOrZp = std::make_shared<InstructionParser6502AbsoluteOrZp>(
                InstructionParser6502AbsoluteOrZp(this));
        absoluteOrZp
            ->withAbsolute(absolute)->withAbsoluteX(absoluteX)
            ->withZeropage(zeropage)->withZeropageX(zeropageX);
        addInstruction(name, absoluteOrZp);

    }

    InstructionSet6502::InstructionSet6502(bool init) {
        if (init) {
            addOfficialInstructions();
        }
    }

    void InstructionSet6502::addOfficialInstructions() {
        addFullInstruction("adc", 0x69, 0x65, 0x75, 0x6D, 0x7D, 0x79, 0x61, 0x71);
        addFullInstruction("and", 0x29, 0x25, 0x35, 0x2D, 0x3D, 0x39, 0x21, 0x31);

        // asl
        {
            addInstruction("asl", std::make_shared<InstructionParser6502Implicit>(
                        InstructionParser6502Implicit(0x0A, this, true)));
            addHalfInstruction("asl", 0x06, 0x16, 0x0E, 0x1E);
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
            absoluteOrZp->withAbsolute(0xEC)
                ->withZeropage(0xE4);
            addInstruction(name, absoluteOrZp);
        }

        // cpy
        {
            auto name = "cpy";
            addInstruction(name, std::make_shared<InstructionParser6502Immediate>(InstructionParser6502Immediate(0xC0, this)));

            auto absoluteOrZp = std::make_shared<InstructionParser6502AbsoluteOrZp>(
                    InstructionParser6502AbsoluteOrZp(this));
            absoluteOrZp->withAbsolute(0xCC)
                ->withZeropage(0xC4);
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

        // jsr
        {
            auto name = "jsr";

            auto absoluteOrZp = std::make_shared<InstructionParser6502AbsoluteOrZp>(
                    InstructionParser6502AbsoluteOrZp(this));
            absoluteOrZp->withAbsolute(0x20);
            addInstruction(name, absoluteOrZp);
        }

        // lda
        addFullInstruction("lda", (char)0xA9, (char)0xA5,
                (char)0xB5, (char)0xAD, (char)0xBD,
                (char)0xB9, (char)0xA1, (char)0xB1);

        // ldx
        {
            auto name = "ldx";
            addInstruction(name,
                    std::make_shared<InstructionParser6502Immediate>
                    (InstructionParser6502Immediate(0xA2, this)));

            auto absoluteOrZp = std::make_shared<InstructionParser6502AbsoluteOrZp>(
                    InstructionParser6502AbsoluteOrZp(this));
            absoluteOrZp
                ->withAbsolute(0xAE)->withAbsoluteY(0xBE)
                ->withZeropage(0xA6)->withZeropageY(0xB6);
            addInstruction(name, absoluteOrZp);
        }

        // ldx
        {
            auto name = "ldy";
            addInstruction(name,
                    std::make_shared<InstructionParser6502Immediate>
                    (InstructionParser6502Immediate(0xA0, this)));
            addHalfInstruction(name, 0xA4, 0xB4, 0xAC, 0xBC);
        }

        // lsr
        {
            addInstruction("lsr", std::make_shared<InstructionParser6502Implicit>(
                        InstructionParser6502Implicit(0x4A, this, true)));
            addHalfInstruction("lsr", 0x46, 0x56, 0x4E, 0x5E);
        }

        // nop
        addInstruction("nop", std::make_shared<InstructionParser6502Implicit>(InstructionParser6502Implicit(0xEA, this)));

        // ora
        {
            addFullInstruction("ora", 0x09, 0x05, 0x15, 0x0D, 0x1D, 0x19, 0x01, 0x11);
        }

        // register instructions
        {
            addInstruction("tax", std::make_shared<InstructionParser6502Implicit>(
                        InstructionParser6502Implicit(0x4A, this)));
            addInstruction("txa", std::make_shared<InstructionParser6502Implicit>(
                        InstructionParser6502Implicit(0x8A, this)));
            addInstruction("dex", std::make_shared<InstructionParser6502Implicit>(
                        InstructionParser6502Implicit(0xCA, this)));
            addInstruction("inx", std::make_shared<InstructionParser6502Implicit>(
                        InstructionParser6502Implicit(0xE8, this)));
            addInstruction("tay", std::make_shared<InstructionParser6502Implicit>(
                        InstructionParser6502Implicit(0xA8, this)));
            addInstruction("tya", std::make_shared<InstructionParser6502Implicit>(
                        InstructionParser6502Implicit(0x98, this)));
            addInstruction("dey", std::make_shared<InstructionParser6502Implicit>(
                        InstructionParser6502Implicit(0x88, this)));
            addInstruction("iny", std::make_shared<InstructionParser6502Implicit>(
                        InstructionParser6502Implicit(0xC8, this)));
        }

        // rol
        {
            addInstruction("rol", std::make_shared<InstructionParser6502Implicit>(
                        InstructionParser6502Implicit(0x2A, this, true)));
            addHalfInstruction("rol", 0x26, 0x36, 0x2E, 0x3E);
        }

        // ror
        {
            addInstruction("ror", std::make_shared<InstructionParser6502Implicit>(
                        InstructionParser6502Implicit(0x6A, this, true)));
            addHalfInstruction("ror", 0x66, 0x76, 0x6E, 0x7E);
        }

        // rti
        addInstruction("rti", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0x40, this)));

        // rts
        addInstruction("rts", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0x60, this)));

        // sta
        {
            auto name = "sta";
            auto indirect = std::make_shared<InstructionParser6502Indirect>(
                    InstructionParser6502Indirect(this));
            indirect->withIndirectX(0x81)->withIndirectY(0x91);
            addInstruction(name, indirect);

            auto absoluteOrZp = std::make_shared<InstructionParser6502AbsoluteOrZp>(
                    InstructionParser6502AbsoluteOrZp(this));
            absoluteOrZp->withAbsolute(0x8D)->withAbsoluteX(0x9D)->withAbsoluteY(0x9D)
                ->withZeropage(0x85)->withZeropageX(0x95);
            addInstruction(name, absoluteOrZp);
        }

        // stack
        {
            addInstruction("txs", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0x9A, this)));
            addInstruction("tsx", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0xBA, this)));
            addInstruction("pha", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0x48, this)));
            addInstruction("pla", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0x68, this)));
            addInstruction("php", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0x08, this)));
            addInstruction("plp", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0x28, this)));
        }

        // stx
        {
            auto absoluteOrZp = std::make_shared<InstructionParser6502AbsoluteOrZp>(
                    InstructionParser6502AbsoluteOrZp(this));
            absoluteOrZp
                ->withAbsolute(0x8E)
                ->withZeropage(0x86)->withZeropageY(0x96);
            addInstruction("stx", absoluteOrZp);
        }

        // sty
        {
            auto absoluteOrZp = std::make_shared<InstructionParser6502AbsoluteOrZp>(
                    InstructionParser6502AbsoluteOrZp(this));
            absoluteOrZp
                ->withAbsolute(0x8C)
                ->withZeropage(0x84)->withZeropageX(0x94);
            addInstruction("sty", absoluteOrZp);
        }
    }

    InstructionResult InstructionSet6502::generate(Interpreter *interpreter,
            std::shared_ptr<InstructionInfo> info,
            InstructionStmt *stmt) {
        return info->getGenerator()->generate(interpreter, info, stmt);
    }
};
