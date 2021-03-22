#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <iostream>
#include <cmath>

#define RDBYTE(n, pos, bits) ((n >> (bits * pos)) & ((unsigned int)std::pow(2, bits)-1))
#define LO(n, bits) RDBYTE(n, 1, bits)
#define HI(n, bits) RDBYTE(n, 0, bits)

namespace lasm {
    char unescapeChar(std::string str, bool &didEscape, unsigned long index=0);
    std::string unescape(std::string src);
}

#endif
