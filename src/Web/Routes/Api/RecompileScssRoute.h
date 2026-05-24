#pragma once
#include <crow.h>
#include <Web/WebState.h>
#include <Web/Scss.h>

namespace Web::Routes::Api {

    inline void registerRecompileScss(crow::SimpleApp& app) {
        CROW_ROUTE(app, "/api/recompile-scss").methods(crow::HTTPMethod::POST)([](const crow::request&) {
            try {
                const std::string& wwwDir = Web::WebHandler::config.wwwDir;
                Web::Scss::compile(wwwDir + "/static/index.scss", wwwDir + "/static/index.css");
                return Web::WebHandler::jsonResponse({{"status", "ok"}});
            } catch (const std::exception& error) {
                return Web::WebHandler::jsonResponse({{"status", "error"}, {"message", error.what()}});
            }
        });
    }
}