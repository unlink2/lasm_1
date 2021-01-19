#include "instruction.h"
#include "test_instruction.h"

#include <iostream>
#include <map>

void test_instruction(void **state) {
    lasm::BaseInstructionSet set;
    set.addInstruction("test", std::make_shared<lasm::InstructionParser>(lasm::InstructionParser()));

    assert_false(set.isInstruction("test1"));
    assert_true(set.isInstruction("test"));
}

