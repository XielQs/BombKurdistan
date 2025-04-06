#pragma once
#ifndef BOSSATTACK_HPP
#define BOSSATTACK_HPP

#include "raylib.h"
#include <vector>
#include <raymath.h>
#include <memory>
#include "Player.hpp"
#include "Bullet.hpp"

enum AttackSize { SMALL, MEDIUM, LARGE };

class BossAttack
{
public:
    BossAttack(Vector2 position, AttackSize size);

    Vector2 position;
    AttackSize size;
    float explodeTime;
    bool exploded;
    std::vector<std::unique_ptr<Bullet>> bullets;

    void draw() const;
    void update(Player& player);
    bool isAlive() const;
    void explode();
};

#endif // BOSSATTACK_HPP
