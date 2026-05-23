#pragma once
#include <language/translation.h>

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
        t.menuViewHideMd5 = "MD5 ausblenden";

        t.menuHelp = "Hilfe";
        t.menuHelpAbout = "Über";

        t.tabStable = "Stable";
        t.tabLazer = "Lazer";

        t.statusReady = "Bereit";

        t.aboutDialogTitle = "Über osu! Sammlungsverwaltung";
        t.aboutVersion = "Version 0.1.0";
        t.aboutDesc = "Durchsuche und verwalte deine osu!-Sammlungen aus Stable und Lazer.";
        t.aboutFormats = "Unterstützt collection.db (Stable) und client.realm (Lazer).";

        t.paneCollectionsLabel = "Sammlungen";
        t.paneLoading = "Wird geladen...";
        t.paneSelectPrompt = "Sammlung auswählen";
        return t;
    }
}

