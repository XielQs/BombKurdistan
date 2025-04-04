#ifndef BOSS_HPP
#define BOSS_HPP

#include "raylib.h"

class Boss
{
public:
    Boss(Texture2D texture, Texture2D lareiTexture);
    void draw();
    void update();
    void init();
    void takeDamage(float damage);
    float health;
    Texture2D texture;
    Texture2D lareiTexture;
};

#endif // BOSS_HPP
