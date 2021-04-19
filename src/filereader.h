#ifndef __FILEREADER_H__
#define __FILEREADER_H__

#include <iostream>
#include <sstream>
#include <memory>
#include <cstring>
#include "iohandler.h"

namespace lasm {
    class FileReader: public IOHandler {
        public:
            virtual std::shared_ptr<std::istream> openFile(std::string fromPath) {
                return std::make_shared<std::istringstream>(std::istringstream(std::string("")));
            }

            virtual std::shared_ptr<char[]> readFullFile(std::shared_ptr<std::istream> is, unsigned long *size=nullptr) {
                // get length of file:
                is->seekg (0, is->end);
                int length = is->tellg();
                is->seekg (0, is->beg);

                auto buffer = std::shared_ptr<char[]>(new char[length+1]);
                memset(buffer.get(), 0x00, length+1);
                is->read(buffer.get(), length);
                buffer[length] = '\0';

                if (size) {
                    *size = length;
                }

                return buffer;
            }

            virtual void closeFile(std::shared_ptr<std::istream> stream) { }
    };
}

#endif 
