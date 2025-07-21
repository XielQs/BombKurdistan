#pragma once
#ifndef BULLET_HPP
#define BULLET_HPP

#include "raylib.h"

class Bullet
{
public:
    Bullet(Vector2 position, Vector2 direction, float speed);

    bool active;
    Vector2 position;

    void update(float deltaTime);
    void draw() const;

private:
    Vector2 direction;
    Vector2 velocity;
};

#endif // BULLET_HPP
