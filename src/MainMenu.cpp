#include "MainMenu.hpp"

#include "Difficulty.hpp"
#include "Game.hpp"
#include "Input.hpp"
#include "Settings.hpp"

extern Game game;

int MainMenu::selectedOption = 0;
int MainMenu::selectedDifficulty = 1; // default to NORMAL difficulty
MainMenuState MainMenu::state = MainMenuState::MAIN_MENU;

void MainMenu::draw()
{
    if (state != MainMenuState::CREDITS) {
        Game::drawTextCenter("Kurdistan Bombalayici", SCREEN_DRAW_X,
                             SCREEN_DRAW_Y + TEXT_HEIGHT * -5, 20, WHITE);
        Game::drawTextCenter("Bu bir oyun projesidir ve tamamiyla eglence amaciyla uretilmistir",
                             SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -4, 20, GRAY);
    }

    switch (state) {
        case MainMenuState::MAIN_MENU:
            drawMainMenu();
            break;
        case MainMenuState::SELECT_DIFFICULTY:
            drawDifficultySelection();
            break;
        case MainMenuState::CREDITS:
            drawCredits();
            break;
        case MainMenuState::SETTINGS:
            Settings::draw();
            break;
    }
}

void MainMenu::handleInput()
{
    switch (state) {
        case MainMenuState::MAIN_MENU:
            handleMainMenuInput();
            break;
        case MainMenuState::SELECT_DIFFICULTY:
            handleDifficultyInput();
            break;
        case MainMenuState::CREDITS:
            if (Input::isEscapeKey() || Input::isEnterOrSpace()) {
                state = MainMenuState::MAIN_MENU;
            }
            break;
        case MainMenuState::SETTINGS:
            Settings::handleInput();
            break;
    }
}

void MainMenu::drawMainMenu()
{
    Game::drawTextCenter("Ana Menu", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -2, 20, WHITE);

    Game::drawTextCenter("Oyuna Basla", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 2, 20,
                         selectedOption == 0 ? GREEN : GRAY);
    Game::drawTextCenter("Ayarlar", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 3, 20,
                         selectedOption == 1 ? YELLOW : GRAY);
    Game::drawTextCenter("Yapimcilar", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 4, 20,
                         selectedOption == 2 ? RED : GRAY);
    Game::drawTextCenter("Cikis", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 5, 20,
                         selectedOption == 3 ? DARKRED : GRAY);

    Game::drawTextCombined(SCREEN_DRAW_X, SCREEN_HEIGHT - TEXT_HEIGHT * 2, 20,
                           {{"XielQ tarafindan", GRAY}, {"sevgi", RED}, {"ile yapildi", GRAY}});
}
void MainMenu::handleMainMenuInput()
{
    // prevent accidental exit
    if (Input::isEscapeKey() && !Input::isMouseRightButton())
        game.cleanup();

    if (Input::isArrowUp())
        selectedOption = (selectedOption - 1 + mainMenuOptions) % mainMenuOptions;
    if (Input::isArrowDown())
        selectedOption = (selectedOption + 1) % mainMenuOptions;

    if (Input::isEnterOrSpace() || Input::isArrowLeft() || Input::isArrowRight()) {
        switch (selectedOption) {
            case 0: // Oyun Baslat
                state = MainMenuState::SELECT_DIFFICULTY;
                break;
            case 1: // Ayarlar
                Settings::previousGameState = GameState::MAIN_MENU;
                Settings::selectedOption = 0;
                Settings::menuOption = 0;
                Settings::tempConfig = Settings::config; // reset temp config to current config
                state = MainMenuState::SETTINGS;
                break;
            case 2: // Yapimcilar
                state = MainMenuState::CREDITS;
                break;
            case 3: // Cikis
                game.cleanup();
                break;
            default:
                break;
        }
    }
}

