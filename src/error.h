#ifndef __ERROR_H__
#define __ERROR_H__

#include <iostream>

namespace lasm {
    typedef enum {
        NO_ERROR,
        UNEXPECTED_CHAR,
        UNTERMINATED_STRING,
        NUMBER_PARSE_ERROR
    } ErrorType;

    std::string errorToString(ErrorType error);

    class BaseError {
        public:
            BaseError() {}
            virtual void onError(ErrorType type, unsigned long line, std::string path) {
                hasErrored = true;
                this->type = type;
                this->path = path;
            }

            bool didError() {
                return hasErrored;
            }

            ErrorType getType() {
                return type;
            }

        protected:
            bool hasErrored = false;
            ErrorType type = NO_ERROR;
            std::string path;
    };
}

#endif 
