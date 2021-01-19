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
            InstructionParser6502AbsoluteOrZp(char absolute, char absoluteX, char absoluteY,
            char zeropage, char zeropageX, InstructionSet6502 *is);
            virtual std::shared_ptr<Stmt> parse(Parser *parser);
        private:
            char absolute;
            char absoluteX;
            char absoluteY;
            char zeropage;
            char zeropageX;
            InstructionSet6502 *is;
    };

    class AbsoluteOrZp6502Generator: public InstructionGenerator {
        public:
            virtual InstructionResult generate(Interpreter *interpreter,
                    std::shared_ptr<InstructionInfo> info,
                    InstructionStmt *stmt);
    };

    /**
     * 6502
     */

    class InstructionSet6502: public BaseInstructionSet {
        public:
            InstructionSet6502();
            virtual InstructionResult generate(Interpreter *interpreter,
                    std::shared_ptr<InstructionInfo> info,
                    InstructionStmt *stmt);

            std::shared_ptr<Immediate6502Generator> immediate = std::make_shared<Immediate6502Generator>(Immediate6502Generator());
            std::shared_ptr<AbsoluteOrZp6502Generator> absolute = std::make_shared<AbsoluteOrZp6502Generator>(AbsoluteOrZp6502Generator());
    };
}

#endif
