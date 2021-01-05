#ifndef __TOKEN_H__
#define __TOKEN_H__

#include <sstream>
#include <iostream>
#include <any>
#include "object.h"
#include "types.h"
#include <memory>

namespace lasm {
    class Token {
        public:
            Token(TokenType type, std::string lexeme, std::shared_ptr<LasmLiteral> literal, int line,
                    std::string path);

            std::string toString();

            TokenType getType() {
                return type;
            }

            std::string getLexeme() {
                return lexeme;
            }

            std::shared_ptr<LasmLiteral> getLiteral() {
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
            std::shared_ptr<LasmLiteral> literal;
            unsigned long line;
            std::string path;
    };
}

#endif
