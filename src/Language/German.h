#pragma once
#include <Language/Translation.h>

namespace Language {
    inline Translation german() {
        Translation t;
        t.appName = "osu! Collection Manager";

        t.menuFile = "Datei";
        t.menuFileReload = "Alle neu laden";
        t.menuFileExit = "Beenden";

        t.menuView = "Ansicht";
        t.menuViewStable = "Stable";
        t.menuViewLazer = "Lazer";
        t.menuViewHideUnknown = "Unbekannte Maps ausblenden";

        t.menuHelp = "Hilfe";
        t.menuHelpAbout = "Über";

        t.tabStable = "Stable";
        t.tabLazer = "Lazer";

        t.statusReady = "Bereit";

        t.aboutDialogTitle = "Über osu! Collection Manager";
        t.aboutVersion = std::string("Version ") + COLLECTION_MANAGER_VERSION;
        t.aboutDesc = "Durchsuche und verwalte Collections aus Stable und Lazer.";
        t.aboutFormats = "Unterstützt collection.db (Stable) und client.realm (Lazer).";

        t.githubLinkText = "GitHub Repository";
        t.githubLink = "https://github.com/nyaruku/collection-manager";

        t.menuFileRecompileScss = "SCSS re-kompilieren";

        t.paneCollectionsLabel = "Collections";
        t.paneLoading = "Wird geladen...";
        t.paneSelectPrompt = "Collection auswählen";
        return t;
    }
}

