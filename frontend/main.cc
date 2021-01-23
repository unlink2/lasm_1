#include <iostream>
#include <memory>
#include "filereader.h"
#include "filewriter.h"
#include "argcc.h"
#include "frontend.h"
#include <fstream>
#include "instruction6502.h"
#include "token.h"
#include <filesystem>

using namespace lasm;

// TODO this is all temporary really!

class LocalFileReader: public FileReader {
    public:
        virtual std::shared_ptr<std::istream> openFile(std::string fromPath) {
            auto stream = std::make_shared<std::ifstream>(std::ifstream(fromPath, std::ifstream::in));

            if (!stream->is_open()) {
                throw LasmException(FILE_NOT_FOUND, std::make_shared<Token>(Token(NIL, "", LasmObject(NIL_O, nullptr), 0, fromPath)));
            }

            return stream;
        }

        virtual void changeDir(std::string path) {
            std::filesystem::path p(path);
            auto parent = p.native();
            if (parent == path || parent == "") {
                return;
            }
            std::filesystem::current_path(parent);
        }

        virtual std::string getDir() {
            return std::filesystem::current_path();
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

    parser.addConsumer("consumer", argcc::ARGPARSE_STRING, "Input file");
    parser.addArgument("-output", argcc::ARGPARSE_STRING, 1, "Output file", "-o");
    parser.addArgument("-symbols", argcc::ARGPARSE_STRING, 1, "Symbols file", "-s");

    auto parsed = parser.parse(argc, argv);
    std::string symbols = "";

    if (parsed.containsAny("-symbols")) {
        symbols = parsed.toString("-symbols");
    }

    std::string outfile = "a.out";
    if (parsed.containsAny("-output")) {
        outfile = parsed.toString("-output");
    }

    if (!parsed.containsAny("consumer")) {
        std::cerr << "Fatal: No input file" << std::endl;
        return -1;
    }

    auto infile = parsed.toString("consumer");


    InstructionSet6502 instructions;
    LocalFileReader reader;
    LocalFileWriter writer;
    Frontend frontend(instructions, reader, writer);

    frontend.assemble(infile, outfile, symbols);
}
