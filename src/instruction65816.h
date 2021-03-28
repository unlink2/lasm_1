#ifndef __INSTRUCTION65C816_H__
#define __INSTRUCTION65C816_H__

#include "instruction6502.h"
#include "instruction.h"
#include "types.h"
#include "token.h"


namespace lasm {
    /**
     * 65816
     */
    class InstructionSet65816;

    class Set16BitDirective65816: public Directive {
        public:
            virtual std::shared_ptr<Stmt> parse(Parser *parser);
            virtual std::any execute(Interpreter *interpreter, DirectiveStmt *stmt);
    };

    class Set8BitDirective85816: public Directive {
        public:
            virtual std::shared_ptr<Stmt> parse(Parser *parser);
            virtual std::any execute(Interpreter *interpreter, DirectiveStmt *stmt);
    };


    /**
     * Indirect long
     */
    class InstructionParser65816IndirectLong: public InstructionParser {
        public:
            InstructionParser65816IndirectLong(InstructionSet65816 *is);
            virtual std::shared_ptr<Stmt> parse(Parser *parser);

            InstructionParser65816IndirectLong* withIndirectLong(char opcode) {
                indirectLong = opcode;
                enableIndirectLong = true;
                return this;
            }

            InstructionParser65816IndirectLong* withIndirectLongY(char opcode) {
                indirectLongY = opcode;
                enableIndirectLongY = true;
                return this;
            }
        private:
            char indirectLong;
            char indirectLongY;

            bool enableIndirectLong = false;
            bool enableIndirectLongY = false;
            InstructionSet65816 *is;
    };

    /**
     * Block move
     */
    class InstructionParser65816BlockMove: public InstructionParser {
        public:
            InstructionParser65816BlockMove(InstructionSet65816 *is, char opcode):
                is(is), opcode(opcode) {}

            virtual std::shared_ptr<Stmt> parse(Parser *parser);
        private:
            InstructionSet65816 *is;
            char opcode;
    };

    class BlockMove65816Generator: public InstructionGenerator {
        public:
            virtual InstructionResult generate(Interpreter *interpreter,
                    std::shared_ptr<InstructionInfo> info,
                    InstructionStmt *stmt);
    };

    class InstructionSet65816: public InstructionSet6502 {
        public:
            InstructionSet65816();

            virtual void addOfficialInstructions();

            std::shared_ptr<BlockMove65816Generator> blockMove = std::make_shared<BlockMove65816Generator>(BlockMove65816Generator());
            std::shared_ptr<Relative6502Generator> relativeLong = std::make_shared<Relative6502Generator>(Relative6502Generator(16));
        private:
            void addFullInstruction(std::string name, char immediate, char zeropage, char zeropageX,
                    char absolute, char absoluteX, char absoluteY, char indirectX, char indirectY,
                    char absoluteLong, char absoluteLongX, char stackRelative,
                    char stackY, char indirectLong, char indirecgLongY);
    };
}

#endif
