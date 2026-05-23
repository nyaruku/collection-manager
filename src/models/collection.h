#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace models {
    struct Beatmap {
        std::string md5;
        std::string title;
        std::string artist;
        std::string difficulty;
        std::string mapper;
        double starRating = 0.0;
    };

    inline void to_json(nlohmann::json& j, const Beatmap& b) {
        j = {
            {"md5", b.md5},
            {"title", b.title},
            {"artist", b.artist},
            {"difficulty", b.difficulty},
            {"mapper", b.mapper},
            {"stars", b.starRating},
        };
    }

    struct Collection {
        std::string name;
        std::vector<std::string> hashes;
        std::vector<Beatmap> beatmaps;
    };

    inline void to_json(nlohmann::json& j, const Collection& c) {
        j = {{"name", c.name}, {"beatmaps", c.beatmaps}};
    }
}