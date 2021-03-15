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
            Token(TokenType type, std::string lexeme, LasmObject literal, int line,
                    std::string path, int tokenStart, std::shared_ptr<std::string> source);

            std::string toString();

            TokenType getType() {
                return type;
            }

            std::string getLexeme() {
                return lexeme;
            }

            LasmObject getLiteral() {
                return literal;
            }

            unsigned long getLine() {
                return line;
            }

            std::string getPath() {
                return path;
            }

            std::shared_ptr<std::string> getSource() {
                return source;
            }

            unsigned long getTokenStart() {
                return tokenStart;
            }

        private:
            TokenType type;
            std::string lexeme;
            LasmObject literal;
            unsigned long line;
            std::string path;
            unsigned long tokenStart;
            std::shared_ptr<std::string> source;
    };
}

#endif
