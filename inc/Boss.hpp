#pragma once
#ifndef BOSS_HPP
#define BOSS_HPP

#include "raylib.h"

class Boss
{
public:
    Boss(const Texture2D &texture, const Texture2D &lareiTexture);

    float health{};

    void draw() const;
    void update(float deltaTime);
    void init();
    void takeDamage(float damage);

private:
    Texture2D texture;
    Texture2D lareiTexture;
    float animTime;
    float lareiOffsetX;
};

#endif // BOSS_HPP
