#pragma once
#ifndef MAINMENU_HPP
#define MAINMENU_HPP

enum class MainMenuState { MAIN_MENU, SELECT_DIFFICULTY, CREDITS, SETTINGS };

class MainMenu
{
public:
    static MainMenuState state;

    static void draw();
    static void handleInput();

private:
    static int selectedOption;
    static int selectedDifficulty;
    static constexpr int mainMenuOptions = 4;

    static void drawMainMenu();
    static void handleMainMenuInput();
    static void drawDifficultySelection();
    static void handleDifficultyInput();
    static void drawCredits();
};

#endif // MAINMENU_HPP
