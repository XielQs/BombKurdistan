#pragma once
#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "raylib.h"

class Player
{
public:
    Player(Texture2D texture);

    float health;
    Texture2D texture;
    Vector2 position;
    Vector2 velocity;

    void draw();
    void update();
    void init();
    void takeDamage(float damage);
    void resetMouseTarget();

private:
    Vector2 previousPosition;
    Vector2 mouseTarget;
    bool isMouseTargetSet;
};

#endif // PLAYER_HPP
