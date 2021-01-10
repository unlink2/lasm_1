#include "scanner.h"
#include "utility.h"

namespace lasm {
    Scanner::Scanner(BaseError &error, BaseInstructionSet &instructions, std::string source, std::string path):
        LasmCommon(error, instructions), source(source), path(path) {
            addKeyword("and", AND);
            addKeyword("else", ELSE);
            addKeyword("false", FALSE);
            addKeyword("for", FOR);
            addKeyword("fn", FUNCTION);
            addKeyword("if", IF);
            addKeyword("nil", NIL);
            addKeyword("or", OR);
            addKeyword("true", TRUE);
            addKeyword("let", LET);
            addKeyword("while", WHILE);
            addKeyword("return", RETURN);
    }

    std::vector<std::shared_ptr<Token>> Scanner::scanTokens() {
        while (!isAtEnd()) {
            start = current;
            scanToken();
        }
        tokens.push_back(std::make_shared<Token>(Token(EOF_T, "",
                        LasmObject(NIL_O, nullptr), line, path)));

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
                    scanIdentifier();
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
        addToken(type, LasmObject(NIL_O, nullptr));
    }

    void Scanner::addToken(TokenType type, LasmObject literal) {
        std::string text = source.substr(start, current-start);
        tokens.push_back(std::make_shared<Token>(Token(type, text, literal, line, path)));
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

        std::string value = unescape(source.substr(start+1, current-start-2));
        addToken(STRING, LasmObject(STRING_O, value));
    }

    void Scanner::scanNumber(char c) {
        bool isFloat = false;
        bool isHex = c == '0' && peek() == 'x';
        bool isBin = c == '0' && peek() == 'b';
        TokenType type = NUMBER;
        ObjectType objType = NUMBER_O;

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
                objType = REAL_O;
                while (isDigit(peek())) {
                    advance();
                }
            }
        }

        try {
            std::any value;
            if (isFloat) {
                auto number = source.substr(start, current-start);
                value = std::any(stringToReal(number));
            } else if (isBin) {
                auto number = source.substr(start+2, current-start);
                value = std::any(stringToNumber(number, 2));
            } else if (isHex) {
                auto number = source.substr(start, current-start);
                value = std::any(stringToNumber(number, 16));
            } else {
                auto number = source.substr(start, current-start);
                value = std::any(stringToNumber(number));
            }
            addToken(type, LasmObject(objType, value));
        } catch (...) {
            error.onError(NUMBER_PARSE_ERROR, line, path);
        }
    }

    void Scanner::scanIdentifier() {
        while (isAlphaNumeric(peek())) {
            advance();
        }

        std::string text = source.substr(start, current-start);
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

    lasmReal Scanner::stringToReal(const std::string& number) {
        return std::stod(number);
    }

    lasmNumber Scanner::stringToNumber(const std::string& number, int base) {
        return std::stol(number, nullptr, base);
    }

    void Scanner::addKeyword(std::string name, TokenType type) {
        keywords.insert(std::pair<std::string, TokenType>(name, type));
    }
}
