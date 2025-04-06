#include "GlobalBounds.hpp"
#include "Constants.hpp"
#include "Player.hpp"
#include "raymath.h"
#include <algorithm>
#include "raylib.h"
#include <cmath>

Player::Player(Texture2D texture):
    texture(texture)
{
    init();
}

void Player::init() {
    health = PLAYER_HEALTH;
    position = {GetScreenWidth() / 2.f, GetScreenHeight() / 2.f};
    previousPosition = position;
    velocity = {0, 0};
}

void Player::draw()
{
    const Rectangle src = { 0.f, 0.f, static_cast<float>(texture.width), static_cast<float>(texture.height) };
    const Rectangle dest = {
        position.x,
        position.y,
        PLAYER_SIZE,
        PLAYER_SIZE
    };

    DrawTexturePro(texture, src, dest, { PLAYER_SIZE / 2.f, PLAYER_SIZE / 2.f }, 0.f, WHITE);

    // draw health bar
    float healthBar = health / PLAYER_HEALTH;
    float barWidth = 130.f;
    float barHeight = 15.f;
    DrawRectangle(SCREEN_DRAW_X - barWidth / 2, GetScreenHeight() - barHeight - 10.f, barWidth, barHeight, GRAY);
    DrawRectangle(SCREEN_DRAW_X - barWidth / 2, GetScreenHeight() - barHeight - 10.f, barWidth * healthBar, barHeight, YELLOW);

    DrawText(TextFormat("HP: %.0f", health), SCREEN_DRAW_X - barWidth / 2 + 5.f, GetScreenHeight() - barHeight - 10.f + 2.f, 10, DARKGRAY);

    #ifdef DEBUG_MODE
    // draw invisible bounds
    DrawRectangleLinesEx(
        (Rectangle){
            movementBounds.left - PLAYER_SIZE,
            movementBounds.top - PLAYER_SIZE,
            movementBounds.right - movementBounds.left + PLAYER_SIZE * 2,
            movementBounds.bottom - movementBounds.top + PLAYER_SIZE * 2
        },
        2.f,
        RED
    );
    // draw player bounds
    DrawCircleLinesV(position, PLAYER_COLLISION_RADIUS, BLUE);
    #endif
}

void Player::update()
{
    previousPosition = position;
    Vector2 input = {0, 0};

    // update player position
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) input.y -= 1.f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) input.y += 1.f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) input.x -= 1.f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) input.x += 1.f;

    // add gamepad support
    if (IsGamepadAvailable(0)) {
        Vector2 gamepadAxis = {
            GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X),
            GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y)
        };

        if (Vector2LengthSqr(gamepadAxis) > 0.01f) {
            input.x += gamepadAxis.x;
            input.y += gamepadAxis.y;
        }

        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_UP)) input.y -= 1.f;
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) input.y += 1.f;
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) input.x -= 1.f;
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) input.x += 1.f;
    }

    if (input.x != 0.f || input.y != 0.f) {
        const float magnitude = Vector2Length(input);
        position.x += (input.x / magnitude) * PLAYER_SPEED * GetFrameTime();
        position.y += (input.y / magnitude) * PLAYER_SPEED * GetFrameTime();
    }

    position.x = std::clamp(position.x, movementBounds.left, movementBounds.right);
    position.y = std::clamp(position.y, movementBounds.top, movementBounds.bottom);

    velocity = { position.x - previousPosition.x, position.y - previousPosition.y };
}

void Player::takeDamage(float damage)
{
    health = fmax(health - damage, 0.0f);

    // we are currently using GLFW instead of SDL2
    // the only backend that supports gamepad vibration is SDL2
    // so we are using this code to make it work on SDL2!!
    #ifdef PLATFORM_DESKTOP_SDL
    if (IsGamepadAvailable(0)) {
        SetGamepadVibration(0, 0.7f, 0.7f, 0.2f);
    }
    #endif
}
