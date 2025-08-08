#include "PauseScreen.hpp"

#include "Difficulty.hpp"
#include "Game.hpp"
#include "Input.hpp"
#include "MainMenu.hpp"
#include "Settings.hpp"
#include "raylib.h"

extern Game game;

int PauseScreen::selectedOption = 0;

void PauseScreen::draw()
{
    Game::drawTextCenter("Kurdistan Bombalayici", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -5,
                         20, WHITE);
    Game::drawTextCenter("Bu bir oyun projesidir ve tamamiyla eglence amaciyla uretilmistir",
                         SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -4, 20, GRAY);

    Game::drawTextCenter("Oyun Duraklatildi", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -2, 20,
                         WHITE);

    Game::drawTextCenter("Devam Et", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 2, 20,
                         selectedOption == 0 ? GREEN : GRAY);
    Game::drawTextCenter("Ayarlar", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 3, 20,
                         selectedOption == 1 ? YELLOW : GRAY);
    Game::drawTextCenter("Ana Menu", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 5, 20,
                         selectedOption == 2 ? RED : GRAY);

    Game::drawTextCombined(SCREEN_DRAW_X, SCREEN_HEIGHT - TEXT_HEIGHT * 2, 20,
                           {{"Oyun Modu:", GRAY},
                            {getDifficultyName(currentDifficulty),
                             currentDifficulty == Difficulty::EASY     ? GREEN
                             : currentDifficulty == Difficulty::NORMAL ? YELLOW
                                                                       : DARKRED}});
}

void PauseScreen::handleInput()
{
    if (Input::isEscapeKey()) {
        // resume the game
        game.setGameState(GameState::PLAYING);
    }

    if (Input::isArrowUp())
        selectedOption = (selectedOption - 1 + 3) % 3;
    if (Input::isArrowDown())
        selectedOption = (selectedOption + 1) % 3;

    if (Input::isEnterOrSpace() || Input::isArrowLeft() || Input::isArrowRight()) {
        switch (selectedOption) {
            case 0: // Devam Et
                game.setGameState(GameState::PLAYING);
                break;
            case 1: // Ayarlar
                Settings::previousGameState = GameState::PAUSED;
                Settings::selectedOption = 0;
                Settings::menuOption = 0;
                Settings::tempConfig = Settings::config; // reset temp config to current config
                MainMenu::state = MainMenuState::SETTINGS;
                game.setGameState(GameState::MAIN_MENU);
                break;
            case 2: // Ana Menu
                MainMenu::state = MainMenuState::MAIN_MENU;
                game.setGameState(GameState::MAIN_MENU);
                break;
            default:
                break;
        }
    }
}
