#ifndef __SCANNER_H__
#define __SCANNER_H__

#include <iostream>
#include <vector>
#include "token.h"
#include "object.h"
#include "error.h"
#include "common.h"
#include "instruction.h"

namespace lasm {
    class Scanner: public LasmCommon {
        public:
            Scanner(BaseError &error, BaseInstructionSet &instructions, std::string source, std::string path);

            std::vector<Token> scanTokens();

            bool isAlpha(char c);
            bool isDigit(char c);
            bool isHexDigit(char c);
            bool isAlphaNumeric(char c);
            bool isBinDigit(char c);
        private:
            bool isAtEnd();
            void scanToken();
            char advance();
            char peek();
            char peekNext();

            void addToken(TokenType type);
            void addToken(TokenType type, LasmLiteral literal);

            bool match(char expected);

            void scanString(char quote);
            void scanNumber(char c);
            void scanIdentifier();

            template<typename T>
            T stringToNumber(const std::string& number);

            void addKeyword(std::string name, TokenType type);

            std::string source;
            std::string path;
            std::vector<Token> tokens;

            unsigned long start = 0;
            unsigned long current = 0;
            unsigned long line = 1;

            std::map<std::string, TokenType> keywords;
    };
}

#endif