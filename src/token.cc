#include "token.h"

namespace lasm {
    Token::Token(TokenType type, std::string lexeme, LasmObject literal, int line,
            std::string path, int tokenStart, std::shared_ptr<std::string> source):
        type(type), lexeme(lexeme), literal(literal), line(line), path(path),
        tokenStart(tokenStart), source(source) {}

    std::string Token::toString() {
        std::stringstream strstream;

        strstream << type << " " << lexeme << " " << literal.toString();

        return strstream.str();
    }
}
