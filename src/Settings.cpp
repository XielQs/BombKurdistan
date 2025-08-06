#include "Settings.hpp"

#include "Constants.hpp"
#include "Game.hpp"
#include "Input.hpp"
#include "MainMenu.hpp"
#include "raylib.h"

#include <filesystem>
#include <fstream>
#include <string>

extern Game game;

std::string Settings::settingsPath =
    std::string(getenv("HOME")) + "/.config/BombKurdistan/settings.cfg";

void Settings::init()
{
    load();
    applySettings();
}

void Settings::reset()
{
    config.vsync = true;
    config.targetFPS = DEFAULT_GAME_FPS;
    config.musicVolume = 1.0f;
    config.muteMusic = false;
    config.discordRPC = true;
    tempConfig = config; // reset temp config to default
}

void Settings::load()
{
    std::ifstream file(settingsPath);
    if (!file.is_open()) {
        TraceLog(LOG_WARNING, "Settings file not found, using default settings.");
        reset();
        return;
    }

    std::string line;

    while (std::getline(file, line)) {
        const size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            tempConfig.fromKey(key, value);
            tempConfig.healthCheck(); // validate settings
        }
    }

    file.close();
}

void Settings::save() const
{
    std::filesystem::create_directory(std::filesystem::path(settingsPath).parent_path());
    std::ofstream file(settingsPath);
    if (!file.is_open()) {
        TraceLog(LOG_ERROR, "Failed to open settings file for writing: %s", settingsPath.c_str());
        return;
    }

    config.writeFile(file);

    file.close();
}

void Settings::applySettings()
{
    if (!IsWindowReady()) {
        TraceLog(LOG_ERROR, "Window is not ready, cannot apply video settings.");
        return;
    }
    if (tempConfig.targetFPS != config.targetFPS || tempConfig.vsync != config.vsync)
        SetTargetFPS(tempConfig.vsync ? 0 : tempConfig.targetFPS);
    if (tempConfig.vsync != config.vsync)
        SetWindowState(tempConfig.vsync ? FLAG_VSYNC_HINT : 0);
    if (tempConfig.discordRPC != config.discordRPC) {
        if (tempConfig.discordRPC)
            game.connectDiscord();
        else
            game.disconnectDiscord();
    }
    // TODO: add fullscreen
    config = tempConfig; // apply temporary config to the main config
}

void Settings::handleInput()
{
    if (state != SettingsState::MAIN_MENU && Input::isEscapeKey()) {
        state = SettingsState::MAIN_MENU;
        selectedOption = 0;
        tempConfig = config; // reset temp config to current config
        return;
    }

    switch (state) {
        case SettingsState::MAIN_MENU:
            handleMainMenuInput();
            break;
        case SettingsState::VIDEO_SETTINGS:
            handleVideoSettingsInput();
            break;
        case SettingsState::AUDIO_SETTINGS:
            handleAudioSettingsInput();
            break;
        case SettingsState::OTHER_SETTINGS:
            handleOtherSettingsInput();
            break;
    }
}

void Settings::draw() const
{
    switch (state) {
        case SettingsState::MAIN_MENU:
            drawMainMenu();
            break;
        case SettingsState::VIDEO_SETTINGS:
            drawVideoSettings();
            break;
        case SettingsState::AUDIO_SETTINGS:
            drawAudioSettings();
            break;
        case SettingsState::OTHER_SETTINGS:
            drawOtherSettings();
            break;
    }

    if (state != SettingsState::MAIN_MENU) {
        Game::drawTextCombined(SCREEN_DRAW_X, SCREEN_HEIGHT - TEXT_HEIGHT * 4, 20,
                               {{"VAZGECMEK", RED}, {"icin ESC bas", WHITE}});

        Game::drawTextCenter("Deger degistirmek icin < veya > tuslarini kullan", SCREEN_DRAW_X,
                             SCREEN_HEIGHT - TEXT_HEIGHT * 2, 20, GRAY);
    }
}

