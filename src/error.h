#ifndef __ERROR_H__
#define __ERROR_H__

#include <iostream>
#include <exception>
#include <memory>
#include "token.h"
#include "object.h"
#include <vector>

namespace lasm {
    typedef enum {
        NO_ERROR,
        UNEXPECTED_CHAR,
        UNTERMINATED_STRING,
        NUMBER_PARSE_ERROR,
        MISSING_LEFT_PAREN,
        MISSING_RIHGT_PAREN,
        EXPECTED_EXPRESSION,
        TYPE_ERROR,
        DIVISION_BY_ZERO,
        MISSING_SEMICOLON,
        MISSING_IDENTIFIER,
        UNDEFINED_REF,
        BAD_ASSIGNMENT,
        BLOCK_NOT_OPENED_ERROR,
        BLOCK_NOT_CLOSED_ERROR,
        NOT_CALLABLE,
        ARITY_ERROR
    } ErrorType;

    std::string errorToString(ErrorType error);

    class LasmException: public std::exception {
        public:
            LasmException(ErrorType type, std::shared_ptr<Token> token=std::shared_ptr<Token>(nullptr)):
                type(type), token(token) {}
            ~LasmException() {}

            virtual const char* what() const throw()  {
                return errorToString(type).c_str();
            }

            ErrorType getType() { return type; }

            std::shared_ptr<Token> getToken() {
                return token;
            }
        private:
            ErrorType type;
            std::shared_ptr<Token> token;
    };

    class LasmTypeError: public LasmException {
        public:
            LasmTypeError(std::vector<ObjectType> expected, ObjectType got, std::shared_ptr<Token> token):
                LasmException(TYPE_ERROR, token), expected(expected), got(got) {}
            ~LasmTypeError() {}

        private:
            std::vector<ObjectType> expected;
            ObjectType got;
    };

    class LasmDivisionByZero: public LasmException {
        public:
            LasmDivisionByZero(std::shared_ptr<Token> token):
                LasmException::LasmException(DIVISION_BY_ZERO, token) {}
            ~LasmDivisionByZero() {}
    };

    class ParserException: public LasmException {
        public:
            ParserException(std::shared_ptr<Token> token, ErrorType type):
               LasmException::LasmException(type), token(token) {}
            ~ParserException() {}

            std::shared_ptr<Token> getToken() {
                return token;
            }
        private:
            std::shared_ptr<Token> token;
    };

    class LasmUndefinedReference: public LasmException {
        public:
            LasmUndefinedReference(std::shared_ptr<Token> token):
                LasmException::LasmException(UNDEFINED_REF, token) {}
            ~LasmUndefinedReference() {}
    };

    class LasmNotCallable: public LasmException {
        public:
            LasmNotCallable(std::shared_ptr<Token> token):
                LasmException::LasmException(NOT_CALLABLE, token) {}
    };

    class LasmArityError: public LasmException {
        public:
            LasmArityError(std::shared_ptr<Token> token):
                LasmException::LasmException(ARITY_ERROR, token) {}
    };

    /**
     * Error handler class
     * This is called when an exception occurs during scanning, parsing, interpreting
     * or code-gen
     * Generally exceptions are thrown inside the modules, but they are handeled internally and
     * then passed to this callback to allow the modules to handle more than
     * one syntax error at once
     */
    class BaseError {
        public:
            BaseError() {}
            virtual void onError(ErrorType type, unsigned long line, std::string path, LasmException *e=nullptr) {
                hasErrored = true;
                this->type = type;
                this->path = path;
                this->line = line;
            }

            virtual void onError(ErrorType type, std::shared_ptr<Token> token, LasmException *e=nullptr) {
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
