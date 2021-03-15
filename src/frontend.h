#ifndef __FRONTEND_H__
#define __FRONTEND_H__

#include <iostream>
#include <memory>
#include "filereader.h"
#include "filewriter.h"
#include "instruction.h"
#include "error.h"
#include "environment.h"
#include "colors.h"

namespace lasm {
    enum CpuType {
        CPU_6502,
        CPU_BF,
        CPU_INVALID
    };

    CpuType parseCpuType(std::string input);

    // cpu instruction set factory
    // throws LasmBadCpuTarget
    std::shared_ptr<BaseInstructionSet> makeInstructionSet(CpuType type);

    class FrontendErrorHandler: public BaseError {
        public:
            FrontendErrorHandler(std::ostream &errorOut, const FormatOutput &format):
                errorOut(errorOut), format(format) {}
            virtual void onError(ErrorType type, unsigned long line, std::string path, LasmException *e=nullptr) {
                hasErrored = true;
                this->type = type;
                this->path = path;
                this->line = line;

                errorOut << format.fred() << errorToString(type) << format.reset()
                    << " in " << path << ":" << line << std::endl;
            }

            virtual void onError(ErrorType type, std::shared_ptr<Token> token, LasmException *e=nullptr) {
                hasErrored = true;
                this->type = type;
                this->path = token->getPath();
                this->line = token->getLine();
                if (e && e->getType() == CALLSTACK_UNWIND) {
                    auto cs = (CallStackUnwind*)e;
                    if (cs->getParent()) {
                        onError(cs->getParent()->getType(), cs->getParent()->getToken(), cs->getParent());
                    }
                }
                errorOut << format.fred() << errorToString(e ? e->getType() : type) << format.reset()
                    << " (" << token->getLexeme() << ")" << " in " <<
                    token->getPath() << ":" << token->getLine() << std::endl;
            }

        private:
            std::ostream &errorOut;
            const FormatOutput &format;
    };

    class Frontend {
        public:
            Frontend(BaseInstructionSet &instructions,
                    FileReader &reader, FileWriter &writer, std::ostream &errorOut=std::cerr,
                    std::string hexPrefix="0x", std::string binPrefix="0b",
                    const FormatOutput &format=FormatOutput(false)):
                instructions(instructions), reader(reader), writer(writer), errorOut(errorOut),
                hexPrefix(hexPrefix), binPrefix(binPrefix), format(format) {}

            int assemble(std::string inPath, std::string outPath, std::string symbolPath="");

        private:

            BaseInstructionSet &instructions;
            FileReader &reader;
            FileWriter &writer;
            std::ostream &errorOut;
            std::string hexPrefix;
            std::string binPrefix;
            const FormatOutput &format;
    };
}

#endif
