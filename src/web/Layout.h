#pragma once
#include <crow.h>
#include <web/WebState.h>
#include <web/i18n.h>

namespace Web::Layout {
    inline crow::response render() {
        crow::mustache::set_base(Web::WebHandler::config.wwwDir + "/templates/");

        crow::mustache::context stableContext;
        stableContext["mode"] = "stable";
        stableContext["active"] = "show active";
        Web::applyTranslations(stableContext, Web::WebHandler::config.translation);

        crow::mustache::context lazerContext;
        lazerContext["mode"] = "lazer";
        lazerContext["active"] = "";
        Web::applyTranslations(lazerContext, Web::WebHandler::config.translation);

        crow::mustache::context navbarContext;
        Web::applyTranslations(navbarContext, Web::WebHandler::config.translation);

        crow::mustache::context aboutContext;
        Web::applyTranslations(aboutContext, Web::WebHandler::config.translation);

        crow::mustache::context ctx;
        ctx["navbar"] = crow::mustache::load("Partials/Navbar.html").render(navbarContext).body_;
        ctx["stable_pane"] = crow::mustache::load("Partials/SourcePane.html").render(stableContext).body_;
        ctx["lazer_pane"] = crow::mustache::load("Partials/SourcePane.html").render(lazerContext).body_;
        ctx["about_dialog"] = crow::mustache::load("Partials/AboutDialog.html").render(aboutContext).body_;
        Web::applyTranslations(ctx, Web::WebHandler::config.translation);

        crow::mustache::context layoutCtx;
        layoutCtx["content"] = crow::mustache::load("index.html").render(ctx).body_;
        Web::applyTranslations(layoutCtx, Web::WebHandler::config.translation);

        return crow::response(crow::mustache::load("Layouts/Default.html").render(layoutCtx));
    }
}
