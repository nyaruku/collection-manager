#pragma once
#include <Language/Translation.h>

namespace Language {
    inline Translation english() {
        Translation t;
        t.appName = "osu! Collection Manager";

        t.menuFile = "File";
        t.menuFileReload = "Reload All";
        t.menuFileExit = "Exit";

        t.menuView = "View";
        t.menuViewStable = "Stable";
        t.menuViewLazer = "Lazer";
        t.menuViewHideUnknown = "Hide Unknown Maps";

        t.menuHelp = "Help";
        t.menuHelpAbout = "About";

        t.tabStable = "Stable";
        t.tabLazer = "Lazer";

        t.statusReady = "Ready";

        t.aboutDialogTitle = "About osu! Collection Manager";
        t.aboutVersion = std::string("Version ") + COLLECTION_MANAGER_VERSION;
        t.aboutDesc = "Browse and manage osu! collections from both stable and lazer.";
        t.aboutFormats = "Supports collection.db (stable) and client.realm (lazer).";

        t.paneCollectionsLabel = "Collections";
        t.paneLoading = "Loading...";
        t.paneSelectPrompt = "Select a collection";
        return t;
    }
}