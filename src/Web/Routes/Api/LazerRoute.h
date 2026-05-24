#pragma once
#include <crow.h>
#include <Web/WebState.h>

namespace Web::Routes::Api {

    inline void registerLazer(crow::SimpleApp& app) {
        CROW_ROUTE(app, "/api/lazer/collections")([](const crow::request& req) {
            std::string name;
            if (const char* param = req.url_params.get("name")) {
                name = param;
            }

            if (!name.empty()) {
                const auto found = std::ranges::find_if(Web::WebHandler::lazerCollections,
                    [&](const auto& collection) { return collection.name == name; });

                if (found != Web::WebHandler::lazerCollections.end()) {
                    return Web::WebHandler::jsonResponse(*found);
                }
                return Web::WebHandler::jsonResponse({{"name", name}, {"beatmaps", nlohmann::json::array()}});
            }

            nlohmann::json result = nlohmann::json::array();
            for (const auto& collection : Web::WebHandler::lazerCollections) {
                result.push_back({{"name", collection.name}, {"count", collection.hashes.size()}});
            }
            return Web::WebHandler::jsonResponse(result);
        });
    }
}
