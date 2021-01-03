#include "token.h"

namespace lasm {
    Token::Token(TokenType type, std::string lexeme, LasmLiteral literal, int line,
            std::string path):
        type(type), lexeme(lexeme), literal(literal), line(line), path(path) {}

    std::string Token::toString() {
        std::stringstream strstream;

        strstream << type << " " << lexeme << " " << literal.toString();

        return strstream.str();
    }
}
