#include "instruction.h"
#include "test_instruction.h"

#include <iostream>
#include <map>

void test_instruction(void **state) {
    lasm::BaseInstructionSet set;
    set.addInstruction("test", new lasm::InstructionParser());

    auto it = set.getInstructions().begin();
    assert_false(it == set.getInstructions().end());
    assert_cc_string_equal(it->first, std::string("test"));
}

