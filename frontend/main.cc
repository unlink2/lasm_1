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
#include "colors.h"

// TODO cross-platform?
#include <unistd.h>

using namespace lasm;

// TODO this is all temporary really!

class LocalFileReader: public FileReader {
    public:
        virtual std::shared_ptr<std::istream> openFile(std::string fromPath) {
            auto stream = std::make_shared<std::ifstream>(std::ifstream(fromPath, std::ifstream::in));

            if (!stream->is_open()) {
                auto source = std::make_shared<std::string>(std::string(""));
                throw LasmException(FILE_NOT_FOUND, std::make_shared<Token>(Token(NIL, "", LasmObject(NIL_O, nullptr), 0, fromPath, 0, source)));
            }

            return stream;
        }

        virtual void changeDir(std::string path, bool hasFilename) {
            std::filesystem::path p(path);
            if (hasFilename) {
                p.remove_filename();
            }
            auto parent = p.native();
            if (parent == "") {
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

    const FormatOutput format(isatty(STDERR_FILENO) && isatty(STDOUT_FILENO));

    parser.addConsumer("consumer", argcc::ARGPARSE_STRING, "Input file");
    parser.addArgument("-output", argcc::ARGPARSE_STRING, 1, "Output file", "-o");
    parser.addArgument("-symbols", argcc::ARGPARSE_STRING, 1, "Symbols file", "-s");
    parser.addArgument("-hprefix", argcc::ARGPARSE_STRING, 1, "Hex-prefix for symbols file", "-hp");
    parser.addArgument("-bprefix", argcc::ARGPARSE_STRING, 1, "Binary-prefix for symbols file", "-bp");
    parser.addArgument("-cpu", argcc::ARGPARSE_STRING, 1, "CPU type (valid options: 6502, bf)", "-c");

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
        std::cerr << format.fred() << "Fatal: " << format.reset() << "No input file" << std::endl;
        return -1;
    }

    std::string cpuString = "6502";

    if (parsed.containsAny("-cpu")) {
        cpuString = parsed.toString("-cpu");
    }
    auto infile = parsed.toString("consumer");

    std::string hexPrefix = "0x";
    if (parsed.containsAny("-hprefix")) {
        hexPrefix = parsed.toString("-hprefix");
    }

    std::string binPrefix = "0b";
    if (parsed.containsAny("-bprefix")) {
        binPrefix = parsed.toString("-bprefix");
    }

    std::shared_ptr<BaseInstructionSet> instructions;
    try {
        instructions = makeInstructionSet(parseCpuType(cpuString));
    } catch (LasmBadCpuTarget &e) {
        std::cerr <<  format.fred() << "Fatal: " << format.reset() << "Unknown instruction set" << std::endl;
        return -1;
    }

    LocalFileReader reader;
    LocalFileWriter writer;
    Frontend frontend(*instructions.get(), reader, writer, std::cerr, hexPrefix, binPrefix, format);

    return frontend.assemble(infile, outfile, symbols);
}
