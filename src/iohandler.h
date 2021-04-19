#ifndef __IO_HANDLER_H__
#define __IO_HANDLER_H__

namespace lasm {
    class IOHandler {
        public:
            IOHandler() {}
            ~IOHandler() {}

            virtual void changeDir(std::string path, bool hasFilename=false) {
            }

            virtual std::string getDir() {
                return "";
            }
    };
}

#endif 
