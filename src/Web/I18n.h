#pragma once

#include <crow/mustache.h>
#include <Language/Translation.h>
#include <nlohmann/json.hpp>

namespace Web {
    inline nlohmann::json jsTranslations(const Language::Translation& t) {
        return {
            {"menuViewHideUnknown", t.menuViewHideUnknown},
            {"menuViewShowUnknown", t.menuViewShowUnknown},
            {"paneLoading", t.paneLoading},
            {"columnTitle", t.columnTitle},
            {"columnArtist", t.columnArtist},
            {"columnDifficulty", t.columnDifficulty},
            {"columnMapper", t.columnMapper},
            {"columnStars", t.columnStars},
            {"columnId", t.columnId},
            {"columnSet", t.columnSet},
            {"columnMd5", t.columnMd5},
            {"noCollections", t.noCollections},
            {"unnamedCollection", t.unnamedCollection},
            {"unknownBeatmapTitle", t.unknownBeatmapTitle},
            {"noBeatmaps", t.noBeatmaps},
            {"mapCount", t.mapCount},
            {"collectionCount", t.collectionCount},
            {"loadingCollection", t.loadingCollection},
            {"collectionStatus", t.collectionStatus},
            {"genericError", t.genericError},
            {"statusReady", t.statusReady},
            {"statusReloading", t.statusReloading},
            {"statusRecompilingScss", t.statusRecompilingScss},
            {"statusScssRecompiled", t.statusScssRecompiled},
            {"statusScssError", t.statusScssError},
            {"unknownError", t.unknownError},
            {"menuRename", t.menuRename},
            {"menuExportOsdb", t.menuExportOsdb},
            {"menuCopyTo", t.menuCopyTo},
            {"menuPasteBeatmaps", t.menuPasteBeatmaps},
            {"menuDelete", t.menuDelete},
            {"menuCopyBeatmaps", t.menuCopyBeatmaps},
            {"menuRemoveFromCollection", t.menuRemoveFromCollection},
            {"clipboardEmpty", t.clipboardEmpty},
            {"copiedBeatmapsFrom", t.copiedBeatmapsFrom},
            {"copiedBeatmapsTo", t.copiedBeatmapsTo},
            {"pastedBeatmapsInto", t.pastedBeatmapsInto},
            {"pasteError", t.pasteError},
            {"promptCollectionName", t.promptCollectionName},
            {"promptNewName", t.promptNewName},
            {"confirmDeleteCollection", t.confirmDeleteCollection},
            {"statusCreatedCollection", t.statusCreatedCollection},
            {"statusDeletedCollection", t.statusDeletedCollection},
            {"statusRenamedCollection", t.statusRenamedCollection},
            {"statusSaving", t.statusSaving},
            {"statusSaved", t.statusSaved},
            {"saveError", t.saveError},
            {"statusBackingUp", t.statusBackingUp},
            {"statusBackupCreated", t.statusBackupCreated},
            {"backupError", t.backupError},
            {"statusImporting", t.statusImporting},
            {"statusImportedCollections", t.statusImportedCollections},
            {"importError", t.importError},
            {"exportError", t.exportError},
        };
    }

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
        ctx["github_link_text"] = translation.githubLinkText;
        ctx["github_link"] = translation.githubLink;
        ctx["menu_file_recompile_scss"] = translation.menuFileRecompileScss;
        ctx["pane_collections_label"] = translation.paneCollectionsLabel;
        ctx["pane_loading"] = translation.paneLoading;
        ctx["pane_select_prompt"] = translation.paneSelectPrompt;
        ctx["pane_action_new"] = translation.paneActionNew;
        ctx["pane_action_import"] = translation.paneActionImport;
        ctx["pane_action_save"] = translation.paneActionSave;
        ctx["pane_action_backup"] = translation.paneActionBackup;
        ctx["js_i18n"] = jsTranslations(translation).dump();
    }
}
