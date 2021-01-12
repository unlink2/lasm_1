#include "frontend.h"
#include "scanner.h"
#include "parser.h"
#include "interpreter.h"
#include <string>

namespace lasm {
    // TODO this is a temporary implementation
    void Frontend::assemble(std::string inPath, std::string outPath, std::string symbolPath) {
        auto is = reader.openFile(inPath);

        // get length of file:
        is->seekg (0, is->end);
        int length = is->tellg();
        is->seekg (0, is->beg);

        auto buffer = std::shared_ptr<char[]>(new char[length]);
        is->read(buffer.get(), length);

        // now we have the entire file read
        std::string source(buffer.get());
        reader.closeFile(is);

        FrontendErrorHandler error;
        Scanner scanner(error, instructions, source, inPath);
        auto tokens = scanner.scanTokens();

        Parser parser(error, tokens, instructions);
        auto ast = parser.parse();

        Interpreter interpreter(error, instructions);
        auto binary = interpreter.interprete(ast);

        auto os = writer.openFile(outPath);
        for (auto b : binary) {
            os->write(b.getData().get(), b.getSize());
        }
        writer.closeFile(os);
    }
}
