#pragma once
#include <crow.h>
#include <Web/WebState.h>
#include <Stable/StableDb.h>
#include <algorithm>
#include <sstream>
#include <unordered_set>

namespace Web::Routes::Api {

    inline auto findStableCollection(const std::string& name) {
        return std::ranges::find_if(Web::WebHandler::stableCollections,
            [&](const auto& c) { return c.name == name; });
    }

    inline void registerManageStable(crow::SimpleApp& app) {

        // Create collection
        CROW_ROUTE(app, "/api/stable/collections/create")
        .methods(crow::HTTPMethod::POST)
        ([](const crow::request& req) {
            auto body = nlohmann::json::parse(req.body, nullptr, false);
            if (body.is_discarded() || !body.contains("name"))
                return crow::response(400, "Missing name");
            std::string name = body["name"];
            if (findStableCollection(name) != Web::WebHandler::stableCollections.end())
                return crow::response(409, "Already exists");
            models::Collection col;
            col.name = name;
            Web::WebHandler::stableCollections.push_back(std::move(col));
            return Web::WebHandler::jsonResponse({{"status", "ok"}});
        });

        // Delete collection
        CROW_ROUTE(app, "/api/stable/collections/delete")
        .methods(crow::HTTPMethod::POST)
        ([](const crow::request& req) {
            auto body = nlohmann::json::parse(req.body, nullptr, false);
            if (body.is_discarded() || !body.contains("name"))
                return crow::response(400, "Missing name");
            auto it = findStableCollection(body["name"]);
            if (it == Web::WebHandler::stableCollections.end())
                return crow::response(404, "Not found");
            Web::WebHandler::stableCollections.erase(it);
            return Web::WebHandler::jsonResponse({{"status", "ok"}});
        });

        // Rename collection
        CROW_ROUTE(app, "/api/stable/collections/rename")
        .methods(crow::HTTPMethod::POST)
        ([](const crow::request& req) {
            auto body = nlohmann::json::parse(req.body, nullptr, false);
            if (body.is_discarded() || !body.contains("name") || !body.contains("newName"))
                return crow::response(400, "Missing name/newName");
            std::string name = body["name"], newName = body["newName"];
            if (findStableCollection(newName) != Web::WebHandler::stableCollections.end())
                return crow::response(409, "Name already exists");
            auto it = findStableCollection(name);
            if (it == Web::WebHandler::stableCollections.end())
                return crow::response(404, "Not found");
            it->name = newName;
            return Web::WebHandler::jsonResponse({{"status", "ok"}});
        });

        // Add beatmaps to collection
        CROW_ROUTE(app, "/api/stable/collections/add-beatmaps")
        .methods(crow::HTTPMethod::POST)
        ([](const crow::request& req) {
            auto body = nlohmann::json::parse(req.body, nullptr, false);
            if (body.is_discarded() || !body.contains("collection") || !body.contains("hashes"))
                return crow::response(400, "Missing fields");
            auto it = findStableCollection(body["collection"]);
            if (it == Web::WebHandler::stableCollections.end())
                return crow::response(404, "Not found");
            std::unordered_set<std::string> existing(it->hashes.begin(), it->hashes.end());
            for (const auto& h : body["hashes"]) {
                std::string hash = h;
                if (!existing.insert(hash).second) continue;
                it->hashes.push_back(hash);
                auto found = Web::WebHandler::stableBeatmapMap.find(hash);
                if (found != Web::WebHandler::stableBeatmapMap.end())
                    it->beatmaps.push_back(found->second);
                else {
                    models::Beatmap missing; missing.md5 = hash;
                    it->beatmaps.push_back(std::move(missing));
                }
            }
            return Web::WebHandler::jsonResponse({{"status", "ok"}});
        });

        // Remove beatmaps from collection
        CROW_ROUTE(app, "/api/stable/collections/remove-beatmaps")
        .methods(crow::HTTPMethod::POST)
        ([](const crow::request& req) {
            auto body = nlohmann::json::parse(req.body, nullptr, false);
            if (body.is_discarded() || !body.contains("collection") || !body.contains("hashes"))
                return crow::response(400, "Missing fields");
            auto it = findStableCollection(body["collection"]);
            if (it == Web::WebHandler::stableCollections.end())
                return crow::response(404, "Not found");
            std::unordered_set<std::string> toRemove;
            for (const auto& h : body["hashes"]) toRemove.insert(std::string(h));
            auto erase = [&](auto& vec, auto pred) { vec.erase(std::remove_if(vec.begin(), vec.end(), pred), vec.end()); };
            erase(it->hashes, [&](const std::string& h) { return toRemove.count(h); });
            erase(it->beatmaps, [&](const models::Beatmap& b) { return toRemove.count(b.md5); });
            return Web::WebHandler::jsonResponse({{"status", "ok"}});
        });

        // Save collection.db to disk
        CROW_ROUTE(app, "/api/stable/save")
        .methods(crow::HTTPMethod::POST)
        ([]() {
            try {
                if (Web::WebHandler::config.stableCollectionDb.empty())
                    return crow::response(400, "No collection.db path configured");
                stable::writeCollectionDb(Web::WebHandler::config.stableCollectionDb, Web::WebHandler::stableCollections);
                return Web::WebHandler::jsonResponse({{"status", "ok"}});
            } catch (const std::exception& e) {
                return crow::response(500, e.what());
            }
        });

        // Import osdb raw file bytes in request body
        CROW_ROUTE(app, "/api/stable/import")
        .methods(crow::HTTPMethod::POST)
        ([](const crow::request& req) {
            try {
                std::istringstream stream(req.body);
                auto imported = stable::parseOsdb(stream);
                auto& cols = Web::WebHandler::stableCollections;
                for (auto& src : imported) {
                    auto it = std::ranges::find_if(cols, [&](const auto& c) { return c.name == src.name; });
                    if (it == cols.end()) {
                    // Re-resolve against osu!.db if available
                    std::vector<models::Collection> tmp = {src};
                    stable::resolveBeatmaps(tmp, Web::WebHandler::stableBeatmapMap);
                    cols.push_back(std::move(tmp[0]));
                    } else {
                        std::unordered_set<std::string> existing(it->hashes.begin(), it->hashes.end());
                        for (auto& hash : src.hashes) {
                            if (!existing.insert(hash).second) continue;
                            it->hashes.push_back(hash);
                            auto found = Web::WebHandler::stableBeatmapMap.find(hash);
                            if (found != Web::WebHandler::stableBeatmapMap.end())
                                it->beatmaps.push_back(found->second);
                            else {
                                models::Beatmap missing; missing.md5 = hash;
                                it->beatmaps.push_back(std::move(missing));
                            }
                        }
                    }
                }
                return Web::WebHandler::jsonResponse({{"status", "ok"}, {"imported", imported.size()}});
            } catch (const std::exception& e) { return crow::response(500, e.what()); }
        });

        // Export selected collections as osdb
        CROW_ROUTE(app, "/api/stable/export")
        .methods(crow::HTTPMethod::POST)
        ([](const crow::request& req) {
            try {
                auto body = nlohmann::json::parse(req.body, nullptr, false);
                if (body.is_discarded() || !body.contains("collections"))
                    return crow::response(400, "Missing collections");
                std::vector<models::Collection> toExport;
                for (const auto& name : body["collections"]) {
                    auto it = findStableCollection(std::string(name));
                    if (it != Web::WebHandler::stableCollections.end())
                        toExport.push_back(*it);
                }
                std::ostringstream out;
                stable::writeOsdb(out, toExport);
                crow::response res(200, out.str());
                res.add_header("Content-Type", "application/octet-stream");
                res.add_header("Content-Disposition", "attachment; filename=\"collections.osdb\"");
                return res;
            } catch (const std::exception& e) { return crow::response(500, e.what()); }
        });
    }
}

