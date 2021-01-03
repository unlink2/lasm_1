#include "instruction.h"

namespace lasm {

    void BaseInstructionSet::addInstruction(std::string name, InstructionMapper *mapper) {
        instructions.insert(
                std::pair<std::string, std::shared_ptr<InstructionMapper>>(name,
                    std::shared_ptr<InstructionMapper>(mapper)));
    }

    InstructionSet6502::InstructionSet6502() {
        // TODO add all instructions
        addInstruction("lda", new InstructionMapper());
    }
}
