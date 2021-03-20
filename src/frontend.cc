#include "frontend.h"
#include "scanner.h"
#include "parser.h"
#include "interpreter.h"
#include "instruction6502.h"
#include "instructionbf.h"
#include <string>
#include "codewriter.h"

namespace lasm {
    CpuType parseCpuType(std::string input) {
        if (input == "6502") {
            return CPU_6502; // defaults to 6502
        } else if (input == "bf") {
            return CPU_BF;
        }

        return CPU_INVALID;
    }

    std::shared_ptr<BaseInstructionSet> makeInstructionSet(CpuType type) {
        switch (type) {
            case CPU_6502:
                return std::shared_ptr<InstructionSet6502>(new InstructionSet6502());
            case CPU_BF:
                return std::shared_ptr<InstructionSetBf>(new InstructionSetBf());
            default:
                break;
        }
        // invalid set!
        throw LasmBadCpuTarget();
    }

    // TODO this is a temporary implementation
    int Frontend::assemble(std::string inPath, std::string outPath, std::string symbolPath) {
        auto previousPath = reader.getDir();

        FrontendErrorHandler error(errorOut, settings.format);
        std::shared_ptr<std::istream> is;
        try {
            is = reader.openFile(inPath);
        } catch (LasmException &e) {
            error.onError(e.getType(), 0, inPath, &e);
            return e.getType();
        }
        reader.changeDir(inPath, true);

        auto buffer = reader.readFullFile(is);
        // now we have the entire file read
        std::string source(buffer.get());
        reader.closeFile(is);

        Scanner scanner(error, instructions, source, inPath);
        auto tokens = scanner.scanTokens();

        if (error.didError()) {
            return error.getType();
        }
        Parser parser(error, tokens, instructions);
        auto ast = parser.parse();

        if (error.didError()) {
            return error.getType();
        }
        Interpreter interpreter(error, instructions, nullptr, &reader);

        auto binary = interpreter.interprete(ast, true);
        if (error.didError()) {
            return error.getType();
        }
        reader.changeDir(previousPath);

        BinaryWriter binWriter(writer, binary);
        binWriter.write(outPath);


        if (symbolPath != "") {
            SymbolsWriter symWriter(writer, interpreter, settings.hexPrefix, settings.binPrefix, settings.delim);
            symWriter.write(symbolPath);
        }

        return 0;
    }

}
