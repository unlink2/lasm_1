#ifndef __INSTRUCTION6502_H__
#define __INSTRUCTION6502_H__

#include "instruction.h"
#include "types.h"
#include "token.h"

namespace lasm {
    class InstructionSet6502;
    /**
     * Common instruction mapper
     * good for most instructions the 6502 offers
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

    class InstructionSet6502: public BaseInstructionSet {
        public:
            InstructionSet6502();
            virtual InstructionResult generate(Interpreter *interpreter,
                    std::shared_ptr<InstructionInfo> info,
                    InstructionStmt *stmt);

            std::shared_ptr<Immediate6502Generator> immediate = std::make_shared<Immediate6502Generator>(Immediate6502Generator());
    };
}

#endif
