#pragma once
#include <Web/Routes/Pages/IndexRoute.h>
#include <Web/Routes/Api/StableRoute.h>
#include <Web/Routes/Api/LazerRoute.h>
#include <Web/Routes/Api/ReloadRoute.h>
#include <Web/Routes/Api/RecompileScssRoute.h>
#include <Web/Routes/Api/ManageStableRoute.h>
#include <Web/Routes/Api/ManageLazerRoute.h>
#include <Web/Routes/Api/CopyRoute.h>
#include <Web/Routes/Api/BackupRoute.h>

namespace Web::RouteRegister {

    inline void registerAll(crow::SimpleApp& app) {
        Web::Routes::Pages::registerIndex(app);
        Web::Routes::Api::registerStable(app);
        Web::Routes::Api::registerLazer(app);
        Web::Routes::Api::registerReload(app);
        Web::Routes::Api::registerRecompileScss(app);
        Web::Routes::Api::registerManageStable(app);
        Web::Routes::Api::registerManageLazer(app);
        Web::Routes::Api::registerCopy(app);
        Web::Routes::Api::registerBackup(app);
    }
}
