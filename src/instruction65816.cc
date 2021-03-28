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

    void InstructionSet65816::addFullInstruction(std::string name, char immediate, char zeropage, char zeropageX,
            char absolute, char absoluteX, char absoluteY, char indirectX, char indirectY,
            char absoluteLong, char absoluteLongX, char stackRelative, char stackY,
            char indirectLong, char indirectLongY) {
        addInstruction(name, std::make_shared<InstructionParser6502Immediate>(InstructionParser6502Immediate(immediate, this)));

        auto ldaIndirect = std::make_shared<InstructionParser6502Indirect>(
                InstructionParser6502Indirect(this));
        ldaIndirect->withIndirectX(indirectX)->withIndirectY(indirectY)->withStackY(stackY);
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
        addFullInstruction("adc", 0x69, 0x65, 0x75, 0x6D, 0x7D, 0x79, 0x61, 0x71, 0x6F, 0x7F, 0x63, 0x73, 0x67, 0x77);

        // brl
        {
            auto brlParser = std::make_shared<InstructionParser6502Relative>(
                    InstructionParser6502Relative(0x82, this, this->relativeLong));
            addInstruction("brl", brlParser);
        }

        // mvp
        {
            auto mvpParser = std::make_shared<InstructionParser65816BlockMove>(
                    InstructionParser65816BlockMove(this, 0x54));
            addInstruction("mvp", mvpParser);
        }
    }
}
