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
            default:
                return "";
        }

        return "";
    }
}
