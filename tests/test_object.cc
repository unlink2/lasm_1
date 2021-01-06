#include "object.h"
#include "error.h"
#include "test_object.h"


void test_object(void **state) {
    lasm::LasmObject num(lasm::NUMBER_O, lasm::lasmNumber(1234));

    assert_true(num.isNumber());
    assert_false(num.isNil());
    assert_false(num.isReal());
    assert_false(num.isString());
    assert_false(num.isBool());

    assert_int_equal(num.castTo<lasm::lasmNumber>(), 1234);

    // typecasts

    assert_int_equal(num.toNumber(), 1234);
    assert_float_equal(num.toReal(), 1234.0, 0.001);

    lasm::LasmObject str(lasm::STRING_O, "Test");
    assert_throws(lasm::LasmTypeError, {
        str.toNumber();
    });
    assert_throws(lasm::LasmTypeError, {
        str.toReal();
    });
}
