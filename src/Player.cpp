#include "Constants.hpp"
#include "Player.hpp"
#include "raylib.h"
#include <cmath>

Player::Player(Texture2D texture):
    texture(texture)
{
    init();
}

void Player::init() {
    health = PLAYER_HEALTH;
    position.x = GetScreenWidth() / 2.f;
    position.y = GetScreenHeight() / 2.f;
    previousPosition.x = position.x;
    previousPosition.y = position.y;
    velocity.x = 0;
    velocity.y = 0;
}

void Player::draw()
{
    Rectangle src = { 0, 0, texture.width - 0.f, texture.height - 0.f };
    Rectangle dest = { position.x, position.y, PLAYER_SIZE, PLAYER_SIZE };

    DrawTexturePro(texture, src, dest, (Vector2){ 0, 0 }, 0.0f, WHITE);
}

void Player::update()
{
    previousPosition = position;
    // update player position
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP) || IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_UP)) position.y -= PLAYER_SPEED * GetFrameTime();
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN) || IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) position.y += PLAYER_SPEED * GetFrameTime();
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT) || IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) position.x -= PLAYER_SPEED * GetFrameTime();
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT) || IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) position.x += PLAYER_SPEED * GetFrameTime();

    // add gamepad support
    if (IsGamepadAvailable(0)) {
        Vector2 gamepadAxis = { GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X), GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) };
        if (fabs(gamepadAxis.x) > 0.1f || fabs(gamepadAxis.y) > 0.1f) {
            position.x += gamepadAxis.x * PLAYER_SPEED * GetFrameTime();
            position.y += gamepadAxis.y * PLAYER_SPEED * GetFrameTime();
        }
    }

    if (position.x < SCREEN_PADDING) position.x = SCREEN_PADDING;
    if (position.x > GetScreenWidth() - SCREEN_PADDING * 2) position.x = GetScreenWidth() - SCREEN_PADDING * 2;
    if (position.y < BOSS_HEIGHT + SCREEN_PADDING) position.y = BOSS_HEIGHT + SCREEN_PADDING;
    if (position.y > GetScreenHeight() - texture.height) position.y = GetScreenHeight() - texture.height;

    velocity.x = position.x - previousPosition.x;
    velocity.y = position.y - previousPosition.y;
}

void Player::takeDamage(float damage)
{
    // we can't use vibration because it doesn't work on GLFW
    // if (IsGamepadAvailable(0))
    //     SetGamepadVibration(0, 0.5f, 0.5f, 0.5f);
    health -= damage;
    if (health < 0) health = 0;
}
