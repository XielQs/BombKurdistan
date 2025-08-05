#include "Player.hpp"

#include "Constants.hpp"
#include "GlobalBounds.hpp"
#include "Input.hpp"
#include "raylib.h"
#include "raymath.h"

#include <algorithm>
#include <cmath>

Player::Player(const Texture2D &texture) : texture(texture)
{
    init();
}

void Player::init()
{
    health = PLAYER_HEALTH;
    position = {GetScreenWidth() / 2.f, GetScreenHeight() / 2.f};
    previousPosition = position;
    velocity = {0, 0};
}

void Player::draw() const
{
    const Rectangle src = {0.f, 0.f, static_cast<float>(texture.width),
                           static_cast<float>(texture.height)};
    const Rectangle dest = {position.x, position.y, PLAYER_SIZE, PLAYER_SIZE};

    DrawTexturePro(texture, src, dest, {PLAYER_SIZE / 2.f, PLAYER_SIZE / 2.f}, 0.f, WHITE);

    // draw health bar
    const float healthBar = health / PLAYER_HEALTH;
    constexpr float barWidth = 130.f;
    constexpr float barHeight = 15.f;
    DrawRectangle(SCREEN_DRAW_X - barWidth / 2, GetScreenHeight() - barHeight - 10.f, barWidth,
                  barHeight, GRAY);
    DrawRectangle(SCREEN_DRAW_X - barWidth / 2, GetScreenHeight() - barHeight - 10.f,
                  barWidth * healthBar, barHeight, YELLOW);

    DrawText(TextFormat("HP: %.0f", health), SCREEN_DRAW_X - barWidth / 2 + 5.f,
             GetScreenHeight() - barHeight - 10.f + 2.f, 10, DARKGRAY);

#ifdef DEBUG_MODE
    // draw invisible bounds
    DrawRectangleLinesEx((Rectangle) {movementBounds.left - PLAYER_SIZE,
                                      movementBounds.top - PLAYER_SIZE,
                                      movementBounds.right - movementBounds.left + PLAYER_SIZE * 2,
                                      movementBounds.bottom - movementBounds.top + PLAYER_SIZE * 2},
                         2.f, RED);
    // draw player bounds
    DrawCircleLinesV(position, PLAYER_COLLISION_RADIUS, BLUE);
#endif
}

void Player::update()
{
    previousPosition = position;
    Vector2 input = {0, 0};

    if (Input::isLeftButton()) {
        mouseTarget = GetMousePosition();
        isMouseTargetSet = true;
    }

    // add gamepad support
    if (IsGamepadAvailable(0)) {
        Vector2 gamepadAxis = {GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X),
                               GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y)};

        if (Vector2LengthSqr(gamepadAxis) > 0.01f) {
            input.x += gamepadAxis.x;
            input.y += gamepadAxis.y;
        }
    }

    // update player position
    if (Input::isPlayerUp()) {
        input.y -= 1.f;
        resetMouseTarget();
    }
    if (Input::isPlayerDown()) {
        input.y += 1.f;
        resetMouseTarget();
    }
    if (Input::isPlayerLeft()) {
        input.x -= 1.f;
        resetMouseTarget();
    }
    if (Input::isPlayerRight()) {
        input.x += 1.f;
        resetMouseTarget();
    }

    if (isMouseTargetSet) {
        Vector2 direction = Vector2Subtract(mouseTarget, position);
        if (Vector2Length(direction) > 2.5f) {
            input = Vector2Normalize(direction);
        } else {
            resetMouseTarget(); // reset if mouse target is reached
        }
    }

    if (input.x != 0.f || input.y != 0.f) {
        const float magnitude = Vector2Length(input);
        position.x += (input.x / magnitude) * PLAYER_SPEED * GetFrameTime();
        position.y += (input.y / magnitude) * PLAYER_SPEED * GetFrameTime();
    }

    position.x = std::clamp(position.x, movementBounds.left, movementBounds.right);
    position.y = std::clamp(position.y, movementBounds.top, movementBounds.bottom);

    velocity = {position.x - previousPosition.x, position.y - previousPosition.y};
}

void Player::takeDamage(float damage)
{
    health = fmax(health - damage, 0.0f);

// we are currently using GLFW instead of SDL2
// the only backend that supports gamepad vibration is SDL2
// so we are using this code to make it work only on SDL2!!
#ifdef PLATFORM_DESKTOP_SDL
    if (IsGamepadAvailable(0)) {
        SetGamepadVibration(0, 0.7f, 0.7f, 0.2f);
    }
#endif
}

void Player::resetMouseTarget()
{
    isMouseTargetSet = false;
    mouseTarget = {0, 0};
}
