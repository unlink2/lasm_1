#include "instruction.h"
#include "parser.h"
#include "stmt.h"

namespace lasm {
    void BaseInstructionSet::addInstruction(std::string name, InstructionParser *parser) {
        instructions.insert(
                std::pair<std::string, std::shared_ptr<InstructionParser>>(name,
                    std::shared_ptr<InstructionParser>(parser)));
    }
}