void Settings::drawMainMenu() const
{
    Game::drawTextCenter("Ayarlar", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -2, 20, WHITE);

    const char *options[] = {"Video Ayarlari", "Ses Ayarlari", "Diger Ayarlar"};

    for (size_t i = 0; i < std::size(options); ++i) {
        const Color color = (menuOption == static_cast<int>(i)) ? YELLOW : GRAY;
        const float y = SCREEN_DRAW_Y + TEXT_HEIGHT * (0 + i);
        Game::drawTextCenter(options[i], SCREEN_DRAW_X, y, 20, color);
    }

    Game::drawTextCenter("Geri Don", SCREEN_DRAW_X, SCREEN_HEIGHT - TEXT_HEIGHT * 4, 20,
                         (menuOption == 3) ? YELLOW : GRAY);

    Game::drawTextCenter("Secim yapmak icin ok tuslarini kullan, enter veya space tusu ile sec",
                         SCREEN_DRAW_X, SCREEN_HEIGHT - TEXT_HEIGHT * 2, 20, GRAY);
}

void Settings::handleMainMenuInput()
{
    if (Input::isEscapeKey()) {
        MainMenu::state = MainMenuState::MAIN_MENU;
        menuOption = 0; // reset menu option
        return;
    }

    if (Input::isArrowUp())
        menuOption = (menuOption - 1 + 4) % 4;
    if (Input::isArrowDown())
        menuOption = (menuOption + 1) % 4;

    if (Input::isEnterOrSpace() || Input::isArrowLeft() || Input::isArrowRight()) {
        switch (menuOption) {
            case 0: // Video Ayarlari
                state = SettingsState::VIDEO_SETTINGS;
                break;
            case 1: // Ses Ayarlari
                state = SettingsState::AUDIO_SETTINGS;
                break;
            case 2: // Diger Ayarlar
                state = SettingsState::OTHER_SETTINGS;
                break;
            case 3: // Geri Don
                MainMenu::state = MainMenuState::MAIN_MENU;
                menuOption = 0; // reset menu option
                break;
            default:
                break;
        }
    }
}

void Settings::drawVideoSettings() const
{
    Game::drawTextCenter("Video Ayarlari", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -2, 20,
                         WHITE);

    drawToggleOption("VSync", tempConfig.vsync, 0, SCREEN_DRAW_Y + TEXT_HEIGHT * 0);

    const float endX = Game::drawTextCombined(
        SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 1, 20,
        {{"FPS Limiti", selectedOption == 1 ? YELLOW : GRAY},
         {tempConfig.vsync            ? "VSync"
          : tempConfig.targetFPS == 0 ? "Sinirsiz"
                                      : TextFormat("%d", tempConfig.targetFPS),
          WHITE}});

    if (tempConfig.targetFPS == 0 && !tempConfig.vsync) {
        DrawText("Ekran yirtilmasi olabilir", endX + 10.f, SCREEN_DRAW_Y + TEXT_HEIGHT * 1 - 8, 18,
                 RED);
    }

    drawToggleOption("Tam Ekran", tempConfig.fullscreen, 2, SCREEN_DRAW_Y + TEXT_HEIGHT * 2);

    if (tempConfig.fullscreen) {
        // warn user the feature is not implemented yet
        Game::drawTextCenter("Tam ekran modu henuz yapim asamasinda!!", SCREEN_DRAW_X,
                             SCREEN_DRAW_Y + TEXT_HEIGHT * 3, 20, RED);
    }

    Game::drawTextCenter("AYARLARI UYGULA", SCREEN_DRAW_X, SCREEN_HEIGHT - TEXT_HEIGHT * 5, 20,
                         (selectedOption == 3) ? GREEN : DARKGREEN);
}

