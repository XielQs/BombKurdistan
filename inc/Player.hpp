#ifndef PLAYER_H
#define PLAYER_H
#define PLAYER_HEALTH 100
#define SCREEN_PADDING 20
#define PLAYER_SIZE 24
#define PLAYER_SPEED 5
#include <raylib.h>

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

#endif // PLAYER_H
