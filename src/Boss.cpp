#include "Boss.hpp"

Boss::Boss(Texture2D texture, Texture2D lareiTexture):
    texture(texture),
    lareiTexture(lareiTexture)
{
    init();
}

void Boss::init()
{
    health = BOSS_HEALTH;
}

void Boss::draw()
{
    Rectangle src = {0.f, 0.f, (float)texture.width, (float)texture.height};
    Rectangle dest = {0.f, 0.f, GetScreenWidth() - 0.f, BOSS_HEIGHT};

    DrawTexturePro(texture, src, dest, (Vector2){0.f, 0.f}, 0.0f, WHITE);
    DrawTexture(lareiTexture, GetScreenWidth() - lareiTexture.width - 50.f, 0, WHITE);
}

void Boss::update()
{
    DrawText(TextFormat("Boss Health: %.0f", health), 10, 40, 20, BLACK);
}

void Boss::takeDamage(float damage)
{
    health -= damage;
    if (health < 0) health = 0;
    TraceLog(LOG_INFO, "Boss took damage: %.0f, remaining health: %.0f", damage, health);
}
