#include <Web/WebHandler.h>
#include <Detect.h>
#include <Language/Registry.h>

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    // for debugging
    if (const char* home = std::getenv("HOME")) {
        std::string path = std::string(home) + "/.local/bin";
        if (const char* existing = std::getenv("PATH")) {
            path += ":" + std::string(existing);
        }
        setenv("PATH", path.c_str(), 1);
    }

    std::string stableDirectory;
    std::string lazerRealm;
    std::string host;
    std::string port;
    std::string lang = "en";

    for (int i = 1; i < argc; i++) {
        std::string arg(argv[i]);
        auto value = [&](const std::string& prefix) {
            return arg.rfind(prefix, 0) == 0 ? arg.substr(prefix.size()) : "";
        };
        if (arg == "--help") {
            std::cout
                << "osu! Collection Manager " << COLLECTION_MANAGER_VERSION << "\n"
                << "\n"
                << "--stable-path= path to osu! stable directory\n"
                << "--realm-file=  path to osu! lazer client.realm\n"
                << "--host= bind host (default: " << COLLECTION_MANAGER_HOST << ")\n"
                << "--port= bind port (default: " << COLLECTION_MANAGER_PORT << ")\n"
                << "--lang= language code, e.g. de (default: en)\n";
            return 0;
        }
        if (auto v = value("--stable-path="); !v.empty()) { stableDirectory = v; }
        else if (auto v = value("--realm-file="); !v.empty()) { lazerRealm = v; }
        else if (auto v = value("--host="); !v.empty()) { host = v; }
        else if (auto v = value("--port="); !v.empty()) { port = v; }
        else if (auto v = value("--lang="); !v.empty()) { lang = v; }
    }

    if (stableDirectory.empty()) { stableDirectory = detect::stableDir(); }
    if (lazerRealm.empty()) { lazerRealm = detect::lazerRealm(); }

    Web::WebHandler::Config config;
    config.wwwDir = (std::filesystem::weakly_canonical(std::filesystem::path(argv[0])).parent_path() / "www").string();
    config.host = host.empty() ? COLLECTION_MANAGER_HOST : host;
    config.port = port.empty() ? COLLECTION_MANAGER_PORT : static_cast<uint16_t>(std::stoi(port));
    config.lazerRealm = lazerRealm;
    config.translation = Language::load(lang);

    if (stableDirectory.empty()) {
        std::cerr << "[warn] osu! stable not found. Pass --stable-path=\n";
    } else {
        config.stableOsuDb = stableDirectory + "/osu!.db";
        config.stableCollectionDb = stableDirectory + "/collection.db";
        if (!std::filesystem::exists(config.stableCollectionDb)) {
            std::cerr << "[warn] collection.db not found in " << stableDirectory << "\n";
        }
    }

    if (config.lazerRealm.empty()) {
        std::cerr << "[warn] osu! lazer not found. Pass --realm-file=\n";
    }

    std::cout << "Starting osu! Collection Manager on http://" << config.host << ":" << config.port << "\n";
    if (!stableDirectory.empty()) {
        std::cout << "Stable: " << stableDirectory << "\n";
    }
    if (!config.lazerRealm.empty()) {
        std::cout << "Lazer: " << config.lazerRealm << "\n";
    }

    Web::WebHandler::start(std::move(config));
}