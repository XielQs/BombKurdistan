#include "Player.h"
#include "Boss.h"

Player::Player(Texture2D texture):
    texture(texture)
{
    init();
}

void Player::init() {
    health = PLAYER_HEALTH;
    position.x = GetScreenWidth() / 2;
    position.y = GetScreenHeight() / 2;
    previousPosition.x = position.x;
    previousPosition.y = position.y;
    velocity.x = 0;
    velocity.y = 0;
}

void Player::draw()
{
    Rectangle src = { 0, 0, texture.width - 0.f, texture.height - 0.f };
    Rectangle dest = { position.x, position.y, PLAYER_SIZE, PLAYER_SIZE };

    DrawTexturePro(texture, src, dest, (Vector2){ 0, 0 }, 0.0f, WHITE);
}

void Player::update()
{
    previousPosition = position;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) position.y -= PLAYER_SPEED;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) position.y += PLAYER_SPEED;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) position.x -= PLAYER_SPEED;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) position.x += PLAYER_SPEED;

    if (position.x < SCREEN_PADDING) position.x = SCREEN_PADDING;
    if (position.x > GetScreenWidth() - SCREEN_PADDING * 2) position.x = GetScreenWidth() - SCREEN_PADDING * 2;
    if (position.y < BOSS_HEIGHT + SCREEN_PADDING) position.y = BOSS_HEIGHT + SCREEN_PADDING;
    if (position.y > GetScreenHeight() - texture.height) position.y = GetScreenHeight() - texture.height;

    // show player health
    DrawText(TextFormat("Health: %.0f", health), 10, 10, 20, WHITE);
    velocity.x = position.x - previousPosition.x;
    velocity.y = position.y - previousPosition.y;
}

void Player::takeDamage(float damage)
{
    health -= damage;
    if (health < 0) health = 0;
}
