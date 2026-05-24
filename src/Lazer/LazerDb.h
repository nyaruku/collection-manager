#pragma once

#include <Models/Collection.h>

#include <cpprealm/sdk.hpp>

#include <array>
#include <chrono>
#include <cstdio>
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

    // Generate a random UUID v4 using /dev/urandom
    inline realm::uuid generateUuid() {
        std::array<uint8_t, 16> b{};
        if (FILE* f = fopen("/dev/urandom", "rb")) {
            fread(b.data(), 1, 16, f);
            fclose(f);
        }
        b[6] = (b[6] & 0x0f) | 0x40;
        b[8] = (b[8] & 0x3f) | 0x80;
        char s[37];
        snprintf(s, sizeof(s),
            "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            b[0],b[1],b[2],b[3], b[4],b[5], b[6],b[7],
            b[8],b[9], b[10],b[11],b[12],b[13],b[14],b[15]);
        return realm::uuid(s);
    }

    // Save collections back to lazer's client.realm.
    // Uses additive_discovered mode so undeclared tables are preserved.
    inline void saveCollections(const std::string& path, const std::vector<models::Collection>& collections) {
        realm::db_config dbConfig;
        dbConfig.set_path(path);
        dbConfig.set_schema_version(46);
        dbConfig.set_schema_mode(realm::db_config::schema_mode::additive_discovered);
        auto db = realm::open<BeatmapCollection>(dbConfig);

        db.write([&]() {
            auto existing = db.objects<BeatmapCollection>();
            size_t count = existing.size();
            for (size_t i = 0; i < count; ++i) {
                auto obj = existing[0];
                db.remove(obj);
            }

            for (const auto& col : collections) {
                BeatmapCollection entry;
                entry.ID            = generateUuid();
                entry.Name          = col.name;
                entry.LastModified  = std::chrono::system_clock::now();
                std::vector<std::optional<std::string>> hashes;
                hashes.reserve(col.hashes.size());
                for (const auto& h : col.hashes) hashes.push_back(h);
                entry.BeatmapMD5Hashes = std::move(hashes);
                db.add(std::move(entry));
            }
        });
    }
}