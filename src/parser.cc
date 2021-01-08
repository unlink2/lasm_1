#include "parser.h"

namespace lasm {
    Parser::Parser(BaseError &error, std::vector<std::shared_ptr<Token>> &tokens):
        tokens(tokens), onError(error) {
    }

    std::vector<std::shared_ptr<Stmt>> Parser::parse() {
        std::vector<std::shared_ptr<Stmt>> statements;

        while (!isAtEnd()) {
            statements.push_back(statement());
        }

        return statements;
    }

    std::shared_ptr<Stmt> Parser::statement() {
        return expressionStatement();
    }

    std::shared_ptr<Stmt> Parser::expressionStatement() {
        auto expr = expression();
        consume(SEMICOLON, MISSING_SEMICOLON);
        return std::make_shared<ExpressionStmt>(ExpressionStmt(expr));
    }

    std::shared_ptr<Expr> Parser::expression() {
        return equality();
    }

    std::shared_ptr<Expr> Parser::equality() {
        std::shared_ptr<Expr> expr = comparison();

        while (match(std::vector<TokenType> {BANG_EQUAL, EQUAL_EQUAL})) {
            auto op = previous();
            auto right = comparison();
            expr = std::make_shared<BinaryExpr>(BinaryExpr(expr, op, right));
        }

        return expr;
    }

    std::shared_ptr<Expr> Parser::comparison() {
        auto expr = term();

        while (match(std::vector<TokenType> {GREATER, GREATER_EQUAL, LESS, LESS_EQUAL})) {
            auto op = previous();
            auto right = term();
            expr = std::make_shared<BinaryExpr>(BinaryExpr(expr, op, right));
        }

        return expr;
    }

    std::shared_ptr<Expr> Parser::term() {
        auto expr = factor();

        while (match(std::vector<TokenType> {MINUS, PLUS})) {
            auto op = previous();
            auto right = factor();
            expr = std::make_shared<BinaryExpr>(BinaryExpr(expr, op, right));
        }

        return expr;
    }

    std::shared_ptr<Expr> Parser::factor() {
        auto expr = unary();

        while (match(std::vector<TokenType> {SLASH, STAR})) {
            auto op = previous();
            auto right = unary();
            expr = std::make_shared<BinaryExpr>(BinaryExpr(expr, op, right));
        }

        return expr;
    }

    std::shared_ptr<Expr> Parser::unary() {
        if (match(std::vector<TokenType> {BANG, MINUS})) {
            auto op = previous();
            auto right = unary();
            return std::make_shared<UnaryExpr>(UnaryExpr(op, right));
        }
        return primary();
    }

    std::shared_ptr<Expr> Parser::primary() {
        if (match(std::vector<TokenType> {FALSE})) {
            return std::make_shared<LiteralExpr>(LiteralExpr(LasmObject(BOOLEAN_O, false)));
        } else if (match(std::vector<TokenType> {TRUE})) {
            return std::make_shared<LiteralExpr>(LiteralExpr(LasmObject(BOOLEAN_O, true)));
        } else if (match(std::vector<TokenType> {NIL})) {
            return std::make_shared<LiteralExpr>(LiteralExpr(LasmObject(NIL_O, nullptr)));
        } else if (match(std::vector<TokenType> {NUMBER, REAL, STRING})) {
            return std::make_shared<LiteralExpr>(LiteralExpr(previous()->getLiteral()));
        }

        if (match(std::vector<TokenType> {LEFT_PAREN})) {
            auto expr = expression();
            consume(RIGHT_PAREN, MISSING_RIHGT_PAREN);
            return std::make_shared<GroupingExpr>(GroupingExpr(expr));
        }

        throw handleError(EXPECTED_EXPRESSION);
    }

    void Parser::consume(TokenType token, ErrorType error) {
        if (check(token)) {
            advance();
            return;
        }

        throw handleError(error);
    }

    bool Parser::match(std::vector<TokenType> types) {
        for (auto it = types.begin(); it != types.end(); it++) {
            if (check(*it)) {
                advance();
                return true;
            }
        }
        return false;
    }

    bool Parser::check(TokenType type) {
        return !isAtEnd() && peek()->getType() == type;
    }

    std::shared_ptr<Token> Parser::advance() {
        if (!isAtEnd()) {
            current++;
        }

        return previous();
    }

    ParserException Parser::handleError(ErrorType error) {
        onError.onError(error, peek());
        return ParserException(peek(), error);
    }

    bool Parser::isAtEnd() {
        return peek()->getType() == EOF_T;
    }

    std::shared_ptr<Token> Parser::peek() {
        return tokens.at(current);
    }

    std::shared_ptr<Token> Parser::previous() {
        return tokens.at(current-1);
    }

    void Parser::sync() {
        advance();

        while (!isAtEnd()) {
            if (previous()->getType() == SEMICOLON) {
                return;
            }

            switch (peek()->getType()) {
                case FUNCTION:
                case LET:
                case FOR:
                case IF:
                case WHILE:
                case RETURN:
                    return;
                default:
                    break;
            }

            advance();
        }
    }
}
