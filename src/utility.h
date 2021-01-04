#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <iostream>

namespace lasm {
    char unescapeChar(std::string str, bool &didEscape, unsigned long index=0);
    std::string unescape(std::string src);
}

#endif
