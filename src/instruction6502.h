#ifndef __INSTRUCTION6502_H__
#define __INSTRUCTION6502_H__

#include "instruction.h"
#include "types.h"
#include "token.h"

namespace lasm {
    class InstructionSet6502;

    /**
     * Immediate
     */
    class InstructionParser6502Immediate: public InstructionParser {
        public:
            InstructionParser6502Immediate(char immediate,
                    InstructionSet6502 *is);
            virtual std::shared_ptr<Stmt> parse(Parser *parser);
        private:
            char immediate;
            InstructionSet6502 *is;
    };

    class Immediate6502Generator: public InstructionGenerator {
        public:
            virtual InstructionResult generate(Interpreter *interpreter,
                    std::shared_ptr<InstructionInfo> info,
                    InstructionStmt *stmt);
    };

    /**
     * Absolute or zp modes
     */

    class InstructionParser6502AbsoluteOrZp: public InstructionParser {
        public:
            InstructionParser6502AbsoluteOrZp(InstructionSet6502 *is);
            virtual std::shared_ptr<Stmt> parse(Parser *parser);

            InstructionParser6502AbsoluteOrZp* withAbsolute(char opcode) {
                absolute = opcode;
                enableAbsolute = true;
                return this;
            }

            InstructionParser6502AbsoluteOrZp* withAbsoluteX(char opcode) {
                absoluteX = opcode;
                enableAbsoluteX = true;
                return this;
            }

            InstructionParser6502AbsoluteOrZp* withAbsoluteY(char opcode) {
                absoluteY = opcode;
                enableAbsoluteY = true;
                return this;
            }

            InstructionParser6502AbsoluteOrZp* withZeropage(char opcode) {
                zeropage = opcode;
                enableZeropage = true;
                return this;
            }

            InstructionParser6502AbsoluteOrZp* withZeropageX(char opcode) {
                zeropageX = opcode;
                enableZeropageX = true;
                return this;
            }

            InstructionParser6502AbsoluteOrZp* withZeropageY(char opcode) {
                zeropageY = opcode;
                enableZeropageY = true;
                return this;
            }

            InstructionParser6502AbsoluteOrZp* withAbsoluteLong(char opcode) {
                absoluteLong = opcode;
                enableAbsoluteLong = true;
                return this;
            }

            InstructionParser6502AbsoluteOrZp* withAbsoluteLongX(char opcode) {
                absoluteLongX = opcode;
                enableAbsoluteLongX = true;
                return this;
            }

            InstructionParser6502AbsoluteOrZp* withStackRelative(char opcode) {
                stackRelative = opcode;
                enableStackRelative = true;
                return this;
            }
        private:
            char absolute;
            char absoluteX;
            char absoluteY;
            char zeropage;
            char zeropageX;
            char zeropageY;

            char absoluteLong;
            char absoluteLongX;
            char stackRelative;

            bool enableAbsolute = false;
            bool enableAbsoluteX = false;
            bool enableAbsoluteY = false;
            bool enableZeropage = false;
            bool enableZeropageX = false;
            bool enableZeropageY = false;

            bool enableAbsoluteLong = false;
            bool enableAbsoluteLongX = false;
            bool enableStackRelative = false;

            InstructionSet6502 *is;
    };

    class AbsoluteOrZp6502Generator: public InstructionGenerator {
        public:
            virtual InstructionResult generate(Interpreter *interpreter,
                    std::shared_ptr<InstructionInfo> info,
                    InstructionStmt *stmt);
    };

    /**
     * Relative modes
     */
    class InstructionParser6502Indirect: public InstructionParser {
        public:
            InstructionParser6502Indirect(InstructionSet6502 *is);
            virtual std::shared_ptr<Stmt> parse(Parser *parser);

            InstructionParser6502Indirect* withIndirectX(char opcode) {
                indirectX = opcode;
                enableIndirectX = true;
                return this;
            }

            InstructionParser6502Indirect* withIndirectY(char opcode) {
                indirectY = opcode;
                enableIndirectY = true;
                return this;
            }

            InstructionParser6502Indirect* withIndirect(char opcode) {
                indirect = opcode;
                enableIndirect = true;
                return this;
            }

            InstructionParser6502Indirect* withIndirectZp(char opcode) {
                indirectZp = opcode;
                enableIndirectZp = true;
                return this;
            }

            InstructionParser6502Indirect* withStackY(char opcode) {
                stackY = opcode;
                enableStackY = true;
                return this;
            }
        private:
            char indirect;
            char indirectX;
            char indirectY;
            char stackY;
            char indirectZp;

            bool enableIndirect = false;
            bool enableIndirectX = false;
            bool enableIndirectY = false;
            bool enableStackY = false;
            char enableIndirectZp = false;
            InstructionSet6502 *is;
    };

    /**
     * Implicit and accumulator mode
     */
    class InstructionParser6502Implicit: public InstructionParser {
        public:
            InstructionParser6502Implicit(char opcode, InstructionSet6502 *is, bool allowAccumulator=false);
            virtual std::shared_ptr<Stmt> parse(Parser *parser);

        private:
            char opcode;
            InstructionSet6502 *is;
            bool allowAccumulator;
    };

    class Implicit6502Generator: public InstructionGenerator {
        public:
            virtual InstructionResult generate(Interpreter *interpreter,
                    std::shared_ptr<InstructionInfo> info,
                    InstructionStmt *stmt);
    };

    /**
     * Relative mode (branches)
     */
    class Relative6502Generator;

    class InstructionParser6502Relative: public InstructionParser {
        public:
            InstructionParser6502Relative(char opcode, InstructionSet6502 *is,
                    std::shared_ptr<Relative6502Generator> generator=std::shared_ptr<Relative6502Generator>(nullptr));
            virtual std::shared_ptr<Stmt> parse(Parser *parser);

        private:
            char opcode;
            InstructionSet6502 *is;
            std::shared_ptr<Relative6502Generator> generator;
    };

    class Relative6502Generator: public InstructionGenerator {
        public:
            Relative6502Generator(short bits=8):
                bits(bits) {}
            virtual InstructionResult generate(Interpreter *interpreter,
                    std::shared_ptr<InstructionInfo> info,
                    InstructionStmt *stmt);
        private:
            short bits;
    };

    /**
     * 6502
     */

    class InstructionSet6502: public BaseInstructionSet {
        public:
            InstructionSet6502(bool init=true);
            virtual void addOfficialInstructions();
            virtual InstructionResult generate(Interpreter *interpreter,
                    std::shared_ptr<InstructionInfo> info,
                    InstructionStmt *stmt);

            std::shared_ptr<Immediate6502Generator> immediate = std::make_shared<Immediate6502Generator>(Immediate6502Generator());
            std::shared_ptr<AbsoluteOrZp6502Generator> absolute = std::make_shared<AbsoluteOrZp6502Generator>(AbsoluteOrZp6502Generator());
            std::shared_ptr<Implicit6502Generator> implicit = std::make_shared<Implicit6502Generator>(Implicit6502Generator());
            std::shared_ptr<Relative6502Generator> relative = std::make_shared<Relative6502Generator>(Relative6502Generator());

        private:
            void addFullInstruction(std::string name, char immediate, char zeropage, char zeropageX,
                    char absolute, char absoluteX, char absoluteY, char indirectX, char indirectY);

            // TODO this name is bad
            // adds an instruction with zp, zpx, absolute and absolutex
            void addHalfInstruction(std::string name, char zeropage, char zeropageX,
                    char absolute, char absoluteX);
    };
}

#endif
