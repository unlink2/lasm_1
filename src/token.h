#ifndef __TOKEN_H__
#define __TOKEN_H__

#include <sstream>
#include <iostream>
#include <any>
#include "object.h"
#include "types.h"

// TODO test

namespace lasm {
    class Token {
        public:
            Token(TokenType type, std::string lexeme, LasmLiteral literal, int line,
                    std::string path);

            std::string toString();

            TokenType getType() {
                return type;
            }

            std::string getLexeme() {
                return lexeme;
            }

            LasmLiteral getLiteral() {
                return literal;
            }

            unsigned long getLine() {
                return line;
            }

            std::string getPath() {
                return path;
            }

        private:
            TokenType type;
            std::string lexeme;
            LasmLiteral literal;
            unsigned long line;
            std::string path;
    };
}

#endif