void MainMenu::drawDifficultySelection()
{
    Game::drawTextCenter("Zorluk Secin", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -2, 20,
                         WHITE);

    float easyX = Game::drawTextCenter("Kurt vatandas", SCREEN_DRAW_X,
                                       SCREEN_DRAW_Y + TEXT_HEIGHT * 0, 20, GREEN);
    float normalX = Game::drawTextCenter("Turk vatandas", SCREEN_DRAW_X,
                                         SCREEN_DRAW_Y + TEXT_HEIGHT * 1, 20, YELLOW);
    float hardX = Game::drawTextCenter("ULKUCU VATANDAS", SCREEN_DRAW_X,
                                       SCREEN_DRAW_Y + TEXT_HEIGHT * 2, 20, DARKRED);

    auto calculateDiff = [](float x) -> float {
        return (x - SCREEN_DRAW_X) + 15.f; // adjust the difficulty indicator position
    };

    // draw a (-) to indicate the current difficulty
    // and display the description below it
    // TODO: maybe replace description with score?
    switch (selectedDifficulty) {
        case 0: // Easy
            Game::drawTextCenter("-", SCREEN_DRAW_X - calculateDiff(easyX),
                                 SCREEN_DRAW_Y + TEXT_HEIGHT * 0, 20, GREEN);

            Game::drawTextCenter("Bomba sayisi fazla, mermi sayisi az", SCREEN_DRAW_X,
                                 SCREEN_HEIGHT - TEXT_HEIGHT * 2, 20, LIGHTGRAY);
            break;
        case 1: // Normal
            Game::drawTextCenter("-", SCREEN_DRAW_X - calculateDiff(normalX),
                                 SCREEN_DRAW_Y + TEXT_HEIGHT * 1, 20, YELLOW);

            Game::drawTextCenter("Bomba sayisi ortalama, mermi sayisi ortalama", SCREEN_DRAW_X,
                                 SCREEN_HEIGHT - TEXT_HEIGHT * 2, 20, LIGHTGRAY);
            break;
        case 2: // Hard
            Game::drawTextCenter("-", SCREEN_DRAW_X - calculateDiff(hardX),
                                 SCREEN_DRAW_Y + TEXT_HEIGHT * 2, 20, DARKRED);

            Game::drawTextCenter(
                "Bomba sayisi az, mermi sayisi fazla, boss'un cani %30 altinda yenilenir",
                SCREEN_DRAW_X, SCREEN_HEIGHT - TEXT_HEIGHT * 2, 20, LIGHTGRAY);

            Game::drawTextCenter("sana guveniyoruz kaptan o7", SCREEN_DRAW_X,
                                 SCREEN_HEIGHT - TEXT_HEIGHT * 1, 20, DARKRED);
            break;
        default:
            break;
    }

    Game::drawTextCenter("Geri Don", SCREEN_DRAW_X, SCREEN_HEIGHT - TEXT_HEIGHT * 4, 20,
                         selectedDifficulty == 3 ? YELLOW : GRAY);
}

void MainMenu::handleDifficultyInput()
{
    if (Input::isEscapeKey()) {
        state = MainMenuState::MAIN_MENU;
        selectedDifficulty = 1; // reset selected difficulty to NORMAL
    }

    if (Input::isKeyPressed(KEY_ONE))
        selectedDifficulty = 0;
    if (Input::isKeyPressed(KEY_TWO))
        selectedDifficulty = 1;
    if (Input::isKeyPressed(KEY_THREE))
        selectedDifficulty = 2;

    if (Input::isEnterOrSpace() || Input::isArrowLeft() || Input::isArrowRight()) {
        if (selectedDifficulty == 3) { // Geri don
            state = MainMenuState::MAIN_MENU;
            selectedDifficulty = 1; // reset selected difficulty to NORMAL
        } else {
            Input::lockMouse();
            currentDifficulty = static_cast<Difficulty>(selectedDifficulty);
            game.reset();
            game.setGameState(GameState::PLAYING);
        }
    }

    if (Input::isArrowUp())
        selectedDifficulty = (selectedDifficulty - 1 + 4) % 4;
    if (Input::isArrowDown())
        selectedDifficulty = (selectedDifficulty + 1) % 4;
}

void MainMenu::drawCredits()
{
    Game::marqueeText("TesekkurlerTesekkurlerTesekkurlerTesekkurlerTesekkurlerTesekkurlerTesekkur"
                      "lerTesekkurler",
                      SCREEN_DRAW_Y + TEXT_HEIGHT * -7, 20, WHITE, 80.f);
    Game::drawTextCenter("Herkese ayri ayri tesekkurlerimi sunuyorum siz olmasaniz", SCREEN_DRAW_X,
                         SCREEN_DRAW_Y + TEXT_HEIGHT * -5, 20, GRAY);
    Game::drawTextCenter("BombKurdistan projesi bu kadar gelisemezdi <3", SCREEN_DRAW_X,
                         SCREEN_DRAW_Y + TEXT_HEIGHT * -4, 20, GRAY);
    Game::drawTextCenter("(dunyanin en iyi oyun projesi)", SCREEN_DRAW_X,
                         SCREEN_DRAW_Y + TEXT_HEIGHT * -3, 20, GRAY);

    Game::drawTextCenter("Programlama", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -1, 20,
                         LIGHTGRAY);
    Game::drawTextCenter("XielQ", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 0, 20, WHITE);

    Game::drawTextCenter("Ozel Tesekkurler", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 2, 20,
                         LIGHTGRAY);

    Game::drawTextCenter("larei <3", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 3, 20, WHITE);
    Game::drawTextCenter("kosero <3", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 4, 20, WHITE);
    Game::drawTextCenter("yesil asya <3", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 5, 20,
                         WHITE);

    Game::drawTextCenter("Geri Don", SCREEN_DRAW_X, SCREEN_HEIGHT - TEXT_HEIGHT * 4, 20, YELLOW);

    Game::drawTextCenter("Bu oyun tamamen eglence amaciyla yapilmistir", SCREEN_DRAW_X,
                         SCREEN_HEIGHT - TEXT_HEIGHT * 2, 20, GRAY);
}
