#include "expr.h"
#include "astprinter.h"
#include "test_expr.h"
#include "macros.h"
#include <memory>

using namespace lasm;

void test_expr(void **state) {
    // build test expr tree and print it

    LasmObject nullLiteral(NIL_O, nullptr);

    // first member
    BinaryExpr expr;
    std::shared_ptr<UnaryExpr> l1(new UnaryExpr());
    std::shared_ptr<Token> o1(new Token(STAR, "*", LasmObject(NIL_O, nullptr), 1, "", 0, nullptr));
    std::shared_ptr<GroupingExpr> r1(new GroupingExpr());
    expr.left = l1;
    expr.op = o1;
    expr.right = r1;

    // unary
    auto unary = static_cast<UnaryExpr*>(expr.left.get());
    LasmObject literal1 = LasmObject(NUMBER_O, lasmNumber(123));
    std::shared_ptr<LiteralExpr> r2(new LiteralExpr(literal1));
    std::shared_ptr<Token> o2(new Token(MINUS, "-", LasmObject(NIL_O, nullptr), 1, "", 0, nullptr));

    unary->right = r2;
    unary->op = o2;

    // grouping
    auto grouping = static_cast<GroupingExpr*>(expr.right.get());
    auto literal2 = LasmObject(REAL_O, 3.1415);
    std::shared_ptr<LiteralExpr> r3(new LiteralExpr(literal2));
    grouping->expression = r3;

    // walk the tree
    AstPrinter printer;
    auto result = printer.toString(expr);

    assert_cc_string_equal(result, std::string("(* (- 123) (Group 3.1415))"));
}
