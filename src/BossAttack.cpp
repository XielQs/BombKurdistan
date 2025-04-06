#include "BossAttack.hpp"
#include "Constants.hpp"
#include "Difficulty.hpp"
#include <algorithm>
#include <memory>

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
    float factor = (currentDifficulty == EASY) ? BossAttackConfig::EASY_FACTOR : (currentDifficulty == NORMAL) ? BossAttackConfig::NORMAL_FACTOR : BossAttackConfig::HARD_FACTOR;
    explodeTime = GetTime() + (size == AttackSize::SMALL ? BossAttackConfig::SMALL_EXPLODE_TIME : (size == AttackSize::MEDIUM ? BossAttackConfig::MEDIUM_EXPLODE_TIME : BossAttackConfig::LARGE_EXPLODE_TIME)) * factor;
}

void BossAttack::draw() const
{
    if (!exploded) {
        Color color = (size == AttackSize::SMALL) ? RED : (size == AttackSize::MEDIUM) ? YELLOW : BLUE;
        float radius = getAttackSizeRadius(size);
        DrawCircleLines(position.x, position.y, radius, color);
    }
    for (const auto& bullet : bullets) bullet->draw();
}

bool BossAttack::isAlive() const
{
    return !exploded || !bullets.empty();
}

void BossAttack::update(Player& player)
{
    if (!exploded && GetTime() > explodeTime) explode();

    const float screenWidth = static_cast<float>(GetScreenWidth());
    const float screenHeight = static_cast<float>(GetScreenHeight());

    for (auto& bullet : bullets) {
        bullet->update(GetFrameTime());
        if (bullet->active && CheckCollisionCircles(bullet->position, BULLET_SIZE, player.position, PLAYER_COLLISION_RADIUS)) {
            TraceLog(LOG_INFO, "Bullet hit player at: (%f, %f)", bullet->position.x, bullet->position.y);
            player.takeDamage(5.f);
            bullet->active = false;
        }
        if (bullet->position.x < 0 || bullet->position.x > screenWidth || bullet->position.y < 0 || bullet->position.y > screenHeight) {
            bullet->active = false;
        }
    }

    // remove inactive bullets
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const auto& bullet) { return !bullet->active; }), bullets.end());
}

void BossAttack::explode()
{
    exploded = true;
    int bulletCount = (size == AttackSize::SMALL) ? BossAttackConfig::SMALL_BULLET_COUNT : (size == AttackSize::MEDIUM) ? BossAttackConfig::MEDIUM_BULLET_COUNT : BossAttackConfig::LARGE_BULLET_COUNT;
    float factor = (currentDifficulty == EASY) ? BossAttackConfig::EASY_FACTOR : (currentDifficulty == NORMAL) ? BossAttackConfig::NORMAL_FACTOR : BossAttackConfig::HARD_FACTOR;
    float bulletSpeed = ((size == AttackSize::SMALL) ? BossAttackConfig::SMALL_BULLET_SPEED : (size == AttackSize::MEDIUM) ? BossAttackConfig::MEDIUM_BULLET_SPEED : BossAttackConfig::LARGE_BULLET_SPEED) * factor;
    float angleStep = 360.0f / bulletCount;

    for (int i = 0; i < bulletCount; i++) {
        float angle = angleStep * i;
        Vector2 dir = { cosf(DEG2RAD * angle), sinf(DEG2RAD * angle) };
        bullets.emplace_back(std::make_unique<Bullet>(Vector2{ position.x + dir.x, position.y + dir.y }, dir, bulletSpeed));
    }
}
