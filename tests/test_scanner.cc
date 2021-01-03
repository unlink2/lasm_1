#include "test_scanner.h"

void test_scannerIsAlphaNumeric(void **state) {
    lasm::BaseError error;
    lasm::BaseInstructionSet instructions;
    lasm::Scanner scanner(error, instructions, "", "");
    for (char c = 'a'; c <= 'z'; c++) {
        assert_true(scanner.isAlpha(c));
        assert_false(scanner.isDigit(c));
        assert_true(scanner.isAlphaNumeric(c));
    }
    for (char c = 'A'; c <= 'Z'; c++) {
        assert_true(scanner.isAlpha(c));
        assert_false(scanner.isDigit(c));
        assert_true(scanner.isAlphaNumeric(c));
    }
    for (char c = '0'; c <= '9'; c++) {
        assert_false(scanner.isAlpha(c));
        assert_true(scanner.isDigit(c));
        assert_true(scanner.isAlphaNumeric(c));
    }

    // hex
    for (char c = '0'; c <= '9'; c++) {
        assert_true(scanner.isHexDigit(c));
    }
    for (char c = 'a'; c <= 'f'; c++) {
        assert_true(scanner.isHexDigit(c));
    }
    for (char c = 'A'; c <= 'F'; c++) {
        assert_true(scanner.isHexDigit(c));
    }
    for (char c = 'G'; c <= 'Z'; c++) {
        assert_false(scanner.isHexDigit(c));
    }
    for (char c = 'g'; c <= 'z'; c++) {
        assert_false(scanner.isHexDigit(c));
    }

    // bin
    assert_true(scanner.isBinDigit('1'));
    assert_true(scanner.isBinDigit('0'));
    assert_false(scanner.isBinDigit('2'));
}

void test_scanner(void **state) {
    lasm::BaseError error;
    lasm::BaseInstructionSet instructions;
    instructions.addInstruction("lda", new lasm::InstructionMapper());

    lasm::Scanner scanner(error, instructions, "()[] , \n"
            ".\t\r\"Hello\\\"\\t \\\"World\"'Hello World!'- + ;  * / ! != = == < > <= >=\n"
            "# comment\n"
            "1234 3.1415\nand\norange\n0b101\n0xFF1 lda", "unit_test");

    auto scanned = scanner.scanTokens();
    assert_false(error.didError());

    auto it = scanned.begin();

    {
        lasm::Token t = *it;
        assert_int_equal(t.getLine(), 1);
        assert_cc_string_equal(t.getLexeme(), std::string("("));
        assert_int_equal(t.getType(), lasm::LEFT_PAREN);
        assert_int_equal(t.getLiteral().getType(), lasm::NIL);
        assert_null(t.getLiteral().castTo<std::nullptr_t>());
    }
    // TODO finish tests

    it++;
    {
        lasm::Token t = *it;
        assert_int_equal(t.getLine(), 1);
        assert_cc_string_equal(t.getLexeme(), std::string(")"));
        assert_int_equal(t.getType(), lasm::RIGHT_PAREN);
        assert_int_equal(t.getLiteral().getType(), lasm::NIL);
        assert_null(t.getLiteral().castTo<std::nullptr_t>());
    }

    it++;
}
