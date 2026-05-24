#pragma once
#include <Web/Routes/Pages/IndexRoute.h>
#include <Web/Routes/Api/StableRoute.h>
#include <Web/Routes/Api/LazerRoute.h>
#include <Web/Routes/Api/ReloadRoute.h>
#include <Web/Routes/Api/RecompileScssRoute.h>

namespace Web::RouteRegister {

    inline void registerAll(crow::SimpleApp& app) {
        Web::Routes::Pages::registerIndex(app);
        Web::Routes::Api::registerStable(app);
        Web::Routes::Api::registerLazer(app);
        Web::Routes::Api::registerReload(app);
        Web::Routes::Api::registerRecompileScss(app);
    }
}

