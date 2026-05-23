#pragma once
#include <functional>
#include <string>
#include <unordered_map>

#include <language/translation.h>
#include <language/english.h>
#include <language/german.h>

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
