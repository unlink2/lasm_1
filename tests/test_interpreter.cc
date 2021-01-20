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
    assert_false(error.didError());\
    interpreter.interprete(stmts);\
    assert_int_equal(error.getType(), NO_ERROR);\
    assert_false(error.didError());\
    assert_non_null(callback.object.get());\
    assert_int_equal(callback.object->getType(), objType);\
    __VA_ARGS__\
}

#define assert_code6502(code, codeSize, address, ...) assert_code6502_a(code, codeSize, address, 0, __VA_ARGS__)

#define assert_code6502_a(code, codeSize, address, resultIndex, ...) {\
    BaseError error;\
    InstructionSet6502 is;\
    TestCallback callback;\
    Scanner scanner(error, is, code, "");\
    auto tokens = scanner.scanTokens();\
    Parser parser(error, tokens, is);\
    auto stmts = parser.parse();\
    assert_int_equal(error.getType(), NO_ERROR);\
    assert_false(error.didError());\
    Interpreter interpreter(error, is, &callback);\
    assert_false(error.didError());\
    auto result = interpreter.interprete(stmts);\
    assert_false(error.didError());\
    assert_int_equal(error.getType(), NO_ERROR);\
    assert_int_equal(result[resultIndex].getSize(), codeSize);\
    assert_int_equal(result[resultIndex].getAddress(), address);\
    char dataArray[] = __VA_ARGS__;\
    assert_memory_equal(dataArray, result[resultIndex].getData().get(), codeSize);\
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
    assert_false(error.didError());\
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

    assert_code6502("lda #0xFF;", 2, 0, {(char)0x69, (char)0xFF});

    assert_code6502("org 0x02; align 0x04, 0xFF;", 2, 2, {(char)0xFF, (char)0xFF});
    assert_code6502("org 0x02; fill 0x06, 0xFF;", 4, 2, {(char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF});

    assert_code6502("db \"Hello\", 2, 3, true;", 6, 0, {'H', 'e', 'l', 'l', 'o', '\0', 0x02, 0x03, 1});
    assert_code6502("dw 100, 100;", 4, 0, {0x64, 0, 0, 0, 0x64, 0, 0, 0});
    assert_code6502("dh 100;", 2, 0, {0x64, 0});
    assert_code6502("dd 100;", 8, 0, {0x64, 0, 0, 0, 0, 0, 0, 0, 0});

    assert_code6502_a("bss 100 {test1 1, test2 2, test3 3} lda #test1; lda #test2; lda #test3;",
            2, 0, 0, {0x69, 100, 0x69, 100+1, 0x69, 100+3});
    assert_code6502_a("bss 100 {test1 1, test2 2, test3 3} lda #test1; lda #test2; lda #test3;",
            2, 2, 1, {0x69, 100+1, 0x69, 100+3});
    assert_code6502_a("bss 100 {test1 1, test2 2, test3 3} lda #test1; lda #test2; lda #test3;",
            2, 4, 2, {0x69, 100+3});


    // lists
    assert_code6502_a("let a = [[2, 3], 1, 2, 3]; let s = \"Hello\"; lda #a[0][1]; lda #a[1]; lda #a[2]; lda #s[1];",
            2, 0, 0, {0x69, 3});
    assert_code6502_a("let a = [[2, 3], 1, 2, 3]; let s = \"Hello\"; lda #a[0][1]; lda #a[1]; lda #a[2]; lda #s[1];",
            2, 2, 1, {0x69, 1});
    assert_code6502_a("let a = [[2, 3], 1, 2, 3]; let s = \"Hello\"; lda #a[0][1]; lda #a[1]; lda #a[2]; lda #s[1];",
            2, 4, 2, {0x69, 2});
    assert_code6502_a("let a = [[2, 3], 1, 2, 3]; let s = \"Hello\"; lda #a[0][1]; lda #a[1]; lda #a[2]; lda #s[1];",
            2, 6, 3, {0x69, 'e'});

    // list assign
    assert_code6502_a("let a = [[2, 3], 1, 2, 3]; a[1] = 4; a[0][1] = 100; lda #a[0][1]; lda #a[1]; lda #a[2];",
            2, 0, 0, {0x69, 100});
    assert_code6502_a("let a = [[2, 3], 1, 2, 3]; a[1] = 4; a[0][1] = 100; lda #a[0][1]; lda #a[1]; lda #a[2];",
            2, 2, 1, {0x69, 4});
    assert_code6502_a("let a = [[2, 3], 1, 2, 3]; a[1] = 4; a[0][1] = 100; lda #a[0][1]; lda #a[1]; lda #a[2];",
            2, 4, 2, {0x69, 2});

    // label resolve
    std::string labelResolveCode = "fn labels() { lda #test_label; test_label: } lda #1; labels(); labels(); global: lda #global;"
    "for (let i = 0; i < 2; i = i + 1) { lda #label; lda #global; lda #after; label: } after:";
    assert_code6502_a(labelResolveCode, 2, 0, 0, {0x69, 0x01});
    assert_code6502_a(labelResolveCode, 2, 2, 1, {0x69, 0x04});
    assert_code6502_a(labelResolveCode, 2, 4, 2, {0x69, 0x06});
    assert_code6502_a(labelResolveCode, 2, 6, 3, {0x69, 0x06});
    assert_code6502_a(labelResolveCode, 2, 8, 4, {0x69, 0x0E});
    assert_code6502_a(labelResolveCode, 2, 10, 5, {0x69, 0x06});
    assert_code6502_a(labelResolveCode, 2, 12, 6, {0x69, 0x14});
    assert_code6502_a(labelResolveCode, 2, 14, 7, {0x69, 0x14});
    assert_code6502_a(labelResolveCode, 2, 16, 8, {0x69, 0x06});
    assert_code6502_a(labelResolveCode, 2, 18, 9, {0x69, 0x14});

    // test absolute
    assert_code6502_a("lda 0x4021;", 3, 0, 0, {0x6D, 0x21, 0x40});

    // absolute, x
    assert_code6502_a("lda 0x4021, x;", 3, 0, 0, {0x7D, 0x21, 0x40});

    // absolute, y
    assert_code6502_a("lda 0x4021, y;", 3, 0, 0, {0x79, 0x21, 0x40});

    // zp
    assert_code6502_a("lda 0x21;", 2, 0, 0, {0x65, 0x21});

    // zp, x
    assert_code6502_a("lda 0x21, x;", 2, 0, 0, {0x75, 0x21});

    // zp, y -> does not exists so it will be absolute
    assert_code6502_a("lda 0x21, y;", 3, 0, 0, {0x79, 0x21, 0x00});

    // indirect, x
    assert_code6502_a("lda (0x21, x);", 2, 0, 0, {0x61, 0x21});

    // indirect, y
    assert_code6502_a("lda (0x21), y;", 2, 0, 0, {0x71, 0x21});
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

    assert_parser_error("org", EXPECTED_EXPRESSION);
    assert_parser_error("align", EXPECTED_EXPRESSION);
    assert_parser_error("align 0xFF", MISSING_COMMA);
    assert_parser_error("align 0xFF, ", EXPECTED_EXPRESSION);
    assert_interpreter_error("align 0xFF, 256;", 1, VALUE_OUT_OF_RANGE);
    assert_parser_error("fill", EXPECTED_EXPRESSION);
    assert_parser_error("fill 0xFF", MISSING_COMMA);
    assert_parser_error("fill 0xFF, ", EXPECTED_EXPRESSION);
    assert_interpreter_error("fill 0xFF, 256;", 1, VALUE_OUT_OF_RANGE);

    assert_parser_error("db 0xFF 0xFF;", MISSING_SEMICOLON);
    assert_parser_error("db;", EXPECTED_EXPRESSION);
    assert_interpreter_error("db hi;", 1, TYPE_ERROR);

    assert_parser_error("bss test 100", BLOCK_NOT_OPENED_ERROR);
    assert_parser_error("bss { test 100}", EXPECTED_EXPRESSION);
    assert_parser_error("bss 100 { test 100", MISSING_COMMA);
    assert_parser_error("bss 100 { 100 100}", MISSING_IDENTIFIER);
    assert_parser_error("bss 100 { 100 100, test 100}", MISSING_IDENTIFIER);
    assert_interpreter_error("bss 3.1 { test 100}", 1, TYPE_ERROR);
    assert_interpreter_error("bss 3 { test 3.1}", 1, TYPE_ERROR);

    // list errors
    assert_parser_error("let a = [;", EXPECTED_EXPRESSION);
    assert_parser_error("let a = [1 2];", MISSING_COMMA);
    assert_interpreter_error("let a = [1, 2, 3]; a[4];", 2, INDEX_OUT_OF_BOUNDS);
    assert_interpreter_error("let a = \"hi\"; a[4];", 2, INDEX_OUT_OF_BOUNDS);
    assert_interpreter_error("let a = [1, 2, 3]; a[\"hello\"];", 2, TYPE_ERROR);
    assert_interpreter_error("let a = 22; a[1];", 2, TYPE_ERROR);

    assert_interpreter_error("let a = [1, 2, 3]; a[4] = 4;", 2, INDEX_OUT_OF_BOUNDS);
    assert_interpreter_error("let a = [1, 2, 3]; a[\"hello\"] = 3;", 2, TYPE_ERROR);
    assert_interpreter_error("let a = 22; a[1] = 1;", 2, TYPE_ERROR);

    // asm syntax errors
    assert_interpreter_error("lda 0x4FFF1;", 1, VALUE_OUT_OF_RANGE);
    assert_parser_error("lda 0x4FFF1, b;", INVALID_INSTRUCTION);

    assert_interpreter_error("lda (0xFF+1, x);", 1, VALUE_OUT_OF_RANGE);
    assert_interpreter_error("lda (0xFF+1), y;", 1, VALUE_OUT_OF_RANGE);
    assert_interpreter_error("lda (0xFF+1), x;", 1, INVALID_INSTRUCTION);
    assert_interpreter_error("lda (0xFF+1, y);", 1, INVALID_INSTRUCTION);
    assert_interpreter_error("lda (0xFF+1, a);", 1, INVALID_INSTRUCTION);
    assert_interpreter_error("lda (0xFF+1), l;", 1, INVALID_INSTRUCTION);
}

void test_misc_interpreter(void **state) {
    InstructionInfo result(nullptr);
    result.addOpcode(0xF1, "test");
    assert_true(result.hasOpcode("test"));
    assert_false(result.hasOpcode("not_included"));
    assert_int_equal(result.getOpcode("test"), 0xF1);
}
