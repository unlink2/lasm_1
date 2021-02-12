#include "error.h"

namespace lasm {
    std::string errorToString(ErrorType error) {
        switch (error) {
            case UNEXPECTED_CHAR:
                return "Unexpected character";
            case UNTERMINATED_STRING:
                return "Unterminated string";
            case NUMBER_PARSE_ERROR:
                return "Number parser error";
            case MISSING_LEFT_PAREN:
                return "Expected '('";
            case MISSING_RIHGT_PAREN:
                return "Expected ')'";
            case EXPECTED_EXPRESSION:
                return "Expected expression";
            case TYPE_ERROR:
                return "Type error";
            case DIVISION_BY_ZERO:
                return "Division by zero";
            case MISSING_SEMICOLON:
                return "Expected ';'";
            case MISSING_IDENTIFIER:
                return "Expected identifier";
            case UNDEFINED_REF:
                return "Undefined reference";
            case BAD_ASSIGNMENT:
                return "Bad assignment";
            case BLOCK_NOT_OPENED_ERROR:
                return "Expected '{'";
            case BLOCK_NOT_CLOSED_ERROR:
                return "Expected '}'";
            case NOT_CALLABLE:
                return "is not callable";
            case ARITY_ERROR:
                return "Unexpected amount of arguments";
            case VALUE_OUT_OF_RANGE:
                return "Value out of range";
            case INVALID_INSTRUCTION:
                return "Invalid instruction";
            case MISSING_COMMA:
                return "Expected comma";
            case INDEX_OUT_OF_BOUNDS:
                return "Index out of bounds";
            case FILE_NOT_FOUND:
                return "File not found";
            case CALLSTACK_UNWIND:
                return "Stack trace";
            default:
                return "";
        }

        return "";
    }


    std::string LasmTypeError::typeToString(ObjectType type) {
        switch (type) {
            case NIL_O:
                return "nil";
            case NUMBER_O:
                return "number";
            case REAL_O:
                return "nil";
            case STRING_O:
                return "string";
            case CALLABLE_O:
                return "callable";
            case LIST_O:
                return "list";
            case BOOLEAN_O:
                return "boolean";

        }

        return "unknown type";
    }
}
