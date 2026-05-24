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
        t.menuViewShowUnknown = "Unbekannte Maps anzeigen";

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
        t.paneActionNew = "Neu";
        t.paneActionImport = "Import";
        t.paneActionSave = "Speichern";
        t.paneActionBackup = "Backup";

        t.columnTitle = "Titel";
        t.columnArtist = "Artist";
        t.columnDifficulty = "Difficulty";
        t.columnMapper = "Mapper";
        t.columnStars = "Stars";
        t.columnId = "ID";
        t.columnSet = "Set";
        t.columnMd5 = "MD5";

        t.noCollections = "Keine Collections";
        t.unnamedCollection = "(unbenannt)";
        t.unknownBeatmapTitle = "nicht heruntergeladen oder alte Version";
        t.noBeatmaps = "{name} - keine Beatmaps";
        t.mapCount = "{count} Maps";
        t.collectionCount = "{mode}: {count} Collections";
        t.loadingCollection = "Lade \"{name}\"...";
        t.collectionStatus = "\"{name}\" ({count} Beatmaps)";
        t.genericError = "Fehler: {error}";

        t.statusReloading = "Lade neu...";
        t.statusRecompilingScss = "Kompiliere SCSS...";
        t.statusScssRecompiled = "SCSS kompiliert, bitte die Seite neu laden.";
        t.statusScssError = "SCSS Fehler: {error}";
        t.unknownError = "unbekannter Fehler";

        t.menuRename = "Umbenennen";
        t.menuExportOsdb = "Als osdb exportieren";
        t.menuCopyTo = "Nach {mode} kopieren";
        t.menuPasteBeatmaps = "{count} Beatmap(s) einfügen";
        t.menuDelete = "Löschen";
        t.menuCopyBeatmaps = "{count} Beatmap(s) kopieren";
        t.menuRemoveFromCollection = "Aus Collection entfernen";

        t.clipboardEmpty = "Zwischenablage ist leer.";
        t.copiedBeatmapsFrom = "{count} Beatmap(s) aus \"{name}\" kopiert";
        t.copiedBeatmapsTo = "{count} Beatmap(s) nach {mode} kopiert";
        t.pastedBeatmapsInto = "{count} Beatmap(s) in \"{name}\" eingefügt";
        t.pasteError = "Einfügen fehlgeschlagen: {error}";

        t.promptCollectionName = "Collection-Name:";
        t.promptNewName = "Neuer Name:";
        t.confirmDeleteCollection = "\"{name}\" löschen?";
        t.statusCreatedCollection = "\"{name}\" erstellt";
        t.statusDeletedCollection = "\"{name}\" gelöscht";
        t.statusRenamedCollection = "Umbenannt zu \"{name}\"";

        t.statusSaving = "Speichere {mode} collection....";
        t.statusSaved = "{mode} collection gespeichert.";
        t.saveError = "Speichern fehlgeschlagen: {error}";
        t.statusBackingUp = "Erstelle Backup für {mode}...";
        t.statusBackupCreated = "{mode} Backup erstellt: {path}";
        t.backupError = "Backup fehlgeschlagen: {error}";
        t.statusImporting = "Importiere {file}...";
        t.statusImportedCollections = "{count} Collections nach {mode} importiert";
        t.importError = "Import fehlgeschlagen: {error}";
        t.exportError = "Export fehlgeschlagen: {error}";
        return t;
    }
}
