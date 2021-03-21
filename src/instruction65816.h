#ifndef __INSTRUCTION65C816_H__
#define __INSTRUCTION65C816_H__

#include "instruction6502.h"
#include "instruction.h"
#include "types.h"
#include "token.h"


namespace lasm {
    /**
     * 65816
     */

    class Set16BitDirective65816: public Directive {
        public:
            virtual std::shared_ptr<Stmt> parse(Parser *parser);
            virtual std::any execute(Interpreter *interpreter, DirectiveStmt *stmt);
    };

    class Set8BitDirective85816: public Directive {
        public:
            virtual std::shared_ptr<Stmt> parse(Parser *parser);
            virtual std::any execute(Interpreter *interpreter, DirectiveStmt *stmt);
    };

    class InstructionSet65816: public InstructionSet6502 {
        public:
            InstructionSet65816();
    };
}

#endif 
