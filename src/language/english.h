#pragma once
#include <language/translation.h>

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
        t.menuViewHideMd5 = "Hide MD5";

        t.menuHelp = "Help";
        t.menuHelpAbout = "About";

        t.tabStable = "Stable";
        t.tabLazer = "Lazer";

        t.statusReady = "Ready";

        t.aboutDialogTitle = "About osu! Collection Manager";
        t.aboutVersion = "Version 0.1.0";
        t.aboutDesc = "Browse and manage your osu! collections from both stable and lazer.";
        t.aboutFormats = "Supports collection.db (stable) and client.realm (lazer).";

        t.paneCollectionsLabel = "Collections";
        t.paneLoading = "Loading...";
        t.paneSelectPrompt = "Select a collection";
        return t;
    }
}