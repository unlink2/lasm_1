#include "interpreter.h"
#include "scanner.h"
#include "parser.h"
#include "instruction.h"
#include "instruction6502.h"
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
    InstructionSet6502 is;\
    TestCallback callback;\
    Scanner scanner(error, is, code, "");\
    auto tokens = scanner.scanTokens();\
    Parser parser(error, tokens, is);\
    auto stmts = parser.parse();\
    assert_int_equal(error.getType(), NO_ERROR);\
    assert_false(error.didError());\
    assert_int_equal(stmts.size(), stmtSize);\
    Interpreter interpreter(error, is, &callback);\
    interpreter.interprete(stmts);\
    assert_int_equal(error.getType(), NO_ERROR);\
    assert_false(error.didError());\
    assert_non_null(callback.object.get());\
    assert_int_equal(callback.object->getType(), objType);\
    __VA_ARGS__\
}

#define assert_parser_error(code, errorType) {\
    BaseError error;\
    InstructionSet6502 is;\
    TestCallback callback;\
    Scanner scanner(error, is, code, "");\
    auto tokens = scanner.scanTokens();\
    Parser parser(error, tokens, is);\
    auto stmts = parser.parse();\
    assert_true(error.didError());\
    assert_int_equal(error.getType(), errorType);\
}

#define assert_interpreter_error(code, stmtSize, errorType) {\
    BaseError error;\
    InstructionSet6502 is;\
    TestCallback callback;\
    Scanner scanner(error, is, code, "");\
    auto tokens = scanner.scanTokens();\
    Parser parser(error, tokens, is);\
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

    assert_interpreter_success("let a = 2; a+1;", 2, NUMBER_O, {assert_int_equal(callback.object->toNumber(), 3);});

    assert_interpreter_success("let a = \"Hi\"+\"World\"; a;", 2, STRING_O,
            {assert_cc_string_equal(callback.object->toString(), std::string("HiWorld"));});

    assert_interpreter_success("let a = \"Hi\"+\"World\"; a = 22;", 2, NUMBER_O, {assert_int_equal(callback.object->toNumber(), 22);});

    assert_interpreter_success("let a = 1; { let a = 2; } a;", 3, NUMBER_O, {assert_int_equal(callback.object->toNumber(), 1);});

    assert_interpreter_success("let a = 1; { a = 22; } a;", 3, NUMBER_O, {assert_int_equal(callback.object->toNumber(), 22);});

    assert_interpreter_success("let a = 1; if (a == 1) {a = 2;}", 2, NUMBER_O, {assert_int_equal(callback.object->toNumber(), 2);});

    assert_interpreter_success("let a = 1; if (a == 2) {a = 2;} else {a;}", 2, NUMBER_O, {assert_int_equal(callback.object->toNumber(), 1);});

    assert_interpreter_success("let a = 1; if (a == 1 or false) {a = 2;} else {a;}", 2, NUMBER_O, {assert_int_equal(callback.object->toNumber(), 2);});
    assert_interpreter_success("let a = 1; if (a == 1 and false) {a = 2;} else {a;}", 2, NUMBER_O, {assert_int_equal(callback.object->toNumber(), 1);});

    assert_interpreter_success("let a = 10; while (a > 0) a = a -1;", 2, NUMBER_O, {assert_int_equal(callback.object->toNumber(), 0);});

    assert_interpreter_success("let a = 10; for (;a > 0; a = a - 1) {} ", 2, NUMBER_O, {assert_int_equal(callback.object->toNumber(), 0);});
    assert_interpreter_success("let a = 10; for (;a > 0;) {a = a - 1;} ", 2, NUMBER_O, {assert_int_equal(callback.object->toNumber(), 0);});
    assert_interpreter_success("let a; for (a = 0;a > 0;a = a - 1) {} ", 2, NUMBER_O, {assert_int_equal(callback.object->toNumber(), 0);});
    assert_interpreter_success("let b = 0; for (let a = 10;a > 0;a = a - 1) {b = b + 1;} b; ", 3, NUMBER_O, {assert_int_equal(callback.object->toNumber(), 10);});

    assert_interpreter_success("lo(0xFF81); ", 1, NUMBER_O, {assert_int_equal(callback.object->toNumber(), 0x81);});
    assert_interpreter_success("hi(0x81FF); ", 1, NUMBER_O, {assert_int_equal(callback.object->toNumber(), 0x81);});

    assert_interpreter_success("fn x() {return 1+1;} x();",
            2, NUMBER_O, {assert_int_equal(callback.object->toNumber(), 2);});

    assert_interpreter_success("fn x(a, b) { if ((a + b) == 2) { return 2; } else {return 5;}} let a = x(1, 1); a;",
            3, NUMBER_O, {assert_int_equal(callback.object->toNumber(), 2);});
    assert_interpreter_success("fn x(a, b) { if ((a + b) == 2) { return 2; } else {return 5;}} let a = x(1, 2); a;",
            3, NUMBER_O, {assert_int_equal(callback.object->toNumber(), 5);});
    assert_interpreter_success("fn x() {} let a  = x(); a;", 3, NIL_O, {assert_null(callback.object->toNil());});
    assert_interpreter_success("fn x() {return;} let a  = x(); a;", 3, NIL_O, {assert_null(callback.object->toNil());});

    assert_interpreter_success("0x8283 & 0xFF;", 1, NUMBER_O, {assert_int_equal(callback.object->toNumber(), 0x8283 & 0xFF);});
    assert_interpreter_success("0x8283 | 0xFF;", 1, NUMBER_O, {assert_int_equal(callback.object->toNumber(), 0x8283 | 0xFF);});
    assert_interpreter_success("0x8283 ^ 0xFF;", 1, NUMBER_O, {assert_int_equal(callback.object->toNumber(), 0x8283 ^ 0xFF);});
    assert_interpreter_success("~0x8283;", 1, NUMBER_O, {assert_int_equal(callback.object->toNumber(), ~0x8283);});
    assert_interpreter_success("0x8283 >> 2;", 1, NUMBER_O, {assert_int_equal(callback.object->toNumber(), 0x8283 >> 2);});
    assert_interpreter_success("0x8283 << 2;", 1, NUMBER_O, {assert_int_equal(callback.object->toNumber(), 0x8283 << 2);});
}

void test_interpreter_errors(void **state) {
    assert_interpreter_error("\"Hi\" >= 3;", 1, TYPE_ERROR);

    assert_parser_error("\"Hi\" >= 3", MISSING_SEMICOLON);

    assert_interpreter_error("2 / 0;", 1, DIVISION_BY_ZERO);

    assert_interpreter_error("a + 1;", 1, UNDEFINED_REF);

    assert_interpreter_error("a=1;", 1, UNDEFINED_REF);

    assert_parser_error("let a = 1; a+1=2;", BAD_ASSIGNMENT);

    assert_parser_error("{ let a = 1;", BLOCK_NOT_CLOSED_ERROR);

    assert_parser_error("if (true { 1; }", EXPECTED_EXPRESSION);

    assert_parser_error("while (false { 1; }", EXPECTED_EXPRESSION);

    assert_parser_error("for (;false; { 1; }", EXPECTED_EXPRESSION);
    assert_parser_error("for () { 1; }", EXPECTED_EXPRESSION);

    assert_interpreter_error("fn x(a, b) {} let a  = x(1);", 2, ARITY_ERROR);
    assert_interpreter_error("fn x(a, b) {} let a  = x(1, 2, 3);", 2, ARITY_ERROR);
    assert_parser_error("fn x a, b) {} x(1, 2);", MISSING_LEFT_PAREN);
}
