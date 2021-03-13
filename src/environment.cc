#include "environment.h"

namespace lasm {
    void Environment::define(std::string name, LasmObject &value) {
        values[name] = std::make_shared<LasmObject>(LasmObject(value));
    }

    std::shared_ptr<LasmObject> Environment::get(std::shared_ptr<Token> name) {
        auto it = values.find(name->getLexeme());
        if (it == values.end()) {
            if (parent.get()) {
                return parent->get(name);
            }
            throw LasmUndefinedReference(name);
        }

        return it->second;
    }

    void Environment::assign(std::shared_ptr<Token> name, LasmObject &value) {
        auto it = values.find(name->getLexeme());
        if (it == values.end()) {
            if (parent.get()) {
                parent->assign(name, value);
                return;
            }
            throw LasmUndefinedReference(name);
        }
        define(name->getLexeme(), value);
    }

    void Environment::clear() {
        values.clear();
    }
}
