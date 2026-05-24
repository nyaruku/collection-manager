#pragma once
#include <crow.h>
#include <Web/WebState.h>
#include <Web/Layout.h>

namespace Web::Routes::Pages {

    inline void registerIndex(crow::SimpleApp& app) {
        CROW_ROUTE(app, "/")([]() {
            return Web::Layout::render();
        });
    }
}
