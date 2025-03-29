#ifndef BULLET_H
#define BULLET_H
#define BULLET_SIZE 5
#include <raylib.h>

class Bullet {
public:
    Vector2 position;
    Vector2 direction;
    float speed;
    bool active;

    Bullet(Vector2 pos, Vector2 dir, float spd);
    void update();
    void draw();
};

#endif // BULLET_H
