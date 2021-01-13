#ifndef __TYPES_H__
#define __TYPES_H__

namespace lasm {
    enum Endianess {
        BIG,
        LITTLE,
        RAW
    };

    typedef enum {
        NO_TOKEN, // this never matches any token!
        // single char tokens
        LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
        LEFT_BRACKET, RIGHT_BRACKET,
        COMMA, DOT, MINUS, PLUS, SEMICOLON, NEWLINE, SLASH, STAR, PERCENT,
        BIN_AND, BIN_OR, BIN_SHIFT_LEFT, BIN_SHIFT_RIGHT, BIN_XOR, BIN_NOT,

        // more char token
        BANG, BANG_EQUAL,
        EQUAL, EQUAL_EQUAL,
        GREATER, GREATER_EQUAL,
        LESS, LESS_EQUAL, HASH,

        // literals
        IDENTIFIER, STRING, NUMBER, REAL, BOOLEAN, ARRAYLIST, BYTES,

        // keywords
        AND, ELSE, FALSE, FUNCTION, FOR, IF, NIL, OR,
        RETURN, TRUE, LET, WHILE, IMPORT,

        // assembler
        INSTRUCTION, LABEL, ASM_DIRECTIVE,
        ORG, FILL, ALIGN,
        DEFINE_BYTE, DEFINE_HALF, DEFINE_WORD, DEFINE_DOUBLE,

        EOF_T
    } TokenType;
}

#endif 
