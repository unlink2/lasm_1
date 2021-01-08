#include "interpreter.h"
#include "scanner.h"
#include "parser.h"
#include "instruction.h"
#include <memory>

#include "macros.h"
#include "test_interpreter.h"

using namespace lasm;

class TestCallback: public InterpreterCallback {
    public:
        virtual void onStatementExecuted(LasmObject *object) {
            this->object = std::make_shared<LasmObject>(LasmObject(object));
        }
        std::shared_ptr<LasmObject> object = std::shared_ptr<LasmObject>(nullptr);
};

#define assert_interpreter_success(code, stmtSize, objType, ...) {\
    BaseError error;\
    BaseInstructionSet is;\
    TestCallback callback;\
    Scanner scanner(error, is, code, "");\
    auto tokens = scanner.scanTokens();\
    Parser parser(error, tokens);\
    auto stmts = parser.parse();\
    assert_false(error.didError());\
    assert_int_equal(stmts.size(), stmtSize);\
    Interpreter interpreter(error, is, &callback);\
    interpreter.interprete(stmts);\
    assert_false(error.didError());\
    assert_non_null(callback.object.get());\
    assert_int_equal(callback.object->getType(), objType);\
    __VA_ARGS__\
}

#define assert_parser_error(code, errorType) {\
    BaseError error;\
    BaseInstructionSet is;\
    TestCallback callback;\
    Scanner scanner(error, is, code, "");\
    auto tokens = scanner.scanTokens();\
    Parser parser(error, tokens);\
    try {\
        auto stmts = parser.parse();\
        assert_true(false);\
    } catch (ParserException &e) {\
        assert_true(error.didError());\
        assert_int_equal(error.getType(), errorType);\
    }\
}

#define assert_interpreter_error(code, stmtSize, errorType) {\
    BaseError error;\
    BaseInstructionSet is;\
    TestCallback callback;\
    Scanner scanner(error, is, code, "");\
    auto tokens = scanner.scanTokens();\
    Parser parser(error, tokens);\
    auto stmts = parser.parse();\
    assert_false(error.didError());\
    assert_int_equal(stmts.size(), stmtSize);\
    Interpreter interpreter(error, is, &callback);\
    interpreter.interprete(stmts);\
    assert_true(error.didError());\
    assert_int_equal(error.getType(), errorType);\
}

void test_interpreter(void **state) {
    // test interpreter functionality
    assert_interpreter_success("(2 + 3) * 2;", 1, NUMBER_O, {assert_int_equal((int)callback.object->toNumber(), 10);});

    assert_interpreter_success("(2.0 + 3) * 2.1;", 1, REAL_O, {assert_float_equal(callback.object->toReal(), 10.5, 0.001);});

    assert_interpreter_success("\"Hello\" + \"World\";", 1, STRING_O,
            {assert_cc_string_equal(callback.object->toString(), std::string("HelloWorld"));});

    assert_interpreter_success("(\"Hello\") == \"Hello\";", 1, BOOLEAN_O, {assert_true(callback.object->toBool());});

    assert_interpreter_success("2 <= 3;", 1, BOOLEAN_O, {assert_true(callback.object->toBool());});
}

void test_interpreter_errors(void **state) {
    assert_interpreter_error("\"Hi\" >= 3;", 1, TYPE_ERROR);

    assert_parser_error("\"Hi\" >= 3", MISSING_SEMICOLON);

    assert_interpreter_error("2 / 0;", 1, DIVISION_BY_ZERO);
}
