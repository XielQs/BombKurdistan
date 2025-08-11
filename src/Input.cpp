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

bool Input::isMouseLeftButtonDown()
{
    if (isMouseLocked || !IsWindowFocused()) {
        return false; // do not allow left button click if mouse is locked or window is not focused
    }
    return IsMouseButtonDown(MOUSE_BUTTON_LEFT);
}

bool Input::isMouseRightButtonDown()
{
    if (isMouseLocked || !IsWindowFocused()) {
        return false; // do not allow right button click if mouse is locked or window is not focused
    }
    return IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
}

bool Input::isMouseLeftButton()
{
    if (isMouseLocked || !IsWindowFocused()) {
        return false; // do not allow left button click if mouse is locked or window is not focused
    }
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

bool Input::isMouseRightButton()
{
    if (isMouseLocked || !IsWindowFocused()) {
        return false; // do not allow right button click if mouse is locked or window is not focused
    }
    return IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
}

bool Input::isEscapeKey()
{
    // gamepad Circle button
    return IsKeyPressed(KEY_ESCAPE) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT) ||
           isMouseRightButton();
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

bool Input::isEnterOrSpace()
{
    // gamepad X button
    return IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
           IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) || isMouseLeftButton();
}

bool Input::isKeyPressed(KeyboardKey key)
{
    return IsKeyPressed(key);
}

bool Input::isArrowUp()
{
    // mouse scrolling up
    return IsKeyPressed(KEY_UP) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_UP) ||
           GetMouseWheelMove() > 0;
}

bool Input::isArrowDown()
{
    // mouse scrolling down
    return IsKeyPressed(KEY_DOWN) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN) ||
           GetMouseWheelMove() < 0;
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
