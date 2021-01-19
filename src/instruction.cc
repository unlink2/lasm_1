#include "instruction.h"
#include "parser.h"
#include "stmt.h"

namespace lasm {
    void BaseInstructionSet::addInstruction(std::string name, std::shared_ptr<InstructionParser> parser) {
        auto it = instructions.find(name);
        if (it != instructions.end()) {
            it->second.push_back(parser);
        } else {
            instructions.insert(
                std::pair<std::string, std::vector<std::shared_ptr<InstructionParser>>>(name,
                    std::vector<std::shared_ptr<InstructionParser>> {parser}));
        }
    }

    std::shared_ptr<Stmt> BaseInstructionSet::parse(Parser *parser) {
        auto name = parser->previous()->getLexeme();
        auto it = instructions.find(name);
        if (it != instructions.end()) {
            auto instParsers = it->second;
            for (auto instParser : instParsers) {
                auto result = instParser->parse(parser);
                if (result.get() != nullptr) {
                    return result;
                }
            }
        }

        return std::shared_ptr<Stmt>(nullptr);
    }
}