void Settings::handleVideoSettingsInput()
{
    if (Input::isArrowUp())
        selectedOption = (selectedOption - 1 + 4) % 4;
    if (Input::isArrowDown())
        selectedOption = (selectedOption + 1) % 4;

    if (Input::isEnterOrSpace() || Input::isArrowLeft() || Input::isArrowRight()) {
        switch (selectedOption) {
            case 0: // VSync
                tempConfig.vsync = !tempConfig.vsync;
                break;
            case 1: // FPS Limiti
                if (!tempConfig.vsync) {
                    int targetFPS = tempConfig.targetFPS;
                    if (Input::isArrowLeft())
                        targetFPS = (targetFPS <= 0) ? 300 : targetFPS - 30;
                    if (Input::isArrowRight() || Input::isEnterOrSpace())
                        targetFPS = (targetFPS >= 300) ? 0 : targetFPS + 30;
                    targetFPS = (targetFPS < 0) ? 0 : (targetFPS > 300) ? 300 : targetFPS;
                    tempConfig.targetFPS = targetFPS;
                }
                break;
            case 2: // Tam Ekran
                tempConfig.fullscreen = !tempConfig.fullscreen;
                break;
            case 3: // Uygula
                applySettings();
                save();
                state = SettingsState::MAIN_MENU;
                selectedOption = 0; // reset selected option
                break;
            default:
                break;
        }
    }
}

void Settings::drawAudioSettings() const
{
    Game::drawTextCenter("Ses Ayarlari", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -2, 20,
                         WHITE);

    drawToggleOption("Muzigi sustur", tempConfig.muteMusic, 0, SCREEN_DRAW_Y + TEXT_HEIGHT * 0);

    Game::drawTextCenter("AYARLARI UYGULA", SCREEN_DRAW_X, SCREEN_HEIGHT - TEXT_HEIGHT * 5, 20,
                         (selectedOption == 1) ? GREEN : DARKGREEN);
}

void Settings::handleAudioSettingsInput()
{
    if (Input::isArrowUp())
        selectedOption = (selectedOption - 1 + 2) % 2;
    if (Input::isArrowDown())
        selectedOption = (selectedOption + 1) % 2;

    if (Input::isEnterOrSpace() || Input::isArrowLeft() || Input::isArrowRight()) {
        switch (selectedOption) {
            case 0: // Muzigi kapat
                tempConfig.muteMusic = !tempConfig.muteMusic;
                break;
            case 1: // Uygula
                applySettings();
                save();
                state = SettingsState::MAIN_MENU;
                selectedOption = 0; // reset selected option
                break;
            default:
                break;
        }
    }
}

void Settings::drawOtherSettings() const
{
    Game::drawTextCenter("Diger Ayarlar", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -2, 20,
                         WHITE);

    drawToggleOption("Discord RPC", tempConfig.discordRPC, 0, SCREEN_DRAW_Y + TEXT_HEIGHT * 0);
#ifdef DISCORD_RPC_ENABLED
    Game::drawTextCenter("Discord durumunuzda gosterilecek", SCREEN_DRAW_X,
                         SCREEN_DRAW_Y + TEXT_HEIGHT * 1, 19, GRAY);
#else
    Game::drawTextCenter("Platformunuz bu ozelligi desteklemiyor", SCREEN_DRAW_X,
                         SCREEN_DRAW_Y + TEXT_HEIGHT * 1, 19, RED);
#endif
    Game::drawTextCenter("AYARLARI UYGULA", SCREEN_DRAW_X, SCREEN_HEIGHT - TEXT_HEIGHT * 5, 20,
                         (selectedOption == 1) ? GREEN : DARKGREEN);
}

void Settings::handleOtherSettingsInput()
{
    if (Input::isArrowUp())
        selectedOption = (selectedOption - 1 + 2) % 2;
    if (Input::isArrowDown())
        selectedOption = (selectedOption + 1) % 2;

    if (Input::isEnterOrSpace() || Input::isArrowLeft() || Input::isArrowRight()) {
        switch (selectedOption) {
            case 0: // Discord RPC
                tempConfig.discordRPC = !tempConfig.discordRPC;
                break;
            case 1: // Uygula
                applySettings();
                save();
                state = SettingsState::MAIN_MENU;
                selectedOption = 0; // reset selected option
                break;
            default:
                break;
        }
    }
}

void Settings::drawToggleOption(const char *label, bool option, int selectIndex, float y) const
{
    const char *status = option ? "ACIK" : "KAPALI";
    const Color statusColor = option ? GREEN : RED;

    const Color textColor = (selectedOption == selectIndex) ? YELLOW : GRAY;
    Game::drawTextCombined(SCREEN_DRAW_X, y, 20, {{label, textColor}, {status, statusColor}});
}
