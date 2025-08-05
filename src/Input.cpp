#include "Input.hpp"

#include "raylib.h"

bool Input::isMouseLocked = false;
bool Input::shouldUnlockMouse = false;

bool Input::isPlayerUp()
{
    return IsKeyDown(KEY_W) || IsKeyDown(KEY_UP) ||
           IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_UP);
}

bool Input::isPlayerDown()
{
    return IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN) ||
           IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN);
}

bool Input::isPlayerLeft()
{
    return IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT) ||
           IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT);
}

bool Input::isPlayerRight()
{
    return IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT) ||
           IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
}

bool Input::isMusicMuted()
{
    // gamepad Square button
    return IsKeyPressed(KEY_M) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT);
}

bool Input::isLeftButton()
{
    if (isMouseLocked) {
        return false; // do not allow left button click if mouse is locked
    }
    return IsMouseButtonDown(MOUSE_BUTTON_LEFT);
}

bool Input::isEscapeKey()
{
    // gamepad Circle button
    return IsKeyPressed(KEY_ESCAPE) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
}

bool Input::isPauseKey()
{
    // gamepad L1 button
    return IsKeyPressed(KEY_P) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_TRIGGER_1);
}

bool Input::isResetKey()
{
    // gamepad X button
    return IsKeyPressed(KEY_R) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
}

bool Input::isCreditsKey()
{
    // gamepad Triangle button
    return IsKeyPressed(KEY_C) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_UP);
}

bool Input::isSettingsKey()
{
    // gamepad L1 button
    return IsKeyPressed(KEY_A) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_TRIGGER_1);
}

bool Input::isEnterOrSpace()
{
    // gamepad X button
    return IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
           IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
}

bool Input::isKeyPressed(KeyboardKey key)
{
    return IsKeyPressed(key);
}

bool Input::isArrowUp()
{
    return IsKeyPressed(KEY_UP) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_UP);
}

bool Input::isArrowDown()
{
    return IsKeyPressed(KEY_DOWN) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN);
}

bool Input::isArrowLeft()
{
    return IsKeyPressed(KEY_LEFT) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT);
}

bool Input::isArrowRight()
{
    return IsKeyPressed(KEY_RIGHT) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
}

void Input::lockMouse()
{
    isMouseLocked = true;
}

void Input::unlockMouse()
{
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        shouldUnlockMouse = true;
    } else if (shouldUnlockMouse && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        shouldUnlockMouse = false;
        isMouseLocked = false;
    }
}
