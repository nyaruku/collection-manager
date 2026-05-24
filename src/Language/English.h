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
        t.menuViewShowUnknown = "Show Unknown Maps";

        t.menuHelp = "Help";
        t.menuHelpAbout = "About";

        t.tabStable = "Stable";
        t.tabLazer = "Lazer";

        t.statusReady = "Ready";

        t.aboutDialogTitle = "About osu! Collection Manager";
        t.aboutVersion = std::string("Version ") + COLLECTION_MANAGER_VERSION;
        t.aboutDesc = "Browse and manage osu! collections from both stable and lazer.";
        t.aboutFormats = "Supports collection.db (stable) and client.realm (lazer).";

        t.githubLinkText = "GitHub Repository";
        t.githubLink = "https://github.com/nyaruku/collection-manager";

        t.menuFileRecompileScss = "Recompile SCSS";

        t.paneCollectionsLabel = "Collections";
        t.paneLoading = "Loading...";
        t.paneSelectPrompt = "Select a collection";
        t.paneActionNew = "New";
        t.paneActionImport = "Import";
        t.paneActionSave = "Save";
        t.paneActionBackup = "Backup";

        t.columnTitle = "Title";
        t.columnArtist = "Artist";
        t.columnDifficulty = "Difficulty";
        t.columnMapper = "Mapper";
        t.columnStars = "Stars";
        t.columnId = "ID";
        t.columnSet = "Set";
        t.columnMd5 = "MD5";

        t.noCollections = "No collections";
        t.unnamedCollection = "(unnamed)";
        t.unknownBeatmapTitle = "not downloaded or old version";
        t.noBeatmaps = "{name} - no beatmaps";
        t.mapCount = "{count} Maps";
        t.collectionCount = "{mode}: {count} collections";
        t.loadingCollection = "Loading \"{name}\"...";
        t.collectionStatus = "\"{name}\" ({count} beatmaps)";
        t.genericError = "Error: {error}";

        t.statusReloading = "Reloading...";
        t.statusRecompilingScss = "Recompiling SCSS...";
        t.statusScssRecompiled = "SCSS recompiled, reload the page to apply changes.";
        t.statusScssError = "SCSS error: {error}";
        t.unknownError = "unknown error";

        t.menuRename = "Rename";
        t.menuExportOsdb = "Export as osdb";
        t.menuCopyTo = "Copy to {mode}";
        t.menuPasteBeatmaps = "Paste {count} beatmap(s)";
        t.menuDelete = "Delete";
        t.menuCopyBeatmaps = "Copy {count} beatmap(s)";
        t.menuRemoveFromCollection = "Remove from collection";

        t.clipboardEmpty = "Clipboard is empty.";
        t.copiedBeatmapsFrom = "Copied {count} beatmap(s) from \"{name}\"";
        t.copiedBeatmapsTo = "Copied {count} beatmap(s) to {mode}";
        t.pastedBeatmapsInto = "Pasted {count} beatmap(s) into \"{name}\"";
        t.pasteError = "Paste error: {error}";

        t.promptCollectionName = "Collection name:";
        t.promptNewName = "New name:";
        t.confirmDeleteCollection = "Delete \"{name}\"?";
        t.statusCreatedCollection = "Created \"{name}\"";
        t.statusDeletedCollection = "Deleted \"{name}\"";
        t.statusRenamedCollection = "Renamed to \"{name}\"";

        t.statusSaving = "Saving {mode}...";
        t.statusSaved = "{mode} saved.";
        t.saveError = "Save error: {error}";
        t.statusBackingUp = "Backing up {mode}...";
        t.statusBackupCreated = "{mode} backup created: {path}";
        t.backupError = "Backup error: {error}";
        t.statusImporting = "Importing {file}...";
        t.statusImportedCollections = "Imported {count} collections into {mode}";
        t.importError = "Import error: {error}";
        t.exportError = "Export error: {error}";
        return t;
    }
}