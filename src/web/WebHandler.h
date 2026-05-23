#pragma once
#include <crow.h>
#include <web/WebState.h>
#include <web/RouteRegister.h>

namespace Web::WebHandler {

    static void start(Config startConfig) {
        config = std::move(startConfig);
        loadStable();
        loadLazer();
        app.loglevel(crow::LogLevel::Warning);
        Web::RouteRegister::registerAll(app);
        app.add_static_dir();
        app.bindaddr(config.host).port(config.port).run();
    }
}
