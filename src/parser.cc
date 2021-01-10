#include "parser.h"

namespace lasm {
    Parser::Parser(BaseError &error, std::vector<std::shared_ptr<Token>> &tokens):
        tokens(tokens), onError(error) {
    }

    std::vector<std::shared_ptr<Stmt>> Parser::parse() {
        std::vector<std::shared_ptr<Stmt>> statements;

        while (!isAtEnd()) {
            statements.push_back(declaration());
        }

        return statements;
    }

    std::shared_ptr<Stmt> Parser::declaration() {
        try {
            if (match(std::vector<TokenType> {LET})) {
                return letDeclaration();
            } else if (match(std::vector<TokenType> {FUNCTION})) {
                return functionDeclaration();
            }
            return statement();
        } catch (ParserException &e) {
            sync();
            onError.onError(e.getType(), peek(), &e);
            return std::shared_ptr<Stmt>(nullptr);
        }
    }

    std::shared_ptr<Stmt> Parser::letDeclaration() {
        auto name = consume(IDENTIFIER, MISSING_IDENTIFIER);

        std::shared_ptr<Expr> init = std::shared_ptr<Expr>(nullptr);
        if (match(std::vector<TokenType> {EQUAL})) {
            init = expression();
        }

        consume(SEMICOLON, MISSING_SEMICOLON);
        return std::make_shared<LetStmt>(LetStmt(name, init));
    }

    std::shared_ptr<Stmt> Parser::functionDeclaration() {
        auto name = consume(IDENTIFIER, MISSING_IDENTIFIER);
        consume(LEFT_PAREN, MISSING_LEFT_PAREN);
        std::vector<std::shared_ptr<Token>> params;

        if (!check(RIGHT_PAREN)) {
            do {
                params.push_back(consume(IDENTIFIER, MISSING_IDENTIFIER));
            } while (match(std::vector<TokenType> {COMMA}));
        }

        consume(RIGHT_PAREN, MISSING_RIHGT_PAREN);
        consume(LEFT_BRACE, BLOCK_NOT_OPENED_ERROR);
        auto body = block();

        return std::make_shared<FunctionStmt>(name, params, body);
    }

    std::shared_ptr<Stmt> Parser::statement() {
        if (match(std::vector<TokenType> {LEFT_BRACE})) {
            return std::make_shared<BlockStmt>(block());
        } else if (match(std::vector<TokenType> {IF})) {
            return ifStatement();
        } else if (match(std::vector<TokenType> {WHILE})) {
            return whileStatement();
        } else if (match(std::vector<TokenType> {FOR})) {
            return forStatement();
        } else if (match(std::vector<TokenType> {RETURN})) {
            return returnStatement();
        }
        return expressionStatement();
    }

    std::shared_ptr<Stmt> Parser::forStatement() {
        consume(LEFT_PAREN, MISSING_LEFT_PAREN);

        std::shared_ptr<Stmt> init;
        if (match(std::vector<TokenType> {SEMICOLON})) {
            init = std::shared_ptr<Stmt>(nullptr);
        } else if (match(std::vector<TokenType> {LET})) {
            init = letDeclaration();
        } else {
            init = expressionStatement();
        }

        std::shared_ptr<Expr> condition = std::shared_ptr<Expr>(nullptr);
        if (!check(SEMICOLON)) {
            condition = expression();
        }

        consume(SEMICOLON, MISSING_SEMICOLON);

        std::shared_ptr<Expr> increment = std::shared_ptr<Expr>(nullptr);
        if (!check(RIGHT_PAREN)) {
            increment = expression();
        }
        consume(RIGHT_PAREN, MISSING_RIHGT_PAREN);

        auto body = statement();

        if (increment.get()) {
            body = std::make_shared<BlockStmt>(std::vector<std::shared_ptr<Stmt>>
                    {body, std::make_shared<ExpressionStmt>(increment)});
        }

        if (!condition.get()) {
            condition = std::make_shared<LiteralExpr>(LasmObject(BOOLEAN_O, true));
        }
        body = std::make_shared<WhileStmt>(condition, body);

        if (init.get()) {
            body = std::make_shared<BlockStmt>(std::vector<std::shared_ptr<Stmt>>
                    {init, body});
        }

        return body;
    }

    std::shared_ptr<Stmt> Parser::whileStatement() {
        consume(LEFT_PAREN, MISSING_LEFT_PAREN);
        auto condition = expression();
        consume(RIGHT_PAREN, MISSING_RIHGT_PAREN);
        auto body = statement();

        return std::make_shared<WhileStmt>(condition, body);
    }

