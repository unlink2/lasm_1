#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

#include <iostream>
#include <map>
#include <memory>
#include "object.h"

//TODO test
namespace lasm {
    class Parser;
    class Interpreter;
    class Stmt;
    class InstructionStmt;
    class InstructionInfo;
    class Token;

    /**
     * Final parsed instruction
     */
    class InstructionResult {
        public:
            InstructionResult(std::shared_ptr<char[]> data=std::shared_ptr<char[]>(nullptr), unsigned long size=0,
                    unsigned long address=0, std::shared_ptr<Token> name=std::shared_ptr<Token>(nullptr)):
                data(data), size(size), address(address) {}

            std::shared_ptr<char[]> getData() { return data; }
            unsigned long getSize() { return size; }
            unsigned long getAddress() { return address; }
            std::shared_ptr<Token> getName() { return name; }
        private:
            std::shared_ptr<char[]> data;
            unsigned long size;
            unsigned long address;
            std::shared_ptr<Token> name;
    };

    /**
     * Base class maps instruction
     */
    class InstructionParser {
        public:
            virtual std::shared_ptr<Stmt> parse(Parser *parser) { return std::shared_ptr<Stmt>(nullptr); }
    };

    /**
     * Generates actual code
     */
    class InstructionGenerator {
        public:
            virtual InstructionResult generate(Interpreter *interpreter,
                    std::shared_ptr<InstructionInfo> info,
                    InstructionStmt *stmt) {
                return InstructionResult();
            }
    };

    /**
     * Information for the stmt node once parsing finished
     */
    class InstructionInfo {
        public:
            InstructionInfo(unsigned long opcode, unsigned long size, std::shared_ptr<InstructionGenerator> generator):
                opcode(opcode), size(size), generator(generator) {}

            unsigned long getSize() { return size; }
            unsigned long getOpcode() { return opcode; }
            std::shared_ptr<InstructionGenerator> getGenerator() { return generator; }
        private:
            unsigned short opcode;
            unsigned short size;
            std::shared_ptr<InstructionGenerator> generator;
    };

    /**
     * Base class for any arch
     */
    class BaseInstructionSet {
        public:
            std::map<std::string, std::shared_ptr<InstructionParser>>& getInstructions() {
                return instructions;
            }
            void addInstruction(std::string name, InstructionParser *parser);

            virtual std::shared_ptr<Stmt> parse(Parser *parser) { return std::shared_ptr<Stmt>(nullptr); }
            virtual InstructionResult generate(Interpreter *interpreter,
                    std::shared_ptr<InstructionInfo> info,
                    InstructionStmt *stmt) {
                return InstructionResult();
            }
        protected:
            std::map<std::string, std::shared_ptr<InstructionParser>> instructions;
    };
}

#endif
