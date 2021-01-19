#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <iostream>

#define LO(n) ((n >> 8) & 0xFF)
#define HI(n) (n & 0xFF)

namespace lasm {
    char unescapeChar(std::string str, bool &didEscape, unsigned long index=0);
    std::string unescape(std::string src);
}

#endif
