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
    instructions.addInstruction("lda", new lasm::InstructionParser());

    lasm::Scanner scanner(error, instructions, "()[] , \n"
            ".\t\r\"Hello\\\"\\t \\\"World\"'Hello World!'- + ;  * / ! != = == < > <= >=\n"
            "// comment\n"
            "1234 3.1415\nand\norange\n0b101\n0xFF1 lda testLabel:", "unit_test");

    auto scanned = scanner.scanTokens();
    assert_int_equal(error.getType(), lasm::NO_ERROR);
    assert_false(error.didError());

    auto it = scanned.begin();

    {
        auto t = *it;
        assert_int_equal(t->getLine(), 1);
        assert_cc_string_equal(t->getLexeme(), std::string("("));
        assert_int_equal(t->getType(), lasm::LEFT_PAREN);
        assert_int_equal(t->getLiteral().getType(), lasm::NIL_O);
        assert_null(t->getLiteral().castTo<std::nullptr_t>());
    }
    // TODO finish tests

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 1);
        assert_cc_string_equal(t->getLexeme(), std::string(")"));
        assert_int_equal(t->getType(), lasm::RIGHT_PAREN);
        assert_int_equal(t->getLiteral().getType(), lasm::NIL_O);
        assert_null(t->getLiteral().castTo<std::nullptr_t>());
    }

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 1);
        assert_cc_string_equal(t->getLexeme(), std::string("["));
        assert_int_equal(t->getType(), lasm::LEFT_BRACKET);
        assert_int_equal(t->getLiteral().getType(), lasm::NIL_O);
        assert_null(t->getLiteral().castTo<std::nullptr_t>());
    }

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 1);
        assert_cc_string_equal(t->getLexeme(), std::string("]"));
        assert_int_equal(t->getType(), lasm::RIGHT_BRACKET);
        assert_int_equal(t->getLiteral().getType(), lasm::NIL_O);
        assert_null(t->getLiteral().castTo<std::nullptr_t>());
    }

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 1);
        assert_cc_string_equal(t->getLexeme(), std::string(","));
        assert_int_equal(t->getType(), lasm::COMMA);
        assert_int_equal(t->getLiteral().getType(), lasm::NIL_O);
        assert_null(t->getLiteral().castTo<std::nullptr_t>());
    }

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 2);
        assert_cc_string_equal(t->getLexeme(), std::string("."));
        assert_int_equal(t->getType(), lasm::DOT);
        assert_int_equal(t->getLiteral().getType(), lasm::NIL_O);
        assert_null(t->getLiteral().castTo<std::nullptr_t>());
    }

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 2);
        assert_cc_string_equal(t->getLexeme(), std::string("\"Hello\\\"\\t \\\"World\""));
        assert_int_equal(t->getType(), lasm::STRING);
        assert_int_equal(t->getLiteral().getType(), lasm::STRING_O);
        assert_cc_string_equal(t->getLiteral().castTo<std::string>(), std::string("Hello\"\t \"World"));
    }

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 2);
        assert_cc_string_equal(t->getLexeme(), std::string("'Hello World!'"));
        assert_int_equal(t->getType(), lasm::STRING);
        assert_int_equal(t->getLiteral().getType(), lasm::STRING_O);
        assert_cc_string_equal(t->getLiteral().castTo<std::string>(), std::string("Hello World!"));
    }

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 2);
        assert_cc_string_equal(t->getLexeme(), std::string("-"));
        assert_int_equal(t->getType(), lasm::MINUS);
        assert_int_equal(t->getLiteral().getType(), lasm::NIL_O);
        assert_null(t->getLiteral().castTo<std::nullptr_t>());
    }

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 2);
        assert_cc_string_equal(t->getLexeme(), std::string("+"));
        assert_int_equal(t->getType(), lasm::PLUS);
        assert_int_equal(t->getLiteral().getType(), lasm::NIL_O);
        assert_null(t->getLiteral().castTo<std::nullptr_t>());
    }

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 2);
        assert_cc_string_equal(t->getLexeme(), std::string(";"));
        assert_int_equal(t->getType(), lasm::SEMICOLON);
        assert_int_equal(t->getLiteral().getType(), lasm::NIL_O);
        assert_null(t->getLiteral().castTo<std::nullptr_t>());
    }

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 2);
        assert_cc_string_equal(t->getLexeme(), std::string("*"));
        assert_int_equal(t->getType(), lasm::STAR);
        assert_int_equal(t->getLiteral().getType(), lasm::NIL_O);
        assert_null(t->getLiteral().castTo<std::nullptr_t>());
    }

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 2);
        assert_cc_string_equal(t->getLexeme(), std::string("/"));
        assert_int_equal(t->getType(), lasm::SLASH);
        assert_int_equal(t->getLiteral().getType(), lasm::NIL_O);
        assert_null(t->getLiteral().castTo<std::nullptr_t>());
    }

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 2);
        assert_cc_string_equal(t->getLexeme(), std::string("!"));
        assert_int_equal(t->getType(), lasm::BANG);
        assert_int_equal(t->getLiteral().getType(), lasm::NIL_O);
        assert_null(t->getLiteral().castTo<std::nullptr_t>());
    }

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 2);
        assert_cc_string_equal(t->getLexeme(), std::string("!="));
        assert_int_equal(t->getType(), lasm::BANG_EQUAL);
        assert_int_equal(t->getLiteral().getType(), lasm::NIL_O);
        assert_null(t->getLiteral().castTo<std::nullptr_t>());
    }

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 2);
        assert_cc_string_equal(t->getLexeme(), std::string("="));
        assert_int_equal(t->getType(), lasm::EQUAL);
        assert_int_equal(t->getLiteral().getType(), lasm::NIL_O);
        assert_null(t->getLiteral().castTo<std::nullptr_t>());
    }

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 2);
        assert_cc_string_equal(t->getLexeme(), std::string("=="));
        assert_int_equal(t->getType(), lasm::EQUAL_EQUAL);
        assert_int_equal(t->getLiteral().getType(), lasm::NIL_O);
        assert_null(t->getLiteral().castTo<std::nullptr_t>());
    }

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 2);
        assert_cc_string_equal(t->getLexeme(), std::string("<"));
        assert_int_equal(t->getType(), lasm::LESS);
        assert_int_equal(t->getLiteral().getType(), lasm::NIL_O);
        assert_null(t->getLiteral().castTo<std::nullptr_t>());
    }

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 2);
        assert_cc_string_equal(t->getLexeme(), std::string(">"));
        assert_int_equal(t->getType(), lasm::GREATER);
        assert_int_equal(t->getLiteral().getType(), lasm::NIL_O);
        assert_null(t->getLiteral().castTo<std::nullptr_t>());
    }

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 2);
        assert_cc_string_equal(t->getLexeme(), std::string("<="));
        assert_int_equal(t->getType(), lasm::LESS_EQUAL);
        assert_int_equal(t->getLiteral().getType(), lasm::NIL_O);
        assert_null(t->getLiteral().castTo<std::nullptr_t>());
    }

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 2);
        assert_cc_string_equal(t->getLexeme(), std::string(">="));
        assert_int_equal(t->getType(), lasm::GREATER_EQUAL);
        assert_int_equal(t->getLiteral().getType(), lasm::NIL_O);
        assert_null(t->getLiteral().castTo<std::nullptr_t>());
    }

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 4);
        assert_cc_string_equal(t->getLexeme(), std::string("1234"));
        assert_int_equal(t->getType(), lasm::NUMBER);
        assert_int_equal(t->getLiteral().getType(), lasm::NUMBER_O);
        assert_int_equal(t->getLiteral().castTo<long>(), 1234);
    }

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 4);
        assert_cc_string_equal(t->getLexeme(), std::string("3.1415"));
        assert_int_equal(t->getType(), lasm::REAL);
        assert_int_equal(t->getLiteral().getType(), lasm::REAL_O);
        assert_float_equal(t->getLiteral().castTo<double>(), 3.1415, 0.001);
    }

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 5);
        assert_cc_string_equal(t->getLexeme(), std::string("and"));
        assert_int_equal(t->getType(), lasm::AND);
        assert_int_equal(t->getLiteral().getType(), lasm::NIL_O);
        assert_null(t->getLiteral().castTo<std::nullptr_t>());
    }

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 6);
        assert_cc_string_equal(t->getLexeme(), std::string("orange"));
        assert_int_equal(t->getType(), lasm::IDENTIFIER);
        assert_int_equal(t->getLiteral().getType(), lasm::NIL_O);
        assert_null(t->getLiteral().castTo<std::nullptr_t>());
    }

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 7);
        assert_cc_string_equal(t->getLexeme(), std::string("0b101"));
        assert_int_equal(t->getType(), lasm::NUMBER);
        assert_int_equal(t->getLiteral().getType(), lasm::NUMBER_O);
        assert_int_equal(t->getLiteral().castTo<long>(), 0b101);
    }

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 8);
        assert_cc_string_equal(t->getLexeme(), std::string("0xFF1"));
        assert_int_equal(t->getType(), lasm::NUMBER);
        assert_int_equal(t->getLiteral().getType(), lasm::NUMBER_O);
        assert_int_equal(t->getLiteral().castTo<long>(), 0xFF1);
    }

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 8);
        assert_cc_string_equal(t->getLexeme(), std::string("lda"));
        assert_int_equal(t->getType(), lasm::INSTRUCTION);
        assert_int_equal(t->getLiteral().getType(), lasm::NIL_O);
        assert_null(t->getLiteral().castTo<std::nullptr_t>());
    }

    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 8);
        assert_cc_string_equal(t->getLexeme(), std::string("testLabel:"));
        assert_int_equal(t->getType(), lasm::LABEL);
        assert_int_equal(t->getLiteral().getType(), lasm::NIL_O);
        assert_null(t->getLiteral().castTo<std::nullptr_t>());
    }

    // EOF
    it++;
    {
        auto t = *it;
        assert_int_equal(t->getLine(), 8);
        assert_cc_string_equal(t->getLexeme(), std::string(""));
        assert_int_equal(t->getType(), lasm::EOF_T);
        assert_int_equal(t->getLiteral().getType(), lasm::NIL_O);
        assert_null(t->getLiteral().castTo<std::nullptr_t>());
    }
}
