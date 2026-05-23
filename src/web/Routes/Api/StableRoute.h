#pragma once
#include <crow.h>
#include <web/WebState.h>

namespace Web::Routes::Api {

    inline void registerStable(crow::SimpleApp& app) {
        CROW_ROUTE(app, "/api/stable/collections")([](const crow::request& req) {
            std::string name;
            if (const char* param = req.url_params.get("name")) {
                name = param;
            }

            if (!name.empty()) {
                const auto found = std::ranges::find_if(Web::WebHandler::stableCollections,
                    [&](const auto& collection) { return collection.name == name; });

                if (found != Web::WebHandler::stableCollections.end()) {
                    return Web::WebHandler::jsonResponse(*found);
                }
                return Web::WebHandler::jsonResponse({{"name", name}, {"beatmaps", nlohmann::json::array()}});
            }

            nlohmann::json result = nlohmann::json::array();
            for (const auto& collection : Web::WebHandler::stableCollections) {
                result.push_back({{"name", collection.name}, {"count", collection.hashes.size()}});
            }
            return Web::WebHandler::jsonResponse(result);
        });
    }
}
