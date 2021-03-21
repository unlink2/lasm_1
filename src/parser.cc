#include "parser.h"

namespace lasm {
    Parser::Parser(BaseError &error, std::vector<std::shared_ptr<Token>> &tokens, BaseInstructionSet &instructions):
        tokens(tokens), onError(error), instructions(instructions) {
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
            } else if (match(std::vector<TokenType> {LABEL})) {
                return labelDeclaration();
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

    std::shared_ptr<Stmt> Parser::labelDeclaration() {
        auto name = previous();
        return std::make_shared<LabelStmt>(name);
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
        } else if (match(std::vector<TokenType> {INSTRUCTION})) {
            auto token = previous();
            auto instr = instructions.parse(this);
            if (!instr.get()) {
                throw ParserException(token, INVALID_INSTRUCTION);
            }
            return instr;
        } else if (match(std::vector<TokenType> { DIRECTIVE })) {
            auto token = previous();
            auto instr = instructions.parse(this);
            if (!instr.get()) {
                throw ParserException(token, INVALID_INSTRUCTION);
            }
            return instr;
        } else if (match(std::vector<TokenType> {ORG})) {
            return orgDirective();
        } else if (match(std::vector<TokenType> {FILL})) {
            return fillDirective();
        } else if (match(std::vector<TokenType> {ALIGN})) {
            return alignDirective();
        } else if (match(std::vector<TokenType> {DEFINE_BYTE})) {
            return defineByteStatement();
        } else if (match(std::vector<TokenType> {DEFINE_HALF})) {
            return defineHalfWorldStatement();
        } else if (match(std::vector<TokenType> {DEFINE_WORD})) {
            return defineWordStatement();
        } else if (match(std::vector<TokenType> {DEFINE_DOUBLE})) {
            return defineDoubleWorldStatement();
        } else if (match(std::vector<TokenType> {BSS})) {
            return bssStatement();
        } else if (match(std::vector<TokenType> {INCBIN})) {
            return incbinStatement();
        } else if (match(std::vector<TokenType> {INCLUDE})) {
            return includeStatement();
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

    std::shared_ptr<Stmt> Parser::orgDirective() {
        auto token = previous();
        std::shared_ptr<Expr> address = expression();
        consume(SEMICOLON, MISSING_SEMICOLON);
        return std::make_shared<OrgStmt>(OrgStmt(token, address));
    }

    std::shared_ptr<Stmt> Parser::fillDirective() {
        auto token = previous();
        std::shared_ptr<Expr> address = expression();
        consume(COMMA, MISSING_COMMA);
        std::shared_ptr<Expr> fillValue = expression();
        consume(SEMICOLON, MISSING_SEMICOLON);
        return std::make_shared<FillStmt>(FillStmt(token, address, fillValue));
    }

    std::shared_ptr<Stmt> Parser::alignDirective() {
        auto token = previous();
        std::shared_ptr<Expr> address = expression();
        consume(COMMA, MISSING_COMMA);
        std::shared_ptr<Expr> fillValue = expression();
        consume(SEMICOLON, MISSING_SEMICOLON);
        return std::make_shared<AlignStmt>(AlignStmt(token, address, fillValue));
    }

    std::shared_ptr<Stmt> Parser::defineNByteStatement(unsigned short size, Endianess endianess) {
        auto token = previous();
        std::vector<std::shared_ptr<Expr>> values;
        do {
            values.push_back(expression());
        } while (match(std::vector<TokenType> {COMMA}));
        consume(SEMICOLON, MISSING_SEMICOLON);

        return std::make_shared<DefineByteStmt>(DefineByteStmt(token, values, size, endianess));
    }

    std::shared_ptr<Stmt> Parser::defineByteStatement() {
        return defineNByteStatement(1, instructions.getEndianess());
    }

    std::shared_ptr<Stmt> Parser::defineHalfWorldStatement() {
        return defineNByteStatement(2, instructions.getEndianess());
    }

    std::shared_ptr<Stmt> Parser::defineWordStatement() {
        return defineNByteStatement(4, instructions.getEndianess());
    }

    std::shared_ptr<Stmt> Parser::defineDoubleWorldStatement() {
        return defineNByteStatement(8, instructions.getEndianess());
    }

    std::shared_ptr<Stmt> Parser::bssStatement() {
        auto token = previous();

        // start address of bss
        auto startAddress = expression();

        std::vector<std::shared_ptr<LetStmt>> declarations;

        consume(LEFT_BRACE, BLOCK_NOT_OPENED_ERROR);
        while (!check(RIGHT_BRACE) && !isAtEnd()) {
            // name size,
            auto name = consume(IDENTIFIER, MISSING_IDENTIFIER);
            auto size = expression();
            // add to list
            declarations.push_back(std::make_shared<LetStmt>(LetStmt(name, size)));

            if (!check(RIGHT_BRACE)) {
                consume(COMMA, MISSING_COMMA);
            }
        }
        consume(RIGHT_BRACE, BLOCK_NOT_CLOSED_ERROR);
        return std::make_shared<BssStmt>(BssStmt(token, startAddress, declarations));
    }

    std::shared_ptr<Stmt> Parser::incbinStatement() {
        auto token = previous();
        auto filePath = expression();
        return std::make_shared<IncbinStmt>(IncbinStmt(token, filePath));
    }

    std::shared_ptr<Stmt> Parser::includeStatement() {
        auto token = previous();
        auto filePath = expression();
        return std::make_shared<IncludeStmt>(IncludeStmt(token, filePath));
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
        return index();
    }

    std::shared_ptr<Expr> Parser::index() {
        auto expr = assignment();

        while (match(std::vector<TokenType> {LEFT_BRACKET})) {
            auto token = previous();
            // index expr found!
            auto index = expression();
            consume(RIGHT_BRACKET, BLOCK_NOT_CLOSED_ERROR);
            if (match(std::vector<TokenType> {EQUAL})) {
                // either assing to an indexed value
                auto value = equality();
                expr = std::make_shared<IndexAssignExpr>(IndexAssignExpr(expr, index, value, token));
            } else {
                // or simply return it
                expr = std::make_shared<IndexExpr>(IndexExpr(expr, index, token));
            }
        }

        return expr;
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
        auto expr = logical();

        while (match(std::vector<TokenType> {GREATER, GREATER_EQUAL, LESS, LESS_EQUAL})) {
            auto op = previous();
            auto right = logical();
            expr = std::make_shared<BinaryExpr>(BinaryExpr(expr, op, right));
        }

        return expr;
    }

    std::shared_ptr<Expr> Parser::logical() {
        auto expr = term();

        while (match(std::vector<TokenType> {BIN_AND, BIN_OR, BIN_XOR, BIN_SHIFT_LEFT, BIN_SHIFT_RIGHT})) {
            auto op = previous();
            auto right = factor();
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

        while (match(std::vector<TokenType> {SLASH, STAR, PERCENT})) {
            auto op = previous();
            auto right = unary();
            expr = std::make_shared<BinaryExpr>(BinaryExpr(expr, op, right));
        }

        return expr;
    }

    std::shared_ptr<Expr> Parser::unary() {
        if (match(std::vector<TokenType> {BANG, MINUS, BIN_NOT})) {
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
        } else if (match(std::vector<TokenType> {LEFT_BRACKET})) {
            return list();
        }
        throw handleError(EXPECTED_EXPRESSION);
    }

    std::shared_ptr<Expr> Parser::list() {
        auto paren = previous();
        std::vector<std::shared_ptr<Expr>> inits;
        while (!check(RIGHT_BRACKET) && !isAtEnd()) {
            inits.push_back(expression());

            if (!check(RIGHT_BRACKET)) {
                consume(COMMA, MISSING_COMMA);
            }
        }
        consume(RIGHT_BRACKET, BLOCK_NOT_CLOSED_ERROR);
        return std::make_shared<ListExpr>(ListExpr(inits, paren));
    }

    std::shared_ptr<Token> Parser::consume(TokenType token, ErrorType error, bool optional) {
        if (check(token)) {
            return advance();
        }

        if (optional) {
            return std::shared_ptr<Token>(nullptr);
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
