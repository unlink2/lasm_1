#ifndef __INSTRUCTION65C816_H__
#define __INSTRUCTION65C816_H__

#include "instruction6502.h"
#include "instruction.h"
#include "types.h"
#include "token.h"


namespace lasm {
    /**
     * 65c816
     */

    class InstructionSet65c816: public InstructionSet6502 {
        public:
            InstructionSet65c816();
    };
}

#endif 
