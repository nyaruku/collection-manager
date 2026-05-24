#pragma once

#include <array>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <string_view>

namespace detect {
    static std::string runCommand(const std::string& command) {
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) return {};
        std::string output;
        std::array<char, 256> buffer;
        while (fgets(buffer.data(), buffer.size(), pipe)) output += buffer.data();
        pclose(pipe);
        return output;
    }

    // Parses the osu! stable folder from `osu-wine --info`.
    // String: osu! folder: '/path/to/osu!/osugame'
    static std::string osuWineStableDir() {
        std::string output = runCommand("osu-wine --info 2>/dev/null");
        constexpr std::string_view marker = "osu! folder: '";
        size_t start = output.find(marker);
        if (start == std::string::npos) return {};
        start += marker.size();
        size_t end = output.find('\'', start);
        if (end == std::string::npos) return {};
        return output.substr(start, end - start);
    }

    static std::string stableDir() {
        for (const auto& path : { osuWineStableDir() }) {
            if (!path.empty() && std::filesystem::exists(std::filesystem::path(path) / "collection.db"))
                return path;
        }
        return {};
    }

    // The Flatpak data root is always $HOME/.var/app/<app-id>/data (Flatpak XDG sandbox spec).
    static std::string flatpakLazerDir() {
        if (runCommand("flatpak info sh.ppy.osu 2>/dev/null").empty()) return {};
        const char* home = std::getenv("HOME");
        if (!home) return {};
        return std::string(home) + "/.var/app/sh.ppy.osu/data/osu";
    }

    // osu! lazer follows the XDG base directory spec for its data directory.
    // XDG spec: if XDG_DATA_HOME is unset, the default is $HOME/.local/share
    static std::string lazerRealm() {
        const char* xdgDataHome = std::getenv("XDG_DATA_HOME");
        const char* home = std::getenv("HOME");
        const std::string xdgData = xdgDataHome ? xdgDataHome : (home ? std::string(home) + "/.local/share" : "");
        const std::string flatpakDir = flatpakLazerDir();

        for (const auto& path : {
            xdgData.empty() ? "" : xdgData + "/osu/client.realm",
            flatpakDir.empty() ? "" : flatpakDir + "/client.realm",
        }) {
            if (!path.empty() && std::filesystem::exists(path)) return path;
        }
        return {};
    }
}
