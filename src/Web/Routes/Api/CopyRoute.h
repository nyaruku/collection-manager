#pragma once
#include <crow.h>
#include <Web/WebState.h>
#include <algorithm>
#include <unordered_set>

namespace Web::Routes::Api {

    inline void registerCopy(crow::SimpleApp& app) {

        // Copy a collection (or specific beatmaps) from one mode to the other.
        CROW_ROUTE(app, "/api/collections/copy")
        .methods(crow::HTTPMethod::POST)
        ([](const crow::request& req) {
            auto body = nlohmann::json::parse(req.body, nullptr, false);
            if (body.is_discarded() || !body.contains("from") || !body.contains("to") || !body.contains("collection"))
                return crow::response(400, "Missing fields");

            std::string from = body["from"];
            std::string to = body["to"];
            std::string collName = body["collection"];
            std::string target = body.value("targetCollection", collName);

            auto& fromCols = (from == "stable") ? Web::WebHandler::stableCollections : Web::WebHandler::lazerCollections;
            auto& toCols = (to == "stable") ? Web::WebHandler::stableCollections : Web::WebHandler::lazerCollections;

            auto fromIt = std::ranges::find_if(fromCols, [&](const auto& c) { return c.name == collName; });
            if (fromIt == fromCols.end())
                return crow::response(404, "Source collection not found");

            // Determine hashes to copy
            std::vector<std::string> hashesToCopy;
            if (body.contains("hashes") && body["hashes"].is_array() && !body["hashes"].empty()) {
                for (const auto& h : body["hashes"]) hashesToCopy.push_back(h);
            } else {
                hashesToCopy = fromIt->hashes;
            }

            // Find or create target collection
            auto toIt = std::ranges::find_if(toCols, [&](const auto& c) { return c.name == target; });
            if (toIt == toCols.end()) {
                models::Collection newCol; newCol.name = target;
                toCols.push_back(std::move(newCol));
                toIt = std::prev(toCols.end());
            }

            std::unordered_set<std::string> existing(toIt->hashes.begin(), toIt->hashes.end());
            for (const auto& hash : hashesToCopy) {
                if (!existing.insert(hash).second) continue;
                toIt->hashes.push_back(hash);
                auto bIt = std::ranges::find_if(fromIt->beatmaps, [&](const auto& b) { return b.md5 == hash; });
                if (bIt != fromIt->beatmaps.end())
                    toIt->beatmaps.push_back(*bIt);
                else {
                    models::Beatmap missing; missing.md5 = hash;
                    toIt->beatmaps.push_back(std::move(missing));
                }
            }

            return Web::WebHandler::jsonResponse({{"status", "ok"}, {"copied", hashesToCopy.size()}});
        });
    }
}

