#include "test_frontend.h"

#include "frontend.h"
#include "macros.h"
#include "instruction6502.h"
#include "instruction65816.h"

using namespace lasm;

class DummyReader: public FileReader {
    public:
        DummyReader(std::string filecontent):
            content(std::make_shared<std::istringstream>(std::istringstream(filecontent))) {}

        virtual std::shared_ptr<std::istream> openFile(std::string fromPath) {
            if (fromPath == "inc.asm") {
                return std::make_shared<std::istringstream>(std::istringstream("lda #0xFF;\nincluded_label:\nnop;"));
            } else if (fromPath == "inc.bin") {
                return std::make_shared<std::istringstream>(std::istringstream("Hello"));
            }
            return content;
        }
    private:
        std::shared_ptr<std::istringstream> content;
};

class DummyWriter: public FileWriter {
    public:
        virtual std::shared_ptr<std::ostream> openFile(std::string fromPath) {
            if (fromPath == "test.lst") {
                return list;
            }

            return bin;
        }
        std::shared_ptr<std::ostringstream> list = std::make_shared<std::ostringstream>(std::ostringstream());
        std::shared_ptr<std::ostringstream> bin = std::make_shared<std::ostringstream>(std::ostringstream());
};

#define test_full(code, lst, is, ...) {\
    auto reader = DummyReader(code);\
    auto writer = DummyWriter();\
    is instructions;\
    Frontend frontend(instructions, reader, writer);\
    frontend.assemble("test.asm", "test.bin", "test.lst");\
    char dataArray[] = __VA_ARGS__;\
    assert_cc_string_equal(writer.list->str(), std::string(lst));\
    assert_memory_equal(dataArray, writer.bin->str().c_str(), writer.bin->str().length());\
}

#define test_full_err(code, is, errorCode) {\
    auto reader = DummyReader(code);\
    auto writer = DummyWriter();\
    is instructions;\
    std::stringstream nopstream;\
    Frontend frontend(instructions, reader, writer, Frontend::defaultSettings, nopstream);\
    assert_int_equal(frontend.assemble("test.asm", "test.bin", "test.lst"), errorCode);\
}

void test_frontend(void **state) {
    test_full("adc #0xFF;\n"
            "test: let j = 20;"
            "let i = 100;\n"
            "cmp i;",

            "test = 0x2\n"
            "i = 0x64\n"
            "j = 0x14\n",
            InstructionSet6502,
            {0x69, (char)0xFF, (char)0xC5, (char)0x64});


    // test include and incbin
    test_full("org 0x8000; nop; include \"inc.asm\"\nnop;\nincbin \"inc.bin\"\nnop; db ord('a'), len(\"Hello\"),"
            "len([1, 2, 3]);",
            "included_label = 0x8003\n",
            InstructionSet6502,
            {(char)0xEA, (char)0xA9, (char)0xFF, (char)0xEA, (char)0xEA,
            'H', 'e', 'l', 'l', 'o', (char)0xEA, 'a', 0x05, 0x03});

    // test label names
    test_full("org 0x8000;\n"
            "scope1: {\n"
            "setScopeName(\"scopeName\");"
                "sublabel: {\nnop;\n"
                "}\n}",
            "scope1 = 0x8000\nscopeName.sublabel = 0x8000\n",
            InstructionSet6502,
            {(char)0xEA});

    // test 65816 immediate16, long and long, x
    test_full("m16; adc #0xFFFF;\n"
            "adc 0x1FFFF;"
            "adc 0x1FFAA, x;",

            "",
            InstructionSet65816,
            {0x69, (char)0xFF, (char)0xFF,
            0x6F, (char)0xFF, (char)0xFF, (char)0x01,
            0x7F, (char)0xAA, (char)0xFF, (char)0x01});

}

void test_frontend_errors(void **state) {
    // test errors
    test_full_err("noo;", InstructionSet6502, UNDEFINED_REF);
}
