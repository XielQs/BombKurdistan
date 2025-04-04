#ifndef BOMB_HPP
#define BOMB_HPP

#include "raylib.h"
#include "Player.hpp"
#include "Boss.hpp"

class Bomb
{
public:
    Bomb(Texture2D texture, Vector2 position);
    void draw() const;
    void update(Player& player, Boss& boss);
    bool isAlive();
    void explode(Boss& boss);
    Vector2 position;
    float expireTime;
    Texture2D texture;
};

#endif // BOMB_HPP
