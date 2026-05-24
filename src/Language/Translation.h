#pragma once
#include <string>

namespace Language {
    struct Translation {
        std::string appName;

        std::string menuFile;
        std::string menuFileReload;
        std::string menuFileExit;

        std::string menuView;
        std::string menuViewStable;
        std::string menuViewLazer;
        std::string menuViewHideUnknown;
        std::string menuViewShowUnknown;

        std::string menuHelp;
        std::string menuHelpAbout;

        std::string tabStable;
        std::string tabLazer;

        std::string statusReady;

        std::string aboutDialogTitle;
        std::string aboutVersion;
        std::string aboutDesc;
        std::string aboutFormats;

        std::string githubLinkText;
        std::string githubLink;

        std::string menuFileRecompileScss;

        std::string paneCollectionsLabel;
        std::string paneLoading;
        std::string paneSelectPrompt;
        std::string paneActionNew;
        std::string paneActionImport;
        std::string paneActionSave;
        std::string paneActionBackup;

        std::string columnTitle;
        std::string columnArtist;
        std::string columnDifficulty;
        std::string columnMapper;
        std::string columnStars;
        std::string columnId;
        std::string columnSet;
        std::string columnMd5;

        std::string noCollections;
        std::string unnamedCollection;
        std::string unknownBeatmapTitle;
        std::string noBeatmaps;
        std::string mapCount;
        std::string collectionCount;
        std::string loadingCollection;
        std::string collectionStatus;
        std::string genericError;

        std::string statusReloading;
        std::string statusRecompilingScss;
        std::string statusScssRecompiled;
        std::string statusScssError;
        std::string unknownError;

        std::string menuRename;
        std::string menuExportOsdb;
        std::string menuCopyTo;
        std::string menuPasteBeatmaps;
        std::string menuDelete;
        std::string menuCopyBeatmaps;
        std::string menuRemoveFromCollection;

        std::string clipboardEmpty;
        std::string copiedBeatmapsFrom;
        std::string copiedBeatmapsTo;
        std::string pastedBeatmapsInto;
        std::string pasteError;

        std::string promptCollectionName;
        std::string promptNewName;
        std::string confirmDeleteCollection;
        std::string statusCreatedCollection;
        std::string statusDeletedCollection;
        std::string statusRenamedCollection;

        std::string statusSaving;
        std::string statusSaved;
        std::string saveError;
        std::string statusBackingUp;
        std::string statusBackupCreated;
        std::string backupError;
        std::string statusImporting;
        std::string statusImportedCollections;
        std::string importError;
        std::string exportError;
    };
}
