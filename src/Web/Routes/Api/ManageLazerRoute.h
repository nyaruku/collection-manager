#pragma once
#include <crow.h>
#include <Web/WebState.h>
#include <Lazer/LazerDb.h>
#include <Stable/StableDb.h>
#include <algorithm>
#include <sstream>
#include <unordered_set>

namespace Web::Routes::Api {

    inline auto findLazerCollection(const std::string& name) {
        return std::ranges::find_if(Web::WebHandler::lazerCollections,
            [&](const auto& c) { return c.name == name; });
    }

    inline void registerManageLazer(crow::SimpleApp& app) {

        // Create collection
        CROW_ROUTE(app, "/api/lazer/collections/create")
        .methods(crow::HTTPMethod::POST)
        ([](const crow::request& req) {
            auto body = nlohmann::json::parse(req.body, nullptr, false);
            if (body.is_discarded() || !body.contains("name"))
                return crow::response(400, "Missing name");
            std::string name = body["name"];
            if (findLazerCollection(name) != Web::WebHandler::lazerCollections.end())
                return crow::response(409, "Already exists");
            models::Collection col;
            col.name = name;
            Web::WebHandler::lazerCollections.push_back(std::move(col));
            return Web::WebHandler::jsonResponse({{"status", "ok"}});
        });

        // Delete collection
        CROW_ROUTE(app, "/api/lazer/collections/delete")
        .methods(crow::HTTPMethod::POST)
        ([](const crow::request& req) {
            auto body = nlohmann::json::parse(req.body, nullptr, false);
            if (body.is_discarded() || !body.contains("name"))
                return crow::response(400, "Missing name");
            auto it = findLazerCollection(body["name"]);
            if (it == Web::WebHandler::lazerCollections.end())
                return crow::response(404, "Not found");
            Web::WebHandler::lazerCollections.erase(it);
            return Web::WebHandler::jsonResponse({{"status", "ok"}});
        });

        // Rename collection
        CROW_ROUTE(app, "/api/lazer/collections/rename")
        .methods(crow::HTTPMethod::POST)
        ([](const crow::request& req) {
            auto body = nlohmann::json::parse(req.body, nullptr, false);
            if (body.is_discarded() || !body.contains("name") || !body.contains("newName"))
                return crow::response(400, "Missing name/newName");
            std::string name = body["name"], newName = body["newName"];
            if (findLazerCollection(newName) != Web::WebHandler::lazerCollections.end())
                return crow::response(409, "Name already exists");
            auto it = findLazerCollection(name);
            if (it == Web::WebHandler::lazerCollections.end())
                return crow::response(404, "Not found");
            it->name = newName;
            return Web::WebHandler::jsonResponse({{"status", "ok"}});
        });

        // Add beatmaps to collection
        CROW_ROUTE(app, "/api/lazer/collections/add-beatmaps")
        .methods(crow::HTTPMethod::POST)
        ([](const crow::request& req) {
            auto body = nlohmann::json::parse(req.body, nullptr, false);
            if (body.is_discarded() || !body.contains("collection") || !body.contains("hashes"))
                return crow::response(400, "Missing fields");
            auto it = findLazerCollection(body["collection"]);
            if (it == Web::WebHandler::lazerCollections.end())
                return crow::response(404, "Not found");
            std::unordered_set<std::string> existing(it->hashes.begin(), it->hashes.end());
            for (const auto& h : body["hashes"]) {
                std::string hash = h;
                if (!existing.insert(hash).second) continue;
                it->hashes.push_back(hash);
                // Beatmap metadata comes from lazer's own realm, add as unknown for now
                models::Beatmap missing; missing.md5 = hash;
                it->beatmaps.push_back(std::move(missing));
            }
            return Web::WebHandler::jsonResponse({{"status", "ok"}});
        });

        // Remove beatmaps from collection
        CROW_ROUTE(app, "/api/lazer/collections/remove-beatmaps")
        .methods(crow::HTTPMethod::POST)
        ([](const crow::request& req) {
            auto body = nlohmann::json::parse(req.body, nullptr, false);
            if (body.is_discarded() || !body.contains("collection") || !body.contains("hashes"))
                return crow::response(400, "Missing fields");
            auto it = findLazerCollection(body["collection"]);
            if (it == Web::WebHandler::lazerCollections.end())
                return crow::response(404, "Not found");
            std::unordered_set<std::string> toRemove;
            for (const auto& h : body["hashes"]) toRemove.insert(std::string(h));
            auto erase = [&](auto& vec, auto pred) { vec.erase(std::remove_if(vec.begin(), vec.end(), pred), vec.end()); };
            erase(it->hashes,   [&](const std::string& h) { return toRemove.count(h); });
            erase(it->beatmaps, [&](const models::Beatmap& b) { return toRemove.count(b.md5); });
            return Web::WebHandler::jsonResponse({{"status", "ok"}});
        });

        // Save to lazer realm
        CROW_ROUTE(app, "/api/lazer/save")
        .methods(crow::HTTPMethod::POST)
        ([]() {
            try {
                if (Web::WebHandler::config.lazerRealm.empty())
                    return crow::response(400, "No lazer realm path configured");

                lazer::saveCollections(Web::WebHandler::config.lazerRealm, Web::WebHandler::lazerCollections);
                return Web::WebHandler::jsonResponse({{"status", "ok"}});
            } catch (const std::exception& e) {
                return crow::response(500, e.what());
            }
        });

        // Import osdb into lazer collections
        CROW_ROUTE(app, "/api/lazer/import")
        .methods(crow::HTTPMethod::POST)
        ([](const crow::request& req) {
            try {
                std::istringstream stream(req.body);
                auto imported = stable::parseOsdb(stream);
                auto& cols = Web::WebHandler::lazerCollections;
                for (auto& src : imported) {
                    auto it = std::ranges::find_if(cols, [&](const auto& c) { return c.name == src.name; });
                    if (it == cols.end()) {
                        cols.push_back(std::move(src));
                    } else {
                        std::unordered_set<std::string> existing(it->hashes.begin(), it->hashes.end());
                        for (auto& hash : src.hashes) {
                            if (!existing.insert(hash).second) continue;
                            it->hashes.push_back(hash);
                            models::Beatmap missing; missing.md5 = hash;
                            it->beatmaps.push_back(std::move(missing));
                        }
                    }
                }
                return Web::WebHandler::jsonResponse({{"status", "ok"}, {"imported", imported.size()}});
            } catch (const std::exception& e) { return crow::response(500, e.what()); }
        });

        // Export selected lazer collections as osdb
        CROW_ROUTE(app, "/api/lazer/export")
        .methods(crow::HTTPMethod::POST)
        ([](const crow::request& req) {
            try {
                auto body = nlohmann::json::parse(req.body, nullptr, false);
                if (body.is_discarded() || !body.contains("collections"))
                    return crow::response(400, "Missing collections");
                std::vector<models::Collection> toExport;
                for (const auto& name : body["collections"]) {
                    auto it = findLazerCollection(std::string(name));
                    if (it != Web::WebHandler::lazerCollections.end())
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

