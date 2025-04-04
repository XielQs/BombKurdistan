#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "raylib.h"

class Player
{
public:
    Player(Texture2D texture);
    void draw();
    void update();
    void init();
    void takeDamage(float damage);
    float health;
    Texture2D texture;
    Vector2 position;
    Vector2 previousPosition;
    Vector2 velocity;
};

#endif // PLAYER_HPP
