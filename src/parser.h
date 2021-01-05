#ifndef __PARSER_H__
#define __PARSER_H__

#include <iostream>
#include <vector>
#include <memory>
#include "token.h"
#include "expr.h"

namespace lasm {
    class Parser {
        public:
            Parser(std::vector<std::shared_ptr<Token>> &tokens);
        private:
            std::shared_ptr<Expr> expression();

            std::shared_ptr<Expr> equality();
            std::shared_ptr<Expr> comparison();
            std::shared_ptr<Expr> term();

            bool match(std::vector<TokenType> types);
            bool check(TokenType type);
            std::shared_ptr<Token> advance();

            bool isAtEnd();
            std::shared_ptr<Token> peek();
            std::shared_ptr<Token> previous();

            std::vector<std::shared_ptr<Token>> &tokens;
            unsigned long current = 0;
    };
}

#endif
