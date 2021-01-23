#ifndef __PARSER_H__
#define __PARSER_H__

#include <iostream>
#include <vector>
#include <memory>
#include "token.h"
#include "expr.h"
#include "error.h"
#include "stmt.h"
#include "instruction.h"

namespace lasm {
    class Parser {
        public:
            Parser(BaseError &error, std::vector<std::shared_ptr<Token>> &tokens, BaseInstructionSet &instructions);
            std::vector<std::shared_ptr<Stmt>> parse();

            std::shared_ptr<Token> consume(TokenType token, ErrorType error, bool optional=false);

            bool match(std::vector<TokenType> types);
            bool check(TokenType type);
            std::shared_ptr<Token> advance();

            bool isAtEnd();
            std::shared_ptr<Token> peek();
            std::shared_ptr<Token> previous();

            std::shared_ptr<Expr> expression();

        private:
            std::shared_ptr<Stmt> declaration();
            std::shared_ptr<Stmt> letDeclaration();
            std::shared_ptr<Stmt> functionDeclaration();
            std::shared_ptr<Stmt> labelDeclaration();
            std::shared_ptr<Stmt> statement();

            std::shared_ptr<Stmt> forStatement();
            std::shared_ptr<Stmt> whileStatement();
            std::shared_ptr<Stmt> ifStatement();
            std::shared_ptr<Stmt> returnStatement();
            std::vector<std::shared_ptr<Stmt>> block();
            std::shared_ptr<Stmt> expressionStatement();
            std::shared_ptr<Stmt> orgDirective();
            std::shared_ptr<Stmt> fillDirective();
            std::shared_ptr<Stmt> alignDirective();

            std::shared_ptr<Stmt> defineNByteStatement(unsigned short size, Endianess endianess=LITTLE);
            std::shared_ptr<Stmt> defineByteStatement();
            std::shared_ptr<Stmt> defineHalfWorldStatement();
            std::shared_ptr<Stmt> defineWordStatement();
            std::shared_ptr<Stmt> defineDoubleWorldStatement();

            std::shared_ptr<Stmt> bssStatement();
            std::shared_ptr<Stmt> incbinStatement();
            std::shared_ptr<Stmt> includeStatement();

            std::shared_ptr<Expr> index();
            std::shared_ptr<Expr> assignment();
            std::shared_ptr<Expr> orExpr();
            std::shared_ptr<Expr> andExpr();
            std::shared_ptr<Expr> equality();
            std::shared_ptr<Expr> comparison();
            std::shared_ptr<Expr> logical();
            std::shared_ptr<Expr> term();
            std::shared_ptr<Expr> factor();
            std::shared_ptr<Expr> unary();
            std::shared_ptr<Expr> call();
            std::shared_ptr<Expr> primary();
            std::shared_ptr<Expr> list();

            ParserException handleError(ErrorType error, std::shared_ptr<Token> token=std::shared_ptr<Token>(nullptr));

            void sync();

            std::vector<std::shared_ptr<Token>> &tokens;
            unsigned long current = 0;

            BaseError &onError;
            BaseInstructionSet &instructions;
    };
}

#endif
