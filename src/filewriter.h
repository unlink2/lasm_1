#ifndef __FILEWRITER_H__
#define __FILEWRITER_H__

#include <iostream>
#include <vector>
#include "instruction.h"
#include <memory>
#include <sstream>

namespace lasm {
    class FileWriter {
        public:
            virtual std::shared_ptr<std::ostream> openFile(std::string fromPath) {
                return std::make_shared<std::ostringstream>(std::ostringstream());
            }
            virtual void closeFile(std::shared_ptr<std::ostream> stream) { }
    };
};

#endif 
