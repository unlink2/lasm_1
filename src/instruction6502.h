#ifndef __INSTRUCTION6502_H__
#define __INSTRUCTION6502_H__

#include "instruction.h"

namespace lasm {
    /**
     * Common instruction mapper
     * good for most instructions the 6502 offers
     */
    class InstructionParser6502Generic: public InstructionParser {
        public:
            InstructionParser6502Generic(char immediate, char zeropage, char zeropageX,
                    char absolute, char absoluteX, char absoluteY, char indirectX, char indirectY);
            virtual std::shared_ptr<Stmt> parse(Parser *parse);

        private:
            char immediate;
            char zeropage;
            char zeropageX;
            char absolute;
            char absoluteX;
            char absoluteY;
            char indirectX;
            char indirectY;
    };

    class InstructionSet6502: public BaseInstructionSet {
        public:
            InstructionSet6502();
            virtual std::shared_ptr<Stmt> parse(Parser *parse);
            virtual InstructionResult generate(Interpreter *interpreter, std::shared_ptr<InstructionStmt> stmt);
    };
}

#endif
