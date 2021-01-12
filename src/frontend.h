#ifndef __FRONTEND_H__
#define __FRONTEND_H__

#include <iostream>
#include <memory>
#include "filereader.h"
#include "filewriter.h"
#include "instruction.h"
#include "error.h"

namespace lasm {
    class FrontendErrorHandler: public BaseError {
    };

    class Frontend {
        public:
            Frontend(BaseInstructionSet &instructions,
                    FileReader &reader, FileWriter &writer, std::ostream &errorOut=std::cerr):
                instructions(instructions), reader(reader), writer(writer), errorOut(errorOut) {}

            void assemble(std::string inPath, std::string outPath, std::string symbolPath);

        private:
            BaseInstructionSet &instructions;
            FileReader &reader;
            FileWriter &writer;
            std::ostream &errorOut;
    };
}

#endif 
