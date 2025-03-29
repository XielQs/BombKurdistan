#ifndef BOSSATTACK_H
#define BOSSATTACK_H
#include <raylib.h>
#include <vector>
#include "Player.hpp"
#include "Bullet.hpp"

enum AttackSize
{
    SMALL,
    MEDIUM,
    LARGE
};

class BossAttack
{
public:
    Vector2 position;
    AttackSize size;
    float explodeTime;
    bool exploded;
    std::vector<Bullet> bullets;

    BossAttack(Vector2 position, AttackSize size);
    void draw();
    void update(Player& player);
    bool isAlive();
    void explode();
};

#endif // BOSSATTACK_H
