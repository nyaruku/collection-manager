#pragma once
#include <crow.h>
#include <Web/WebState.h>

#include <chrono>
#include <filesystem>
#include <string>

namespace Web::Routes::Api {

    inline std::string backupFile(const std::string& source) {
        namespace fs = std::filesystem;
        if (source.empty()) throw std::runtime_error("No source file configured");

        fs::path sourcePath(source);
        if (!fs::exists(sourcePath)) throw std::runtime_error("Source file does not exist: " + source);

        const auto now = std::chrono::system_clock::now().time_since_epoch();
        const auto stamp = std::chrono::duration_cast<std::chrono::seconds>(now).count();

        fs::path backupPath;
        for (int i = 0; ; ++i) {
            backupPath = sourcePath;
            backupPath += ".bak-" + std::to_string(stamp) + (i ? "-" + std::to_string(i) : "");
            if (!fs::exists(backupPath)) break;
        }

        fs::copy_file(sourcePath, backupPath);
        return backupPath.string();
    }

    inline void registerBackup(crow::SimpleApp& app) {
        CROW_ROUTE(app, "/api/<string>/backup")
        .methods(crow::HTTPMethod::POST)
        ([](const std::string& mode) {
            try {
                std::string source;
                if (mode == "stable") source = Web::WebHandler::config.stableCollectionDb;
                else if (mode == "lazer") source = Web::WebHandler::config.lazerRealm;
                else return crow::response(400, "Unknown mode");

                return Web::WebHandler::jsonResponse({
                    {"status", "ok"},
                    {"path", backupFile(source)},
                });
            } catch (const std::exception& e) {
                return crow::response(500, e.what());
            }
        });
    }
}

