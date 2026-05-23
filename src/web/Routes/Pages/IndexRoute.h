#pragma once
#include <crow.h>
#include <web/WebState.h>
#include <web/Layout.h>

namespace Web::Routes::Pages {

    inline void registerIndex(crow::SimpleApp& app) {
        CROW_ROUTE(app, "/")([]() {
            return Web::Layout::render();
        });
    }
}
