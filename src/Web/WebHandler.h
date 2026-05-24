#pragma once
#include <crow.h>
#include <Web/WebState.h>
#include <Web/RouteRegister.h>
#include <Web/Scss.h>

namespace Web::WebHandler {

    static void start(Config startConfig) {
        config = std::move(startConfig);
        try {
            Web::Scss::compile(config.wwwDir + "/static/index.scss", config.wwwDir + "/static/index.css");
        } catch (const std::exception& error) {
            CROW_LOG_WARNING << "SCSS compilation failed: " << error.what();
        }
        loadStable();
        loadLazer();
        app.loglevel(crow::LogLevel::Warning);
        Web::RouteRegister::registerAll(app);
        app.add_static_dir();
        app.bindaddr(config.host).port(config.port).run();
    }
}
