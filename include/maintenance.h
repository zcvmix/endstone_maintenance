#pragma once

#include <endstone/endstone.hpp>
#include <unordered_map>
#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>

class SimpleConfig {
    std::string path_;
    std::unordered_map<std::string, std::string> data_;

public:
    SimpleConfig(std::string folder, std::string filename) {
        if (!std::filesystem::exists(folder)) {
            std::filesystem::create_directories(folder);
        }
        path_ = folder + "/" + filename;
    }

    void saveDefault(const std::string& content) {
        if (!std::filesystem::exists(path_)) {
            std::ofstream out(path_);
            out << content;
            out.close();
        }
        reload();
    }

    void reload() {
        data_.clear();
        if (!std::filesystem::exists(path_)) return;

        std::ifstream in(path_);
        std::string line;
        while (std::getline(in, line)) {
            auto delimiterPos = line.find(":");
            if (delimiterPos != std::string::npos) {
                std::string key = line.substr(0, delimiterPos);
                std::string value = line.substr(delimiterPos + 1);
                
                auto trim = [](std::string &s) {
                    s.erase(0, s.find_first_not_of(" \t\""));
                    s.erase(s.find_last_not_of(" \t\"") + 1);
                };
                trim(key);
                trim(value);
                
                data_[key] = value;
            }
        }
    }

    std::string getString(const std::string& key, const std::string& def) {
        return (data_.count(key)) ? data_[key] : def;
    }

    int getInt(const std::string& key, int def) {
        try {
            return (data_.count(key)) ? std::stoi(data_[key]) : def;
        } catch (...) { return def; }
    }

    bool getBoolean(const std::string& key, bool def) {
        if (data_.count(key)) {
            std::string v = data_[key];
            return v == "true" || v == "1" || v == "yes";
        }
        return def;
    }
};

class MaintenancePlugin : public endstone::Plugin {
public:
    void onEnable() override;
    void onDisable() override;
    void onPlayerJoin(endstone::PlayerJoinEvent &event);

    SimpleConfig& getConfig() { return *config_; }

private:
    void sendLoginWindow(endstone::Player &player);

    std::unique_ptr<SimpleConfig> config_;
    std::unordered_map<endstone::UUID, endstone::Task *> pending_kicks_;
};
