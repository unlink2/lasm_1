#ifndef __COLORS_H__
#define __COLORS_H__

#include <iostream>
#include <string>
#include <sstream>
#include <iostream>

namespace lasm {
    // TODO test

    class FormatOutput {
        public:
            FormatOutput(bool enableColor=false):
                enableColor(enableColor) {}

            const std::string reset() const;
            const std::string bold() const;
            const std::string unbold() const;

            const std::string fred() const;
            const std::string fgreen() const;
            const std::string fyellow() const;
            const std::string fblue() const;
            const std::string fmagenta() const;
            const std::string fcyan() const;
            const std::string fwhite() const;
        private:
            const std::string output(const char *fmt) const;
            bool enableColor;
    };

}

#endif
