#pragma once
#include <crow.h>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <Models/Collection.h>
#include <Language/Registry.h>
#include <Stable/StableDb.h>
#include <Lazer/LazerDb.h>

namespace Web::WebHandler {

    struct Config {
        std::string wwwDir;
        std::string stableOsuDb;
        std::string stableCollectionDb;
        std::string lazerRealm;
        std::string host = COLLECTION_MANAGER_HOST;
        uint16_t port = COLLECTION_MANAGER_PORT;
        Language::Translation translation;
    };

    inline Config config;
    inline crow::SimpleApp app;

    inline std::vector<models::Collection> stableCollections;
    inline std::vector<models::Collection> lazerCollections;
    inline std::unordered_map<std::string, models::Beatmap> stableBeatmapMap;

    static crow::response jsonResponse(nlohmann::json body) {
        crow::response response(body.dump());
        response.add_header("Content-Type", "application/json");
        return response;
    }

    static void loadStable() {
        if (config.stableCollectionDb.empty()) {
            return;
        }
        try {
            stableCollections = stable::parseCollectionDb(config.stableCollectionDb);
            if (!config.stableOsuDb.empty()) {
                stableBeatmapMap = stable::parseOsuDb(config.stableOsuDb);
                stable::resolveBeatmaps(stableCollections, stableBeatmapMap);
            }
        } catch (const std::exception& error) {
            CROW_LOG_WARNING << "Failed loading stable data: " << error.what();
        }
    }

    static void loadLazer() {
        if (config.lazerRealm.empty()) {
            return;
        }
        try {
            lazerCollections = lazer::loadCollections(config.lazerRealm);
        } catch (const std::exception& error) {
            CROW_LOG_WARNING << "Failed loading lazer data: " << error.what();
        }
    }
}
