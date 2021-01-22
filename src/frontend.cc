#include "frontend.h"
#include "scanner.h"
#include "parser.h"
#include "interpreter.h"
#include <string>

namespace lasm {
    // TODO this is a temporary implementation
    void Frontend::assemble(std::string inPath, std::string outPath, std::string symbolPath) {
        auto is = reader.openFile(inPath);

        auto buffer = reader.readFullFile(is);
        // now we have the entire file read
        std::string source(buffer.get());
        reader.closeFile(is);

        FrontendErrorHandler error(errorOut);
        Scanner scanner(error, instructions, source, inPath);
        auto tokens = scanner.scanTokens();

        if (error.didError()) {
            return;
        }
        Parser parser(error, tokens, instructions);
        auto ast = parser.parse();

        if (error.didError()) {
            return;
        }
        Interpreter interpreter(error, instructions);

        if (error.didError()) {
            return;
        }
        auto binary = interpreter.interprete(ast, true);

        auto os = writer.openFile(outPath);
        for (auto b : binary) {
            os->write(b.getData().get(), b.getSize());
        }
        writer.closeFile(os);

        if (symbolPath == "") {
            return; // no symbol file!
        }

        // output symbols to file
        // TODO also dump a mapping of code to addresses
        auto sos = writer.openFile(symbolPath);

        for (auto env : interpreter.getLabelTable()) {
            outputSymbolsEnviorment(writer, sos, env);
        }
        outputSymbolsEnviorment(writer, sos, interpreter.getGlobals());
        writer.closeFile(sos);

    }

    void Frontend::outputSymbolsEnviorment(FileWriter &writer,
            std::shared_ptr<std::ostream> os, std::shared_ptr<Enviorment> env) {
        // dump strings, numbers and floats only
        // in format <name> = <value>
        std::ostream &stream = *(os.get());

        auto values = env->getValues();
        for (auto it = values.begin(); it != values.end(); it++) {
            auto obj = it->second;
            std::string name = it->first;

            switch (obj->getType()) {
                case STRING_O:
                    stream << name << " = " << obj->toString() << std::endl;
                    break;
                case NUMBER_O:
                    stream << name << " = 0x" << std::hex << obj->toNumber() << std::endl;
                    break;
                case REAL_O:
                    stream << name << " = " << obj->toReal() << std::endl;
                    break;
                default:
                    // skip 
                    break;
            }
        }
    }
}
