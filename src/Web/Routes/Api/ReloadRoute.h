#pragma once
#include <crow.h>
#include <Web/WebState.h>

namespace Web::Routes::Api {

    inline void registerReload(crow::SimpleApp& app) {
        CROW_ROUTE(app, "/api/reload").methods(crow::HTTPMethod::POST)([](const crow::request&) {
            Web::WebHandler::loadStable();
            Web::WebHandler::loadLazer();
            return Web::WebHandler::jsonResponse({{"status", "reloaded"}});
        });
    }
}
