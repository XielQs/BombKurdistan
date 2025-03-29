#include "Bomb.hpp"

Bomb::Bomb(Texture2D texture, Vector2 position):
    position(position),
    expireTime(GetTime() + 10.f),
    texture(texture)
{
}

void Bomb::draw()
{
    Rectangle src = { 0, 0, texture.width - 0.f, texture.height - 0.f };
    Rectangle dest = { position.x, position.y, 40, 40 };

    DrawTexturePro(texture, src, dest, (Vector2){ 0, 0 }, 0.0f, WHITE);
}

void Bomb::update(Player& player, Boss& boss)
{
    if (isAlive()) {
        float explosionRadius = 20.f;
        if (CheckCollisionCircles(position, explosionRadius, player.position, 24.f)) {
            TraceLog(LOG_INFO, "Bomb exploded at: (%f, %f)", position.x, position.y);
            explode(boss);
        }
    }
}

bool Bomb::isAlive()
{
    return (GetTime() < expireTime);
}

void Bomb::explode(Boss& boss)
{
    boss.takeDamage(BOMB_DAMAGE);
    expireTime = 0.f;
}
