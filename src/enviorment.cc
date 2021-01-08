#include "enviorment.h"

namespace lasm {
    void Enviorment::define(std::string name, LasmObject &value) {
        values[name] = std::make_shared<LasmObject>(LasmObject(value));
    }

    std::shared_ptr<LasmObject> Enviorment::get(std::shared_ptr<Token> name) {
        auto it = values.find(name->getLexeme());

        if (it == values.end()) {
            throw LasmUndefinedReference(name);
        }

        return it->second;
    }
}
