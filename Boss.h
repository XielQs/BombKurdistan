#ifndef BOSS_H
#define BOSS_H
#define BOSS_HEALTH 100
#define BOSS_HEIGHT 200
#include <raylib.h>

class Boss
{
public:
    Boss(Texture2D texture);
    void draw();
    void update();
    void init();
    void takeDamage(float damage);
    float health;
    Texture2D texture;
};

#endif // BOSS_H
