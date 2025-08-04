#include "BossAttack.hpp"
#include "Constants.hpp"
#include "Difficulty.hpp"
#include <algorithm>
#include <cmath>
#include <memory>

float getAttackSizeRadius(AttackSize size)
{
    switch (size) {
        case SMALL:
            return 25.f;
        case MEDIUM:
            return 30.f;
        case LARGE:
            return 50.f;
        default:
            return 0.f;
    }
}

BossAttack::BossAttack(Vector2 position, AttackSize size)
    : position(position), size(size), explodeTime(0.f), exploded(false)
{
    const float factor = (currentDifficulty == EASY)     ? BossAttackConfig::EASY_FACTOR
                         : (currentDifficulty == NORMAL) ? BossAttackConfig::NORMAL_FACTOR
                                                         : BossAttackConfig::HARD_FACTOR;
    explodeTime =
        GetTime() + (size == SMALL ? BossAttackConfig::SMALL_EXPLODE_TIME
                                   : (size == MEDIUM ? BossAttackConfig::MEDIUM_EXPLODE_TIME
                                                     : BossAttackConfig::LARGE_EXPLODE_TIME)) *
                        factor;
}

void BossAttack::draw() const
{
    if (!exploded) {
        const Color color = (size == SMALL) ? RED : (size == MEDIUM) ? YELLOW : BLUE;
        const float radius = getAttackSizeRadius(size);
        DrawCircleLines(position.x, position.y, radius, color);
    }
    for (const auto &bullet : bullets)
        bullet->draw();
}

bool BossAttack::isAlive() const
{
    return !exploded || !bullets.empty();
}

void BossAttack::update(Player &player)
{
    if (!exploded && GetTime() > explodeTime)
        explode();

    const auto screenWidth = static_cast<float>(GetScreenWidth());
    const auto screenHeight = static_cast<float>(GetScreenHeight());

    for (const auto &bullet : bullets) {
        bullet->update(GetFrameTime());
        if (bullet->active && CheckCollisionCircles(bullet->position, BULLET_SIZE, player.position,
                                                    PLAYER_COLLISION_RADIUS)) {
            TraceLog(LOG_INFO, "Bullet hit player at: (%f, %f)", bullet->position.x,
                     bullet->position.y);
            player.takeDamage(5.f);
            bullet->active = false;
        }
        if (bullet->position.x < 0 || bullet->position.x > screenWidth || bullet->position.y < 0 ||
            bullet->position.y > screenHeight) {
            bullet->active = false;
        }
    }

    // remove inactive bullets
    std::erase_if(bullets, [](const auto &bullet) { return !bullet->active; });
}

void BossAttack::explode()
{
    exploded = true;
    const int bulletCount = (size == SMALL)    ? BossAttackConfig::SMALL_BULLET_COUNT
                            : (size == MEDIUM) ? BossAttackConfig::MEDIUM_BULLET_COUNT
                                               : BossAttackConfig::LARGE_BULLET_COUNT;
    const float factor = (currentDifficulty == EASY)     ? BossAttackConfig::EASY_FACTOR
                         : (currentDifficulty == NORMAL) ? BossAttackConfig::NORMAL_FACTOR
                                                         : BossAttackConfig::HARD_FACTOR;
    const float bulletSpeed = ((size == SMALL)    ? BossAttackConfig::SMALL_BULLET_SPEED
                               : (size == MEDIUM) ? BossAttackConfig::MEDIUM_BULLET_SPEED
                                                  : BossAttackConfig::LARGE_BULLET_SPEED) *
                              factor;
    const float angleStep = 360.0f / bulletCount;

    for (int i = 0; i < bulletCount; i++) {
        const float angle = angleStep * i;
        Vector2 dir = {cosf(DEG2RAD * angle), sinf(DEG2RAD * angle)};
        bullets.emplace_back(std::make_unique<Bullet>(
            Vector2{position.x + dir.x, position.y + dir.y}, dir, bulletSpeed));
    }
}
