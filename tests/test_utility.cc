#include "utility.h"
#include "macros.h"

void test_unescape(void **state) {
    std::string unescaped = lasm::unescape("Hello \\\"World\\\"\\nTHis.\\tIs\\nAn\\vEscaped\\rString!\\\\");
    assert_cc_string_equal(unescaped, std::string("Hello \"World\"\nTHis.\tIs\nAn\vEscaped\rString!\\"));
}
