#include "parser.h"
#include "scanner.h"
#include "astprinter.h"

#include "test_parser.h"
#include "macros.h"

using namespace lasm;

void test_parser(void **state) {
    // this code makes little sense but that is ok for now
    std::string code = "1 + 2 * (1 - 5 + 2) == 2 >= 3 + \"Hello\" + !false;";

    BaseError error;
    BaseInstructionSet is;

    Scanner scanner(error, is, code, std::string("test"));

    auto tokens = scanner.scanTokens();
    assert_false(error.didError());

    Parser parser(error, tokens);
    auto stmts = parser.parse();

    assert_false(error.didError());
    assert_int_equal(stmts.size(), 1);
    assert_int_equal(stmts[0]->getType(), EXPRESSION_STMT);

    auto exprStmt = static_cast<ExpressionStmt*>(stmts[0].get());

    AstPrinter astPrinter;
    auto result = astPrinter.toString(exprStmt->expr.get());

    assert_cc_string_equal(result, std::string("(== (+ 1 (* 2 (Group (+ (- 1 5) 2)))) (>= 2 (+ (+ 3 Hello) (! false))))"));
}

