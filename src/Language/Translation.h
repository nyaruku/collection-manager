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
    };
}

