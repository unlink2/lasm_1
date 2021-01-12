#ifndef __FILEREADER_H__
#define __FILEREADER_H__

#include <iostream>
#include <sstream>
#include <memory>

namespace lasm {
    class FileReader {
        public:
            virtual std::shared_ptr<std::istream> openFile(std::string fromPath) {
                return std::make_shared<std::istringstream>(std::istringstream(std::string("")));
            }
            virtual void closeFile(std::shared_ptr<std::istream> stream) { }
    };
}

#endif 