    std::shared_ptr<Stmt> Parser::ifStatement() {
        consume(LEFT_PAREN, MISSING_LEFT_PAREN);
        auto condition = expression();
        consume(RIGHT_PAREN, MISSING_RIHGT_PAREN);

        auto thenBranch = statement();
        std::shared_ptr<Stmt> elseBranch = std::shared_ptr<Stmt>(nullptr);
        if (match(std::vector<TokenType> {ELSE})) {
            elseBranch = statement();
        }

        return std::make_shared<IfStmt>(IfStmt(condition, thenBranch, elseBranch));
    }

    std::shared_ptr<Stmt> Parser::returnStatement() {
        auto keyword = previous();
        std::shared_ptr<Expr> value = std::shared_ptr<Expr>(nullptr);

        if (!check(SEMICOLON)) {
            value = expression();
        }
        consume(SEMICOLON, MISSING_SEMICOLON);
        return std::make_shared<ReturnStmt>(ReturnStmt(keyword, value));
    }

    std::vector<std::shared_ptr<Stmt>> Parser::block() {
        std::vector<std::shared_ptr<Stmt>> statements;

        while (!check(RIGHT_BRACE) && !isAtEnd()) {
            statements.push_back(declaration());
        }

        consume(RIGHT_BRACE, BLOCK_NOT_CLOSED_ERROR);

        return statements;
    }

    std::shared_ptr<Stmt> Parser::expressionStatement() {
        auto expr = expression();
        consume(SEMICOLON, MISSING_SEMICOLON);
        return std::make_shared<ExpressionStmt>(ExpressionStmt(expr));
    }

    std::shared_ptr<Expr> Parser::expression() {
        return assignment();
    }

    std::shared_ptr<Expr> Parser::assignment() {
        auto expr = orExpr();

        if (match(std::vector<TokenType> {EQUAL})) {
            auto equals = previous();
            auto value = equality();

            if (expr->getType() == VARIABLE_EXPR) {
                auto name = std::static_pointer_cast<VariableExpr>(expr)->name;

                return std::make_shared<AssignExpr>(AssignExpr(name, value));
            }

            handleError(BAD_ASSIGNMENT, equals);
        }

        return expr;
    }

    std::shared_ptr<Expr> Parser::orExpr() {
        auto expr = andExpr();

        while (match(std::vector<TokenType> {OR})) {
            auto op = previous();
            auto right = andExpr();
            expr = std::make_shared<LogicalExpr>(LogicalExpr(expr, op, right));
        }

        return expr;
    }

    std::shared_ptr<Expr> Parser::andExpr() {
        auto expr = equality();

        while (match(std::vector<TokenType> {AND})) {
            auto op = previous();
            auto right = equality();
            expr = std::make_shared<LogicalExpr>(LogicalExpr(expr, op, right));
        }

        return expr;
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
        return call();
    }

    std::shared_ptr<Expr> Parser::call() {
        auto expr = primary();

        while (match(std::vector<TokenType> {LEFT_PAREN})) {
            std::vector<std::shared_ptr<Expr>> arguments;
            if (!check(RIGHT_PAREN)) {
                do {
                    arguments.push_back(expression());
                } while(match(std::vector<TokenType> {COMMA}));
            }
            auto paren = consume(RIGHT_PAREN, MISSING_RIHGT_PAREN);
            expr = std::make_shared<CallExpr>(expr, paren, arguments);
        }

        return expr;
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
        } else if (match(std::vector<TokenType> { IDENTIFIER })) {
            return std::make_shared<VariableExpr>(VariableExpr(previous()));
        }

        if (match(std::vector<TokenType> {LEFT_PAREN})) {
            auto expr = expression();
            consume(RIGHT_PAREN, MISSING_RIHGT_PAREN);
            return std::make_shared<GroupingExpr>(GroupingExpr(expr));
        }

        throw handleError(EXPECTED_EXPRESSION);
    }

    std::shared_ptr<Token> Parser::consume(TokenType token, ErrorType error) {
        if (check(token)) {
            return advance();
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

    ParserException Parser::handleError(ErrorType error, std::shared_ptr<Token> token) {
        if (!token.get()) {
            token = peek();
        }
        onError.onError(error, token);
        return ParserException(token, error);
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
