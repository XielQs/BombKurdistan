#include "Bullet.hpp"
#include "Constants.hpp"
#include "raymath.h"

Bullet::Bullet(Vector2 position, Vector2 direction, float speed)
    : active(true), position(position), direction(Vector2Normalize(direction))
{
    velocity = Vector2Scale(direction, speed * GAME_FPS); // direction * speed
}

void Bullet::update(float deltaTime)
{
    if (!active)
        return;

    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;

    // check if bullet is out of bounds
    if (position.x < 0 || position.x > GetScreenWidth() || position.y < 0 ||
        position.y > GetScreenHeight()) {
        active = false;
    }
}

void Bullet::draw() const
{
    if (active) {
        DrawCircleV(position, BULLET_SIZE, Color{230, 41, 55, 200}); // translucent red
    }
}
