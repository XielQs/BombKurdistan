#pragma once
#ifndef BOMB_HPP
#define BOMB_HPP

#include "Boss.hpp"
#include "Player.hpp"
#include "raylib.h"

class Bomb
{
public:
    Bomb(const Texture2D &texture, Vector2 position);

    Vector2 position;

    void draw() const;
    void update(const Player &player, Boss &boss, float deltaTime);
    [[nodiscard]] bool isAlive() const;
    void explode(Boss &boss);

private:
    Texture2D texture;
    float expireTime;
    float currentScale;
};

#endif // BOMB_HPP
