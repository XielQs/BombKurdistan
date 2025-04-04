#ifndef BOSSATTACK_HPP
#define BOSSATTACK_HPP

#include "raylib.h"
#include <vector>
#include <raymath.h>
#include "Player.hpp"
#include "Bullet.hpp"

enum AttackSize { SMALL, MEDIUM, LARGE };

class BossAttack
{
public:
    Vector2 position;
    AttackSize size;
    float explodeTime;
    bool exploded;
    std::vector<Bullet> bullets;

    BossAttack(Vector2 position, AttackSize size);
    void draw() const;
    void update(Player& player);
    bool isAlive();
    void explode();
};

#endif // BOSSATTACK_HPP
