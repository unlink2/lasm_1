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

    InstructionParser65816IndirectLong::InstructionParser65816IndirectLong(InstructionSet65816 *is):
     is(is) {}

    std::shared_ptr<Stmt> InstructionParser65816IndirectLong::parse(Parser *parser) {
        auto name = parser->previous();
        if (parser->match(std::vector<TokenType> {LEFT_BRACKET})) {
            auto expr = parser->expression();
            std::vector<std::shared_ptr<Expr>> args;

            args.push_back(expr);

            auto info = std::make_shared<InstructionInfo>(InstructionInfo(is->absolute));
            if (parser->match(std::vector<TokenType> {RIGHT_BRACKET})) {
                if (parser->match(std::vector<TokenType> {COMMA})) {
                    if (parser->match(std::vector<TokenType> {IDENTIFIER})) {
                        auto reg = parser->previous();
                        if (reg->getLexeme() == "y") {
                            if (enableIndirectLongY) {
                                info->addOpcode(indirectLongY, "zeropage");
                            }
                        }
                    }
                } else if (enableIndirectLong) {
                    info->addOpcode(indirectLong, "zeropage");
                }
            }

            parser->consume(SEMICOLON, MISSING_SEMICOLON);

            return std::make_shared<InstructionStmt>(InstructionStmt(name, info, args));
        }

        return std::shared_ptr<Stmt>(nullptr);
    }

    std::shared_ptr<Stmt> InstructionParser65816BlockMove::parse(Parser *parser) {
        // mvp expr, expr
        auto name = parser->previous();
        auto expr1 = parser->expression();
        parser->consume(COMMA, MISSING_COMMA);
        auto expr2 = parser->expression();
        parser->consume(SEMICOLON, MISSING_SEMICOLON);

        std::vector<std::shared_ptr<Expr>> args;
        args.push_back(expr1);
        args.push_back(expr2);

        auto info = std::make_shared<InstructionInfo>(InstructionInfo(is->blockMove));
        info->addOpcode(opcode);
        return std::make_shared<InstructionStmt>(InstructionStmt(name, info, args));
    }

    InstructionResult BlockMove65816Generator::generate(Interpreter *interpreter,
            std::shared_ptr<InstructionInfo> info,
            InstructionStmt *stmt) {
        LasmObject value1 = LasmObject(NIL_O, nullptr);
        LasmObject value2 = LasmObject(NIL_O, nullptr);
        try {
            value1 = interpreter->evaluate(stmt->args[0]);
            value2 = interpreter->evaluate(stmt->args[1]);
        } catch (LasmTypeError &e) {
            if (interpreter->getPass() != 0) {
                throw e;
            }
        }

        const unsigned int size = 3;

        if (!value1.isScalar() || !value2.isScalar()) {
            // handle first pass
            if ((value1.isNil() || value2.isNil()) && interpreter->getPass() == 0) {
                stmt->fullyResolved = false;
                value1 = LasmObject(NUMBER_O, (lasmNumber)0);
                value2 = LasmObject(NUMBER_O, (lasmNumber)0);
            } else {
                throw LasmException(TYPE_ERROR, stmt->name);
            }
        }

        short src = value1.toNumber();
        short dst = value2.toNumber();
        if (interpreter->getPass() != 0 && (src > 255 || dst > 255)) {
            throw LasmException(VALUE_OUT_OF_RANGE, stmt->name);
        } else if (interpreter->getPass() == 0) {
            stmt->fullyResolved = false;
        }

        std::shared_ptr<char[]> data(new char[size]);
        data[0] = info->getOpcode();

        // src, dst in code turns into dst, src in binary output! => this is correct!
        data[1] = (char)dst;
        data[2] = (char)src;
        interpreter->setAddress(interpreter->getAddress()+size);
        return InstructionResult(data, size, interpreter->getAddress()-size, stmt->name);
    }


    /**
     * Instructionset
     */

    void InstructionSet65816::addFullInstruction(std::string name, char immediate,
            char absolute, char absoluteLong, char zeropage, char indirectZp,
            char indirectLong, char absoluteX, char absoluteLongX, char absoluteY,
            char zeropageX, char indirectX, char indirectY, char indirectLongY,
            char stackRelative, char stackY) {
        addInstruction(name, std::make_shared<InstructionParser6502Immediate>(InstructionParser6502Immediate(immediate, this)));

        auto ldaIndirect = std::make_shared<InstructionParser6502Indirect>(
                InstructionParser6502Indirect(this));
        ldaIndirect->withIndirectX(indirectX)->withIndirectY(indirectY)->withStackY(stackY)
            ->withIndirectZp(indirectZp);
        addInstruction(name, ldaIndirect);

        auto ldaIndirectLong = std::make_shared<InstructionParser65816IndirectLong>(
                InstructionParser65816IndirectLong(this));
        ldaIndirectLong->withIndirectLong(indirectLong)
            ->withIndirectLongY(indirectLongY);
        addInstruction(name, ldaIndirectLong);

        auto ldaAbsoluteOrZp = std::make_shared<InstructionParser6502AbsoluteOrZp>(
                InstructionParser6502AbsoluteOrZp(this));
        ldaAbsoluteOrZp->withAbsolute(absolute)->withAbsoluteX(absoluteX)->withAbsoluteY(absoluteY)
            ->withZeropage(zeropage)->withZeropageX(zeropageX)
            ->withAbsoluteLong(absoluteLong)->withAbsoluteLongX(absoluteLongX)
            ->withStackRelative(stackRelative);
        addInstruction(name, ldaAbsoluteOrZp);
    }

    void InstructionSet65816::addOfficialInstructions() {
        // adc
        addFullInstruction("adc", 0x69, 0x6D, 0x6F, 0x65, 0x72, 0x67, 0x7D, 0x7F, 0x79, 0x75, 0x61, 0x71, 0x77, 0x63, 0x73);
        // and
        addFullInstruction("and", 0x29, 0x2D, 0x2F, 0x25, 0x32, 0x27, 0x3D, 0x3F, 0x39, 0x35, 0x21, 0x31, 0x37, 0x23, 0x33);

        // asl
        {
            addInstruction("asl", std::make_shared<InstructionParser6502Implicit>(
                        InstructionParser6502Implicit(0x0A, this, true)));
            addHalfInstruction("asl", 0x06, 0x16, 0x0E, 0x1E);
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

            addInstruction("bra",
                    std::make_shared<InstructionParser6502Relative>(InstructionParser6502Relative(0x80, this)));
        }

        // brl
        {
            auto brlParser = std::make_shared<InstructionParser6502Relative>(
                    InstructionParser6502Relative(0x82, this, this->relativeLong));
            addInstruction("brl", brlParser);
        }

        // bit
        {

            addInstruction("bit",
                    std::make_shared<InstructionParser6502Immediate>
                    (InstructionParser6502Immediate(0x89, this)));
            auto absoluteOrZp = std::make_shared<InstructionParser6502AbsoluteOrZp>(InstructionParser6502AbsoluteOrZp(this));
            absoluteOrZp
                ->withAbsolute(0x24)
                ->withZeropage(0x2C)
                ->withAbsoluteX(0x3C)
                ->withZeropageX(0x34);
            addInstruction("bit", absoluteOrZp);
        }

        // brk
        addInstruction("brk", std::make_shared<InstructionParser6502Implicit>(InstructionParser6502Implicit(0x00, this)));

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

        // cmp
        addFullInstruction("cmp", 0xC9, 0xCD, 0xCF, 0xC5, 0xD2, 0xC7, 0xDD, 0xDF, 0xD9, 0xD5, 0xC1, 0xD1, 0xD7, 0xC3, 0xD3);

        // TODO cop
        {
        }

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

            addInstruction(name, std::make_shared<InstructionParser6502Implicit>(
                        InstructionParser6502Implicit(0x3A, this, true)));

            auto absoluteOrZp = std::make_shared<InstructionParser6502AbsoluteOrZp>(
                    InstructionParser6502AbsoluteOrZp(this));
            absoluteOrZp->withAbsolute(0xCE)->withAbsoluteX(0xDE)
                ->withZeropage(0xC6)->withZeropageX(0xD6);
            addInstruction(name, absoluteOrZp);
        }

        // eor
        addFullInstruction("eor", 0x49, 0x4D, 0x4F, 0x45, 0x52, 0x47, 0x5D, 0x5F, 0x59, 0x55, 0x41, 0x51, 0x57, 0x43, 0x53);

        // inc
        {
            auto name = "inc";

            addInstruction(name, std::make_shared<InstructionParser6502Implicit>(
                        InstructionParser6502Implicit(0x1A, this, true)));

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
            indirect->withIndirect(0x6C)
                ->withIndirectXAbsolute(0x7C);
            addInstruction(name, indirect);

            auto absoluteOrZp = std::make_shared<InstructionParser6502AbsoluteOrZp>(
                    InstructionParser6502AbsoluteOrZp(this));
            absoluteOrZp
                ->withAbsolute(0x4C)
                ->withAbsoluteLong(0x5C);
            addInstruction(name, absoluteOrZp);

            auto indirectLong = std::make_shared<InstructionParser65816IndirectLong>(
                    InstructionParser65816IndirectLong(this));
            indirectLong->withIndirectLong(0xDC);
        }

        // jml
        {
            auto name = "jml";

            auto absoluteOrZp = std::make_shared<InstructionParser6502AbsoluteOrZp>(
                    InstructionParser6502AbsoluteOrZp(this));
            absoluteOrZp
                ->withAbsoluteLong(0x5C);
            addInstruction(name, absoluteOrZp);

            auto indirectLong = std::make_shared<InstructionParser65816IndirectLong>(
                    InstructionParser65816IndirectLong(this));
            indirectLong->withIndirectLong(0xDC);
        }

        // jsr
        {
            auto name = "jsr";

            auto indirect = std::make_shared<InstructionParser6502Indirect>(
                    InstructionParser6502Indirect(this));
            indirect
                ->withIndirectXAbsolute(0xFC);
            addInstruction(name, indirect);

            auto absoluteOrZp = std::make_shared<InstructionParser6502AbsoluteOrZp>(
                    InstructionParser6502AbsoluteOrZp(this));
            absoluteOrZp
                ->withAbsolute(0x20)
                ->withAbsoluteLong(0x22);
            addInstruction(name, absoluteOrZp);
        }

        // jsl
        {
            auto name = "jsl";

            auto absoluteOrZp = std::make_shared<InstructionParser6502AbsoluteOrZp>(
                    InstructionParser6502AbsoluteOrZp(this));
            absoluteOrZp
                ->withAbsoluteLong(0x22);
            addInstruction(name, absoluteOrZp);
        }

        // lda
        addFullInstruction("lda", 0xA9, 0xAD, 0xAF, 0xA5, 0xB2, 0xA7, 0xBD, 0xBF, 0xB9, 0xB5, 0xA1, 0xB1, 0xB7, 0xA3, 0xB3);

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

        // mvn
        {
            auto mvpParser = std::make_shared<InstructionParser65816BlockMove>(
                    InstructionParser65816BlockMove(this, 0x54));
            addInstruction("mvn", mvpParser);
        }

        // mvp
        {
            auto mvpParser = std::make_shared<InstructionParser65816BlockMove>(
                    InstructionParser65816BlockMove(this, 0x44));
            addInstruction("mvp", mvpParser);
        }

        // nop
        addInstruction("nop", std::make_shared<InstructionParser6502Implicit>(InstructionParser6502Implicit(0xEA, this)));

        // ora
        addFullInstruction("ora", 0x09, 0x0D, 0x0F, 0x05, 0x12, 0x07, 0x1D, 0x1F, 0x19, 0x15, 0x01, 0x11, 0x17, 0x03, 0x13);

        // pea
        {
            auto peaParser = std::make_shared<InstructionParser6502AbsoluteOrZp>(
                    InstructionParser6502AbsoluteOrZp(this));
            peaParser
                ->withAbsolute(0xF4);
            addInstruction("pea", peaParser);
        }

        // pei
        {
            auto peiParser = std::make_shared<InstructionParser6502Indirect>(
                    InstructionParser6502Indirect(this));
            peiParser
                ->withIndirectZp(0xD4);
            addInstruction("pei", peiParser);
        }

        // per
        {
            auto peaParser = std::make_shared<InstructionParser6502AbsoluteOrZp>(
                    InstructionParser6502AbsoluteOrZp(this));
            peaParser
                ->withAbsolute(0x62);
            addInstruction("per", peaParser);
        }

        // TODO add all stack
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

            addInstruction("phx", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0xDA, this)));
            addInstruction("plx", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0xFA, this)));

            addInstruction("phy", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0x5A, this)));
            addInstruction("ply", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0x7A, this)));

            addInstruction("phb", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0x8B, this)));
            addInstruction("phd", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0x0B, this)));
            addInstruction("phk", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0x4B, this)));

            addInstruction("plb", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0xAB, this)));
            addInstruction("pld", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0x2B, this)));
        }

        // rep
        {
            auto parser = std::make_shared<InstructionParser6502Immediate>(
                    InstructionParser6502Immediate(0xC2, this));
            parser->force8Bits();
            addInstruction("rep", parser);
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
        // rtl
        addInstruction("rtl", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0x6B, this)));
        // rts
        addInstruction("rts", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0x60, this)));

        // sbc
        addFullInstruction("sbc", 0xE9, 0xED, 0xEF, 0xE5, 0xF2, 0xE7, 0xFD, 0xFF, 0xF9, 0xF5, 0xE1, 0xF1, 0xF7, 0xE3, 0xF3);

        // sep
        {
            auto parser = std::make_shared<InstructionParser6502Immediate>(
                    InstructionParser6502Immediate(0xE2, this));
            parser->force8Bits();
            addInstruction("sep", parser);
        }

        // stp
        addInstruction("stp", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0x01, this)));

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

        // stz
        {
            auto absoluteOrZp = std::make_shared<InstructionParser6502AbsoluteOrZp>(
                    InstructionParser6502AbsoluteOrZp(this));
            absoluteOrZp
                ->withAbsolute(0x9C)
                ->withAbsoluteX(0x9E)
                ->withZeropage(0x64)
                ->withZeropageX(0x74);
            addInstruction("sty", absoluteOrZp);
        }

        // add all register instructions
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

            addInstruction("txy", std::make_shared<InstructionParser6502Implicit>(
                        InstructionParser6502Implicit(0x9B, this)));
            addInstruction("tyx", std::make_shared<InstructionParser6502Implicit>(
                        InstructionParser6502Implicit(0xBB, this)));
        }

        // tcd
        addInstruction("tcd", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0x5B, this)));
        addInstruction("tad", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0x5B, this)));

        // tdc
        addInstruction("tdc", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0x7B, this)));
        addInstruction("tcd", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0x7B, this)));

        // tcs
        addInstruction("tcs", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0x1B, this)));
        addInstruction("tas", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0x1B, this)));

        // tsc
        addInstruction("tsc", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0x3B, this)));
        addInstruction("tsa", std::make_shared<InstructionParser6502Implicit>(
                    InstructionParser6502Implicit(0x3B, this)));

    }
}
