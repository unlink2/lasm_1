#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

#include <iostream>
#include <map>
#include <memory>

//TODO test
namespace lasm {
    /**
     * Base class maps instruction
     */
    class InstructionMapper {
    };

    /**
     * Base class for any arch
     */
    class BaseInstructionSet {
        public:
            std::map<std::string, std::shared_ptr<InstructionMapper>>& getInstructions() {
                return instructions;
            }
            void addInstruction(std::string name, InstructionMapper *mapper);
        protected:
            std::map<std::string, std::shared_ptr<InstructionMapper>> instructions;
    };

    class InstructionSet6502: public BaseInstructionSet {
        public:
            InstructionSet6502();
    };
}

#endif 
