#ifndef __INSTRUCTIONBF_H__
#define __INSTRUCTIONBF_H__

#include "instruction.h"
#include "types.h"
#include "token.h"


namespace lasm {
    class InstructionSetBf;

    /**
     * Implicit
     */
    class InstructionParserBfImplicit: public InstructionParser {
        public:
            InstructionParserBfImplicit(char opcode,
                    InstructionSetBf *is);
            virtual std::shared_ptr<Stmt> parse(Parser *parser);
        private:
            char opcode;
            InstructionSetBf *is;
    };

    class ImplicitBfGenerator: public InstructionGenerator {
        public:
            virtual InstructionResult generate(Interpreter *interpreter,
                    std::shared_ptr<InstructionInfo> info,
                    InstructionStmt *stmt);
    };

    class InstructionSetBf: public BaseInstructionSet {
        public:
            InstructionSetBf();
            virtual InstructionResult generate(Interpreter *interpreter,
                    std::shared_ptr<InstructionInfo> info,
                    InstructionStmt *stmt);

            std::shared_ptr<ImplicitBfGenerator> implicit = std::make_shared<ImplicitBfGenerator>(ImplicitBfGenerator());
    };
}

#endif
