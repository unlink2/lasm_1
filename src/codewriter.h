#ifndef __CODDEWRITER_H__
#define __CODDEWRITER_H__

#include <iostream>
#include <vector>
#include <memory>
#include <ostream>
#include "instruction.h"
#include "filewriter.h"
#include "interpreter.h"
#include "environment.h"

namespace lasm {
    class CodeWriter {
        public:
            CodeWriter(FileWriter &writer):
                writer(writer) {
                }
            ~CodeWriter() {}

            virtual void write(std::string path) {};
        protected:
            FileWriter &writer;


    };

    class BinaryWriter: public CodeWriter {
        public:
            BinaryWriter(FileWriter &writer, std::vector<InstructionResult> binary):
                CodeWriter::CodeWriter(writer), binary(binary) {
                }

            virtual void write(std::string path);

        private:
            std::vector<InstructionResult> binary;
    };

    class SymbolsWriter: public CodeWriter {
        public:
            SymbolsWriter(FileWriter &writer, Interpreter &interpreter, std::string hexPrefix="0x", std::string binPrefix="0b"):
                CodeWriter::CodeWriter(writer), interpreter(interpreter), hexPrefix(hexPrefix) {
                }

            virtual void write(std::string path);
        private:
            void outputSymbolsEnvironment(FileWriter &writer, std::shared_ptr<std::ostream> os, std::shared_ptr<Environment> env);
            Interpreter &interpreter;
            std::string hexPrefix;
    };
}

#endif
