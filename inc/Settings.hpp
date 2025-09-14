#pragma once
#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include "Constants.hpp"
#include "Game.hpp"
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
    int bgMusicIndex; // index of the selected background music track

    bool discordRPC;
    bool shakeScreen;

    void writeFile(std::ofstream &file) const
    {
        file << "vsync=" << (vsync ? "1" : "0") << "\n";
        file << "targetFPS=" << targetFPS << "\n";
        file << "fullscreen=" << (fullscreen ? "1" : "0") << "\n";
        file << "musicVolume=" << musicVolume << "\n";
        file << "bgMusicIndex=" << bgMusicIndex << "\n";
        file << "discordRPC=" << (discordRPC ? "1" : "0") << "\n";
        file << "shakeScreen=" << (shakeScreen ? "1" : "0") << "\n";
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
        else if (key == "bgMusicIndex")
            bgMusicIndex = std::stoi(value);
        else if (key == "discordRPC")
            discordRPC = (value == "1");
        else if (key == "shakeScreen")
            shakeScreen = (value == "1");
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
        if (bgMusicIndex != 0 && bgMusicIndex != 1) { // currently we have only 2 tracks
            TraceLog(LOG_WARNING, "Invalid background music index, resetting to default.");
            bgMusicIndex = 0;
        }
    }
};

class Settings
{
public:
    static Config config;
    // using tempConfig to store changes before applying them
    // maybe copying the whole config is not the best idea
    // but that's just a pixel game :pray:
    static Config tempConfig;

    static int selectedOption;
    static int menuOption; // used for main menu navigation
    static GameState previousGameState;

    static void init();
    static void reset();
    static void handleInput();
    static void draw();
    static void save();
    static void load();

private:
    static SettingsState state;

    static std::string getSettingsPath();
    static void applySettings(bool isInit = false);
    static void drawMainMenu();
    static void handleMainMenuInput();
    static void drawVideoSettings();
    static void handleVideoSettingsInput();
    static void drawAudioSettings();
    static void handleAudioSettingsInput();
    static void drawOtherSettings();
    static void handleOtherSettingsInput();
    static void drawToggleOption(const char *label, bool option, int selectIndex, float y);
};

#endif // SETTINGS_HPP
