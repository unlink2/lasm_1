#ifndef __PARSER_H__
#define __PARSER_H__

#include <iostream>
#include <vector>
#include <memory>
#include "token.h"
#include "expr.h"
#include "error.h"
#include "stmt.h"

namespace lasm {
    class Parser {
        public:
            Parser(BaseError &error, std::vector<std::shared_ptr<Token>> &tokens);
            std::vector<std::shared_ptr<Stmt>> parse();
        private:
            std::shared_ptr<Stmt> declaration();
            std::shared_ptr<Stmt> letDeclaration();
            std::shared_ptr<Stmt> statement();

            std::shared_ptr<Stmt> expressionStatement();
            
            std::shared_ptr<Expr> expression();

            std::shared_ptr<Expr> equality();
            std::shared_ptr<Expr> comparison();
            std::shared_ptr<Expr> term();
            std::shared_ptr<Expr> factor();
            std::shared_ptr<Expr> unary();
            std::shared_ptr<Expr> primary();

            std::shared_ptr<Token> consume(TokenType token, ErrorType error);

            bool match(std::vector<TokenType> types);
            bool check(TokenType type);
            std::shared_ptr<Token> advance();

            bool isAtEnd();
            std::shared_ptr<Token> peek();
            std::shared_ptr<Token> previous();

            ParserException handleError(ErrorType error);

            void sync();

            std::vector<std::shared_ptr<Token>> &tokens;
            unsigned long current = 0;

            BaseError &onError;
    };
}

#endif
