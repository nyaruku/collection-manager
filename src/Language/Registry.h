#pragma once
#include <functional>
#include <string>
#include <unordered_map>

#include <Language/Translation.h>
#include <Language/English.h>
#include <Language/German.h>

namespace Language {
    inline const std::unordered_map<std::string, std::function<Translation()>> languages = {
        {"en", english}
        ,{"de", german}
    };

    inline Translation load(const std::string& code) {
        if (const auto iterator = languages.find(code); iterator != languages.end()) {
            return iterator->second();
        }
        return english();
    }
}
