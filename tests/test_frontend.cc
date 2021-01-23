#include "test_frontend.h"

#include "frontend.h"
#include "macros.h"
#include "instruction6502.h"

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

#define test_full(code, lst, ...) {\
    auto reader = DummyReader(code);\
    auto writer = DummyWriter();\
    InstructionSet6502 instructions;\
    Frontend frontend(instructions, reader, writer);\
    frontend.assemble("test.asm", "test.bin", "test.lst");\
    char dataArray[] = __VA_ARGS__;\
    assert_cc_string_equal(writer.list->str(), std::string(lst));\
    assert_memory_equal(dataArray, writer.bin->str().c_str(), writer.bin->str().length());\
}

void test_frontend(void **state) {
    test_full("adc #0xFF;\n"
            "test: let j = 20;"
            "let i = 100;\n"
            "cmp i;",

            "test = 0x2\n"
            "i = 0x64\n"
            "j = 0x14\n",
            {0x69, (char)0xFF, (char)0xC5, (char)0x64});


    // test include and incbin
    test_full("org 0x8000; nop; include \"inc.asm\"\nnop;\nincbin \"inc.bin\"\nnop;",
            "included_label = 0x8003\n",
            {(char)0xEA, (char)0xA9, (char)0xFF, (char)0xEA, (char)0xEA,
            'H', 'e', 'l', 'l', 'o', (char)0xEA});
}
