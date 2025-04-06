#pragma once
#ifndef BOMB_HPP
#define BOMB_HPP

#include "raylib.h"
#include "Player.hpp"
#include "Boss.hpp"

class Bomb
{
public:
    Bomb(Texture2D texture, Vector2 position);

    Vector2 position;

    void draw() const;
    void update(Player& player, Boss& boss, float deltaTime);
    bool isAlive() const;
    void explode(Boss& boss);
private:
    Texture2D texture;
    float expireTime;
    float currentScale;
};

#endif // BOMB_HPP
