#pragma once
#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include "Constants.hpp"
#include "raylib.h"

#include <fstream>
#include <string>

enum class SettingsState { MAIN_MENU, VIDEO_SETTINGS, AUDIO_SETTINGS, OTHER_SETTINGS };

struct Config
{
    bool vsync;
    int targetFPS;
    bool fullscreen;

    float musicVolume;
    bool muteMusic;

    bool discordRPC;

    void writeFile(std::ofstream &file) const
    {
        file << "vsync=" << (vsync ? "1" : "0") << "\n";
        file << "targetFPS=" << targetFPS << "\n";
        file << "fullscreen=" << (fullscreen ? "1" : "0") << "\n";
        file << "musicVolume=" << musicVolume << "\n";
        file << "muteMusic=" << (muteMusic ? "1" : "0") << "\n";
        file << "discordRPC=" << (discordRPC ? "1" : "0");
    }

    void fromKey(const std::string &key, const std::string &value)
    {
        if (key == "vsync")
            vsync = (value == "1");
        else if (key == "targetFPS")
            targetFPS = std::stoi(value);
        else if (key == "fullscreen")
            fullscreen = (value == "1");
        else if (key == "musicVolume")
            musicVolume = std::stof(value);
        else if (key == "muteMusic")
            muteMusic = (value == "1");
        else if (key == "discordRPC")
            discordRPC = (value == "1");
        else
            TraceLog(LOG_WARNING, "Unknown setting: %s", key.c_str());
    }

    void healthCheck()
    {
        if (targetFPS < 0) {
            TraceLog(LOG_WARNING, "Invalid target FPS setting, resetting to default.");
            targetFPS = DEFAULT_GAME_FPS;
        }
        if (musicVolume < 0.0f || musicVolume > 1.0f) {
            TraceLog(LOG_WARNING, "Invalid music volume setting, resetting to default.");
            musicVolume = 1.0f;
        }
    }
};

class Settings
{
public:
    Config config{};
    // using tempConfig to store changes before applying them
    // maybe copying the whole config is not the best idea
    // but that's just a pixel game :pray:
    Config tempConfig{};

    int selectedOption = 0;
    int menuOption = 0; // used for main menu navigation

    void init();
    void reset();
    void handleInput();
    void draw() const;
    void save() const;

private:
    SettingsState state = SettingsState::MAIN_MENU;
    static std::string settingsPath;

    void load();
    void applySettings();
    void drawMainMenu() const;
    void handleMainMenuInput();
    void drawVideoSettings() const;
    void handleVideoSettingsInput();
    void drawAudioSettings() const;
    void handleAudioSettingsInput();
    void drawOtherSettings() const;
    void handleOtherSettingsInput();
    void drawToggleOption(const char *label, bool option, int selectIndex, float y) const;
};

#endif // SETTINGS_HPP
