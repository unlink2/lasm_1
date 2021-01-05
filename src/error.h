#ifndef __ERROR_H__
#define __ERROR_H__

#include <iostream>
#include <exception>
#include <memory>
#include "token.h"

namespace lasm {
    typedef enum {
        NO_ERROR,
        UNEXPECTED_CHAR,
        UNTERMINATED_STRING,
        NUMBER_PARSE_ERROR,
        MISSING_RIHGT_PAREN,
        EXPECTED_EXPRESSION
    } ErrorType;

    std::string errorToString(ErrorType error);

    class LasmException: public std::exception {
        public:
            ~LasmException() {}
            virtual const char* what() const throw() = 0;
    };

    class ParserException: public LasmException {
        public:
            ParserException(std::shared_ptr<Token> token, ErrorType type):
                token(token), type(type) {}
            ~ParserException() {}
            virtual const char* what() const throw()  {
                return errorToString(type).c_str();
            }

            std::shared_ptr<Token> getToken() {
                return token;
            }
        private:
            std::shared_ptr<Token> token;
            ErrorType type;
    };

    class BaseError {
        public:
            BaseError() {}
            virtual void onError(ErrorType type, unsigned long line, std::string path) {
                hasErrored = true;
                this->type = type;
                this->path = path;
                this->line = line;
            }

            virtual void onError(ErrorType type, std::shared_ptr<Token> token) {
                hasErrored = true;
                this->type = type;
                this->path = token->getPath();
                this->line = token->getLine();
            }

            bool didError() {
                return hasErrored;
            }

            ErrorType getType() {
                return type;
            }

            unsigned long getLine() {
                return line;
            }

            std::shared_ptr<Token> getToken() {
                return token;
            }

        protected:
            bool hasErrored = false;
            unsigned long line;
            ErrorType type = NO_ERROR;
            std::string path;
            std::shared_ptr<Token> token = std::shared_ptr<Token>(nullptr);
    };
}

#endif 
