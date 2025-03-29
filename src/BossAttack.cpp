#include "BossAttack.hpp"
#include "Difficulty.hpp"
#include <raymath.h>
#include <algorithm>

float getAttackSizeRadius(AttackSize size)
{
    switch (size) {
        case AttackSize::SMALL: return 25.f;
        case AttackSize::MEDIUM: return 30.f;
        case AttackSize::LARGE: return 50.f;
        default: return 0.f;
    }
}

BossAttack::BossAttack(Vector2 position, AttackSize size):
    position(position),
    size(size),
    explodeTime(0.f),
    exploded(false)
{
    float factor = (currentDifficulty == EASY) ? 2.0f : (currentDifficulty == NORMAL) ? 1.f : 0.5f;
    explodeTime = GetTime() + ((size == AttackSize::SMALL) ? 0.5f : (size == AttackSize::MEDIUM) ? 1.0f : 1.5f) * factor;
}

void BossAttack::draw()
{
    if (!exploded) {
        Color color = (size == AttackSize::SMALL) ? RED : (size == AttackSize::MEDIUM) ? YELLOW : BLUE;
        float radius = getAttackSizeRadius(size);
        DrawCircleLines(position.x, position.y, radius, color);
    }
    for (Bullet& bullet : bullets) {
        bullet.draw();
    }
}

bool BossAttack::isAlive()
{
    return !exploded || !bullets.empty();
}

void BossAttack::update(Player& player)
{
    if (!exploded && GetTime() > explodeTime) {
        explode();
    }

    for (Bullet& bullet : bullets) {
        bullet.update();
    }

    for (Bullet& bullet : bullets) {
        if (bullet.active && CheckCollisionCircles(bullet.position, BULLET_SIZE, player.position, PLAYER_SIZE - 5.f)) { // -5.f to avoid false positives
            TraceLog(LOG_INFO, "Bullet hit player at: (%f, %f)", bullet.position.x, bullet.position.y);
            player.takeDamage(5.f);
            bullet.active = false;
        }
        if (bullet.position.x < 0 || bullet.position.x > GetScreenWidth() || bullet.position.y < 0 || bullet.position.y > GetScreenHeight()) {
            bullet.active = false;
        }
    }

    // remove inactive bullets
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](Bullet& bullet) { return !bullet.active; }), bullets.end());
}

void BossAttack::explode()
{
    exploded = true;
    float radius = getAttackSizeRadius(size);
    int bulletCount = (size == AttackSize::SMALL) ? 8 : (size == AttackSize::MEDIUM) ? 10 : 12;
    float factor = (currentDifficulty == EASY) ? 0.8f : (currentDifficulty == NORMAL) ? 1.f : 1.5f;
    int bulletSpeed = ((size == AttackSize::SMALL) ? 6 : (size == AttackSize::MEDIUM) ? 4 : 3) * factor;
    float angleStep = 360.0f / bulletCount;

    for (int i = 0; i < bulletCount; i++) {
        float angle = angleStep * i;
        Vector2 dir = { cosf(DEG2RAD * angle), sinf(DEG2RAD * angle) };
        bullets.push_back(Bullet({ position.x + dir.x * radius, position.y + dir.y * radius }, dir, bulletSpeed));
    }
}