#include "colors.h"

namespace lasm {

    const std::string FormatOutput::output(const char *fmt) const {
        std::stringstream strstream;

        strstream << fmt;

        if (!enableColor) {
            return "";
        }
        return strstream.str();
    }

    const std::string FormatOutput::reset() const {
        return output("\x1B[0m");
    }

    const std::string FormatOutput::bold() const {
        return output("\x1B[1m");
    }

    const std::string FormatOutput::unbold() const {
        return output("\x1B[4m");
    }

    const std::string FormatOutput::fred() const {
        return output("\x1B[31m");
    }

    const std::string FormatOutput::fgreen() const {
        return output("\x1B[32m");
    }

    const std::string FormatOutput::fyellow() const {
        return output("\x1B[33m");
    }

    const std::string FormatOutput::fblue() const {
        return output("\x1B[34m");
    }

    const std::string FormatOutput::fmagenta() const {
        return output("\x1B[35m");
    }

    const std::string FormatOutput::fcyan() const {
        return output("\x1B[36m");
    }

    const std::string FormatOutput::fwhite() const {
        return output("\x1B[37m");
    }
}
