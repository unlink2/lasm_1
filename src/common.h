#ifndef __COMMON_H__
#define __COMMON_H__

#include "error.h"
#include "instruction.h"

namespace lasm {
    class LasmCommon {
        public:
            LasmCommon(BaseError &error, BaseInstructionSet &instructions):
                error(error), instructions(instructions) {}
        protected:
            BaseError &error;
            BaseInstructionSet instructions;
    };
}

#endif 
