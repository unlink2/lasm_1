#ifndef __TYPES_H__
#define __TYPES_H__

namespace lasm {
    typedef enum {
        NO_TOKEN, // this never matches any token!
        // single char tokens
        LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
        LEFT_BRACKET, RIGHT_BRACKET,
        COMMA, DOT, MINUS, PLUS, SEMICOLON, NEWLINE, SLASH, STAR,
        LOR, LXOR, LAND, LNOT, LEFT_SHIFT, RIGHT_SHIFT,

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

        EOF_T
    } TokenType;
}

#endif 
