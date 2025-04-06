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

private:
    Vector2 previousPosition;
};

#endif // PLAYER_HPP
