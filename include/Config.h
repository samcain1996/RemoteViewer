#pragma once

#include "Quickshot/Capture.h"

struct Configs {

    using ConfigMap = std::unordered_map<string, int>;

    static inline ConfigMap configMap = {
        { "VIDEO_THREADS", 0 } ,
        { "TARGET_FPS",    0 }
    };

    static inline const int& VIDEO_THREADS = configMap["VIDEO_THREADS"];
    static inline const int& TARGET_FPS    = configMap["TARGET_FPS"];

};

static auto LoadConfig = []() {

    ifstream configFile("config.conf");
    string line;
    string contents;
    while (getline(configFile, line)) {
        contents += line;
    }

    for_each(Configs::configMap.begin(), Configs::configMap.end(), [&contents](auto& nameVal) {
        auto& [name, val] = nameVal;
        auto offset = contents.find(name) + 1;
        if (offset != string::npos) {
            nameVal.second = stoi(contents.substr(offset + name.size(), 1));
        }
    });

    return true;
};

static inline bool ConfigLoaded = LoadConfig();