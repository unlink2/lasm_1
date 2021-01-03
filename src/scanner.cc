#include "scanner.h"

namespace lasm {
    Scanner::Scanner(BaseError &error, BaseInstructionSet &instructions, std::string source, std::string path):
        LasmCommon(error, instructions), source(source), path(path) {
            addKeyword("and", AND);
            addKeyword("else", ELSE);
            addKeyword("false", FALSE);
            addKeyword("for", FOR);
            addKeyword("func", FUNCTION);
            addKeyword("if", IF);
            addKeyword("nil", NIL);
            addKeyword("or", OR);
            addKeyword("true", TRUE);
            addKeyword("let", LET);
            addKeyword("while", WHILE);
    }

    std::vector<Token> Scanner::scanTokens() {
        while (!isAtEnd()) {
            start = current;
            scanToken();
        }
        tokens.push_back(Token(EOF_T, "", LasmLiteral(NIL, nullptr), line, path));

        return tokens;
    }

    bool Scanner::isAtEnd() {
        return current >= source.size();
    }

    void Scanner::scanToken() {
        char c = advance();
        switch (c) {
            case '(':
                addToken(LEFT_PAREN);
                break;
            case ')':
                addToken(RIGHT_PAREN);
                break;
            case '{':
                addToken(LEFT_BRACE);
                break;
            case '}':
                addToken(RIGHT_BRACE);
                break;
            case '[':
                addToken(LEFT_BRACKET);
                break;
            case ']':
                addToken(RIGHT_BRACKET);
                break;
            case ',':
                addToken(COMMA);
                break;
            case '.':
                addToken(DOT);
                break;
            case '-':
                addToken(MINUS);
                break;
            case '+':
                addToken(PLUS);
                break;
            case ';':
                addToken(SEMICOLON);
                break;
            case '*':
                addToken(STAR);
                break;
            case '!':
                addToken(match('=') ? BANG_EQUAL : BANG);
                break;
            case '=':
                addToken(match('=') ? EQUAL_EQUAL : EQUAL);
                break;
            case '<':
                addToken(match('=') ? LESS_EQUAL : LESS);
                break;
            case '>':
                addToken(match('=') ? GREATER_EQUAL : GREATER);
                break;
            case '/':
                if (match('/')) {
                    // until end of line
                    while (peek() != '\n' && !isAtEnd()) {
                        advance();
                    }
                } else {
                    addToken(SLASH);
                }
                break;
            case '#':
                addToken(HASH);
                break;
            // ignored characters
            case ' ':
            case '\t':
            case '\r':
                break;

            // new line
            case '\n':
                line++;
                break;

            case '"':
            case '\'':
                scanString(c);
                break;

            default:
                if (isDigit(c)) {
                    scanNumber(c);
                } else if (isAlpha(c)) {
                } else {
                    error.onError(UNEXPECTED_CHAR, line, path);
                }
                break;
        }
    }

    bool Scanner::isDigit(char c) {
        return c >= '0' && c <= '9';
    }

    bool Scanner::isHexDigit(char c) {
        return (c >= 'a' && c <= 'f') || (c >= 'A' && c<= 'F') || isDigit(c);
    }

    bool Scanner::isBinDigit(char c) {
        return c == '0' || c == '1';
    }

    bool Scanner::isAlpha(char c) {
        return (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            c == '_';
    }

    bool Scanner::isAlphaNumeric(char c) {
        return isAlpha(c) || isDigit(c);
    }


    char Scanner::advance() {
        current++;
        return source.at(current-1);
    }

    char Scanner::peek() {
        if (isAtEnd()) {
            return '\0';
        }
        return source.at(current);
    }

    char Scanner::peekNext() {
        if (current+1 >= source.size()) {
            return '\0';
        }
        return source.at(current+1);
    }

    void Scanner::addToken(TokenType type) {
        addToken(type, LasmLiteral(NIL, nullptr));
    }

    void Scanner::addToken(TokenType type, LasmLiteral literal) {
        std::string text = source.substr(start, current-start);
        tokens.push_back(Token(type, text, literal, line, path));
    }

    bool Scanner::match(char expected) {
        if (isAtEnd()
                || source.at(current) != expected) {
            return false;
        }
        current++;
        return true;
    }

    void Scanner::scanString(char quote) {
        while (peek() != quote && !isAtEnd()) {
            if (peek() == '\n') {
                line++;
            }

            // escape character
            if (peek() == '\\') {
                advance();
            }
            advance();
        }

        if (isAtEnd()) {
            error.onError(UNTERMINATED_STRING, line, path);
            return;
        }

        // closing "
        advance();

        std::string value = source.substr(start + 1, current - 1);

        // TODO unescape string
        addToken(STRING, LasmLiteral(STRING, value));
    }

    void Scanner::scanNumber(char c) {
        bool isFloat = false;
        bool isHex = c == '0' && peek() == 'x';
        bool isBin = c == '0' && peek() == 'b';
        TokenType type = NUMBER;

        if (isHex) {
            advance();
            while (isHexDigit(peek())) {
                advance();
            }
        } else if (isBin) {
            advance();
            while (isBinDigit(peek())) {
                advance();
            }
        } else {
            // decimal
            while (isDigit(peek())) {
                advance();
            }

            // is float?
            if (peek() == '.' && isDigit(peekNext())) {
                advance();
                isFloat = true;
                type = REAL;
                while (isDigit(peek())) {
                    advance();
                }
            }
        }

        try {
            std::any value;
            auto number = source.substr(start, current);
            if (isFloat) {
                value = std::any(stringToNumber<double>(number));
            } else {
                value = std::any(stringToNumber<long>(number));
            }
            addToken(type, LasmLiteral(type, value));
        } catch (...) {
            error.onError(NUMBER_PARSE_ERROR, line, path);
        }
    }

    void Scanner::scanIdentifier() {
        while (isAlphaNumeric(peek())) {
            advance();
        }

        std::string text = source.substr(start, current);
        // is it an opcode from the opcode table?
        auto opIt = instructions.getInstructions().find(text);
        auto keywordIt = keywords.find(text);

        TokenType type = IDENTIFIER;
        if (opIt != instructions.getInstructions().end()) {
            type = INSTRUCTION;
        } else if (keywordIt != keywords.end()) {
            type = keywordIt->second;
        }

        addToken(type);
    }

    template<typename T>
    T Scanner::stringToNumber(const std::string& number) {
        T value;

        std::stringstream stream(number);
        stream >> value;
        if (stream.fail()) {
            std::runtime_error e(number);
            throw e;
        }
        return value;
    }

    void Scanner::addKeyword(std::string name, TokenType type) {
        keywords.insert(std::pair<std::string, TokenType>(name, type));
    }
}