#pragma once
#ifndef INPUT_HPP
#define INPUT_HPP

#include "raylib.h"

class Input
{
public:
    static bool isPlayerUp();
    static bool isPlayerDown();
    static bool isPlayerLeft();
    static bool isPlayerRight();
    static bool isMusicMuted();
    static bool isLeftButton();
    static bool isEscapeKey();
    static bool isPauseKey();
    static bool isResetKey();
    static bool isCreditsKey();
    static bool isSettingsKey();
    static bool isEnterOrSpace();
    static bool isKeyPressed(KeyboardKey key);
    static bool isArrowUp();
    static bool isArrowDown();
    static bool isArrowLeft();
    static bool isArrowRight();
    static void lockMouse();
    static void unlockMouse();

private:
    static bool isMouseLocked;
    static bool shouldUnlockMouse;
};

#endif // INPUT_HPP
