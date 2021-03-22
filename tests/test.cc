#include "test_scanner.h"
#include "test_token.h"
#include "test_object.h"
#include "test_instruction.h"
#include "test_utility.h"
#include "test_expr.h"
#include "test_parser.h"
#include "test_interpreter.h"
#include "test_environment.h"
#include "test_frontend.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
extern "C" {
#include <cmocka.h>
}
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

extern "C" {
    int main(int argc, char **argv) {
        const struct CMUnitTest tests[] = {
            // utility
            cmocka_unit_test(test_unescape),
            // scanner
            cmocka_unit_test(test_scanner),
            cmocka_unit_test(test_scannerIsAlphaNumeric),

            // token
            cmocka_unit_test(test_token),

            // object
            cmocka_unit_test(test_object),

            // instruction
            cmocka_unit_test(test_instruction),

            // expr
            cmocka_unit_test(test_expr),

            // parser
            cmocka_unit_test(test_parser),

            // interpreter
            cmocka_unit_test(test_interpreter),
            cmocka_unit_test(test_interpreter_errors),
            cmocka_unit_test(test_misc_interpreter),

            // environment
            cmocka_unit_test(test_environment),

            // frontend
            cmocka_unit_test(test_frontend),
            cmocka_unit_test(test_frontend_errors)
        };
        return cmocka_run_group_tests(tests, NULL, NULL);
    }
}
