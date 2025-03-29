#include "Bullet.hpp"

Bullet::Bullet(Vector2 position, Vector2 direction, float speed)
    : position(position), direction(direction), speed(speed), active(true) {}

void Bullet::update() {
    if (active) {
        position.x += direction.x * speed;
        position.y += direction.y * speed;
    }
}

void Bullet::draw() {
    if (active) {
        DrawCircle(position.x, position.y, BULLET_SIZE, RED);
    }
}
