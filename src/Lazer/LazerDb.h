#pragma once

#include <Models/Collection.h>

#include <cpprealm/sdk.hpp>

#include <chrono>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct RealmUser {
    int64_t OnlineID;
    std::optional<std::string> Username;
    std::optional<std::string> CountryCode;
};

struct BeatmapCollection {
    realm::primary_key<realm::uuid> ID;
    std::optional<std::string> Name;
    std::vector<std::optional<std::string>> BeatmapMD5Hashes;
    std::chrono::time_point<std::chrono::system_clock> LastModified;
};

struct BeatmapMetadata {
    std::optional<std::string> Title;
    std::optional<std::string> TitleUnicode;
    std::optional<std::string> Artist;
    std::optional<std::string> ArtistUnicode;
    RealmUser* Author;
};

struct BeatmapSet {
    realm::primary_key<realm::uuid> ID;
    int64_t OnlineID = 0;
};

struct Beatmap {
    realm::primary_key<realm::uuid> ID;
    int64_t OnlineID = 0;
    std::optional<std::string> MD5Hash;
    std::optional<std::string> DifficultyName;
    double StarRating;
    BeatmapMetadata* Metadata;
    BeatmapSet* BeatmapSet;
};

namespace realm {
    REALM_SCHEMA(BeatmapSet, ID, OnlineID)
    REALM_EMBEDDED_SCHEMA(RealmUser, OnlineID, Username, CountryCode)
    REALM_SCHEMA(BeatmapCollection, ID, Name, BeatmapMD5Hashes, LastModified)
    REALM_EMBEDDED_SCHEMA(BeatmapMetadata, Title, TitleUnicode, Artist, ArtistUnicode, Author)
    REALM_SCHEMA(Beatmap, ID, OnlineID, MD5Hash, DifficultyName, StarRating, Metadata, BeatmapSet)
}

namespace lazer {
    inline std::vector<models::Collection> loadCollections(const std::string& path) {
        realm::db_config dbConfig;
        dbConfig.set_path(path);
        dbConfig.set_schema_version(46);
        dbConfig.set_schema_mode(realm::db_config::schema_mode::read_only);
        auto database = realm::open<RealmUser, BeatmapCollection, Beatmap, BeatmapMetadata, BeatmapSet>(dbConfig);

        std::unordered_map<std::string, models::Beatmap> beatmapMap;
        for (const auto& entry : database.objects<Beatmap>()) {
            std::string md5 = entry.MD5Hash.detach().value_or("");
            if (md5.empty()) {
                continue;
            }

            models::Beatmap beatmap;
            beatmap.md5 = md5;
            beatmap.beatmapId = static_cast<int32_t>(entry.OnlineID);
            if (entry.BeatmapSet) {
                beatmap.beatmapSetId = static_cast<int32_t>(entry.BeatmapSet->OnlineID);
            }
            beatmap.difficulty = entry.DifficultyName.detach().value_or("");
            beatmap.starRating = entry.StarRating;

            if (entry.Metadata) {
                beatmap.title  = entry.Metadata->Title.detach().value_or("");
                beatmap.artist = entry.Metadata->Artist.detach().value_or("");
                if (entry.Metadata->Author) {
                    beatmap.mapper = entry.Metadata->Author->Username.detach().value_or("");
                }
            }

            beatmapMap[md5] = std::move(beatmap);
        }

        std::vector<models::Collection> collections;
        for (const auto& entry : database.objects<BeatmapCollection>()) {
            models::Collection collection;
            collection.name = entry.Name.detach().value_or("(unnamed)");

            for (const auto& hash : entry.BeatmapMD5Hashes.detach()) {
                if (!hash) {
                    continue;
                }
                collection.hashes.push_back(*hash);

                if (auto found = beatmapMap.find(*hash); found != beatmapMap.end()) {
                    collection.beatmaps.push_back(found->second);
                } else {
                    models::Beatmap missing;
                    missing.md5 = *hash;
                    collection.beatmaps.push_back(std::move(missing));
                }
            }
            collections.push_back(std::move(collection));
        }
        return collections;
    }
}