#include "Boss.hpp"
#include "Constants.hpp"
#include "Difficulty.hpp"
#include "Game.hpp"
#include "raylib.h"
#include <cmath>
#include <unistd.h>

extern Game game;

Boss::Boss(Texture2D texture, Texture2D lareiTexture)
    : texture(texture), lareiTexture(lareiTexture), animTime(0.f), lareiOffsetX(0.f)
{
    init();
}

void Boss::init()
{
    health = BOSS_HEALTH;
}

void Boss::draw() const
{
    const float screenWidth = static_cast<float>(GetScreenWidth());

    DrawTexturePro(
        texture, {0.f, 0.f, static_cast<float>(texture.width), static_cast<float>(texture.height)},
        {0.f, 0.f, screenWidth, BOSS_HEIGHT}, {0.f, 0.f}, 0.f, WHITE);

    const Vector2 lareiPos = {screenWidth - lareiTexture.width + lareiOffsetX,
                              (BOSS_HEIGHT - lareiTexture.height) * 0.5f};
    DrawTextureV(lareiTexture, lareiPos, WHITE);

    const float healthWidth = (GetScreenWidth() - 40.f) * (health / BOSS_HEALTH);
    DrawRectangle(20, BOSS_HEIGHT - 15, healthWidth, 10, RED);
    DrawRectangleLines(20, BOSS_HEIGHT - 15, GetScreenWidth() - 40, 10, DARKGRAY);
}

void Boss::update(float deltaTime)
{
    animTime += deltaTime;

    float animSpeed = 3.f;
    const float animOffset = 5.f;

    if (health < BOSS_HEALTH * 0.3f) {
        animSpeed = 12.f;
    } else if (health < BOSS_HEALTH * 0.6f) {
        animSpeed = 8.f;
    }

    lareiOffsetX = sinf(animTime * animSpeed) * animOffset;

    // if health is less than 30% and difficulty is HARD
    // regenerate health
    if (health < BOSS_HEALTH * 0.3f && currentDifficulty == HARD)
        health += deltaTime * 0.5f;
}

void Boss::takeDamage(float damage)
{
    health = fmax(health - damage, 0.0f);
    TraceLog(LOG_INFO, "Boss took damage: %.0f, remaining health: %.0f", damage, health);

    // shake the window
    game.shakeWindow(0.5f, 10.f);
}
