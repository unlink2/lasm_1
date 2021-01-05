#include "parser.h"

namespace lasm {
    Parser::Parser(std::vector<std::shared_ptr<Token>> &tokens):
        tokens(tokens) {
    }

    std::shared_ptr<Expr> Parser::expression() {
        return equality();
    }

    std::shared_ptr<Expr> Parser::equality() {
        std::shared_ptr<Expr> expr = comparison();

        while (match(std::vector<TokenType> {BANG_EQUAL, EQUAL})) {
            auto op = previous();
            auto right = comparison();
            expr = std::make_shared<Expr>(BinaryExpr(expr, op, right));
        }

        return expr;
    }

    std::shared_ptr<Expr> Parser::comparison() {
        auto expr = term();

        while (match(std::vector<TokenType> {GREATER, GREATER_EQUAL, LESS, LESS_EQUAL})) {
            auto op = previous();
            auto right = term();
            expr = std::make_shared<Expr>(BinaryExpr(expr, op, right));
        }

        return expr;
    }

    std::shared_ptr<Expr> Parser::term() {
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

    bool Parser::isAtEnd() {
        return peek()->getType() == EOF_T;
    }

    std::shared_ptr<Token> Parser::peek() {
        return tokens.at(current);
    }

    std::shared_ptr<Token> Parser::previous() {
        return tokens.at(current);
    }
}
