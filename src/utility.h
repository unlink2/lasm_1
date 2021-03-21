#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <iostream>
#include <cmath>

#define LO(n, bits) ((n >> bits) & ((unsigned int)std::pow(2, bits)-1))
#define HI(n, bits) (n & ((unsigned int)std::pow(2, bits)-1))

namespace lasm {
    char unescapeChar(std::string str, bool &didEscape, unsigned long index=0);
    std::string unescape(std::string src);
}

#endif
