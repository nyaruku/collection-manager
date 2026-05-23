#pragma once
#include <web/Routes/Pages/IndexRoute.h>
#include <web/Routes/Api/StableRoute.h>
#include <web/Routes/Api/LazerRoute.h>
#include <web/Routes/Api/ReloadRoute.h>

namespace Web::RouteRegister {

    inline void registerAll(crow::SimpleApp& app) {
        Web::Routes::Pages::registerIndex(app);
        Web::Routes::Api::registerStable(app);
        Web::Routes::Api::registerLazer(app);
        Web::Routes::Api::registerReload(app);
    }
}

