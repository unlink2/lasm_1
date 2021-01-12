#ifndef __FRONTEND_H__
#define __FRONTEND_H__

#include <iostream>
#include <memory>
#include "filereader.h"
#include "filewriter.h"

namespace lasm {
    class Frontend {
        public:
            Frontend(FileReader &reader, FileWriter &writer, std::ostream &errorOut=std::cerr):
                reader(reader), writer(writer), errorOut(errorOut) {}

            void assemble(std::string inPath, std::string outPath, std::string symbolPath);

        private:
            FileReader &reader;
            FileWriter &writer;
            std::ostream &errorOut;
    };
}

#endif 
