#pragma once

#include <crow/mustache.h>
#include <Language/Translation.h>

namespace Web {
    static void applyTranslations(crow::mustache::context& ctx, const Language::Translation& translation) {
        ctx["app_name"] = translation.appName;
        ctx["menu_file"] = translation.menuFile;
        ctx["menu_file_reload"] = translation.menuFileReload;
        ctx["menu_file_exit"] = translation.menuFileExit;
        ctx["menu_view"] = translation.menuView;
        ctx["menu_view_stable"] = translation.menuViewStable;
        ctx["menu_view_lazer"] = translation.menuViewLazer;
        ctx["menu_view_hide_unknown"] = translation.menuViewHideUnknown;
        ctx["menu_help"] = translation.menuHelp;
        ctx["menu_help_about"] = translation.menuHelpAbout;
        ctx["tab_stable"] = translation.tabStable;
        ctx["tab_lazer"] = translation.tabLazer;
        ctx["status_ready"] = translation.statusReady;
        ctx["about_dialog_title"] = translation.aboutDialogTitle;
        ctx["about_version"] = translation.aboutVersion;
        ctx["about_desc"] = translation.aboutDesc;
        ctx["about_formats"] = translation.aboutFormats;
        ctx["pane_collections_label"] = translation.paneCollectionsLabel;
        ctx["pane_loading"] = translation.paneLoading;
        ctx["pane_select_prompt"] = translation.paneSelectPrompt;
    }
}
