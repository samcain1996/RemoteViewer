#pragma once

#include "QuickShot/Capture.h"

struct Configs {

    using ConfigMap = std::unordered_map<string, int>;

    // Default config values
    static inline ConfigMap configMap = {
        { "VIDEO_THREADS", 1  },
        { "TARGET_FPS",    30 },
        { "TIMEOUT",       10 }
    };

    static inline const int& VIDEO_THREADS = configMap["VIDEO_THREADS"];
    static inline const int& TARGET_FPS    = configMap["TARGET_FPS"];
    static inline const int& TIMEOUT       = configMap["TIMEOUT"];

    static bool CreateConfigFile() {
        std::ofstream configFile("config.conf");

        if (!configFile.good()) { return false; }

        for_each(configMap.begin(), configMap.end(),
            [&configFile](const auto& config) {
                configFile << config.first << "=" << std::to_string(config.second)
                    << "\n";
            });

        return true;
    }

};

inline auto LoadConfig = []() {

    // Create config file if non exist
    ifstream configFile("config.conf");
    if (!configFile.good()) { return Configs::CreateConfigFile(); }

    // Read config file
    string line;
    while (getline(configFile, line)) {
        if (line.find('=') != line.npos) {
            string config = line.substr(0, line.find('='));
            int val = std::stoi(line.substr(line.find('=') + 1));

            if (Configs::configMap.contains(config)) {
                Configs::configMap[config] = val;
            }
        }
    }
    return true;
};

inline bool ConfigLoaded = LoadConfig();