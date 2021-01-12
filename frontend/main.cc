#include <iostream>
#include <memory>
#include "filereader.h"
#include "filewriter.h"
#include "argcc.h"
#include "frontend.h"
#include <fstream>
#include "instruction6502.h"

using namespace lasm;

// TODO this is all temporary really!

class LocalFileReader: public FileReader {
    public:
        virtual std::shared_ptr<std::istream> openFile(std::string fromPath) {
            auto stream = std::make_shared<std::ifstream>(std::ifstream(fromPath, std::ifstream::in));

            return stream;
        }

        virtual void closeFile(std::shared_ptr<std::istream> stream) {
            std::static_pointer_cast<std::ifstream>(stream)->close();
        }
};

class LocalFileWriter: public FileWriter {
    public:
        virtual std::shared_ptr<std::ostream> openFile(std::string fromPath) {
            auto stream = std::make_shared<std::ofstream>(std::ofstream(fromPath, std::ofstream::out));

            return stream;
        }

        virtual void closeFile(std::shared_ptr<std::ostream> stream) {
            std::static_pointer_cast<std::ofstream>(stream)->close();
        }

};

int main(int argc, char **argv) {
    argcc::Argparse parser("lasm");

    parser.addArgument("-input", argcc::ARGPARSE_STRING, 1, "Input file", "-i");
    parser.addArgument("-output", argcc::ARGPARSE_STRING, 1, "Output file", "-o");

    auto parsed = parser.parse(argc, argv);

    if (parsed.containsAny("-input") && parsed.containsAny("-output")) {
        InstructionSet6502 instructions;
        LocalFileReader reader;
        LocalFileWriter writer;
        Frontend frontend(instructions, reader, writer);

        frontend.assemble(parsed.toString("-input"), parsed.toString("-output"), "");
    }
}
