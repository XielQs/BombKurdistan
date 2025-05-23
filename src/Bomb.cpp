#include "Constants.hpp"
#include "raylib.h"
#include "Bomb.hpp"
#include <cmath>

Bomb::Bomb(Texture2D texture, Vector2 position):
    position(position),
    texture(texture),
    expireTime(GetTime() + BOMB_LIFETIME),
    currentScale(1.0f)
{
}

void Bomb::draw() const
{
    if (!isAlive()) return;

    const float scale = currentScale;
    const Vector2 origin = {BOMB_SIZE / 2.f * scale, BOMB_SIZE / 2.f * scale};

    Rectangle src = { 0, 0, static_cast<float>(texture.width), static_cast<float>(texture.height) };
    Rectangle dest = { position.x, position.y, BOMB_SIZE * scale, BOMB_SIZE * scale };

    DrawTexturePro(texture, src, dest, origin, 0.f, WHITE);
    #ifdef DEBUG_MODE
    // draw bomb bounds
    DrawCircleLinesV(position, BOMB_COLLISION_RADIUS, RED);
    #endif
}

void Bomb::update(Player& player, Boss& boss, float deltaTime)
{
    if (!isAlive()) return;

    // pulse animation
    static float animTime = 0.0f;
    animTime += deltaTime * 5.0f;
    currentScale = 1.0f + sinf(animTime) * 0.1f;

    if (CheckCollisionCircles(position, BOMB_COLLISION_RADIUS, player.position, PLAYER_COLLISION_RADIUS)) {
        TraceLog(LOG_INFO, "Bomb exploded at: (%f, %f)", position.x, position.y);
        explode(boss);
    }
}

bool Bomb::isAlive() const
{
    return (GetTime() < expireTime);
}

void Bomb::explode(Boss& boss)
{
    boss.takeDamage(BOMB_DAMAGE);
    expireTime = 0.f;
}
