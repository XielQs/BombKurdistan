#ifndef BULLET_HPP
#define BULLET_HPP

#include "raylib.h"

class Bullet
{
public:
    Vector2 position;
    Vector2 direction;
    float speed;
    bool active;

    Bullet(Vector2 position, Vector2 direction, float speed);
    void update();
    void draw() const;
};

#endif // BULLET_HPP
