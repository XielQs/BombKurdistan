#pragma once
#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#define TEXT_HEIGHT 25.f
#define GAME_FPS 60

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define ATTACK_OFFSET 150

#define BULLET_SIZE 5
#define BOMB_DAMAGE 20
#define BOMB_SIZE 40.f
#define BOMB_COLLISION_RADIUS 25.f
#define BOMB_LIFETIME 10.f

#define BOSS_HEALTH 100
#define BOSS_HEIGHT 200

#define PLAYER_HEALTH 100
#define SCREEN_PADDING 20.f
#define PLAYER_SIZE 24.f
#define PLAYER_COLLISION_RADIUS 16.f
#define PLAYER_SPEED GAME_FPS * 5

#define SCREEN_DRAW_X (SCREEN_WIDTH / 2.f)
#define SCREEN_DRAW_Y (SCREEN_HEIGHT / 2.f)
#define DARKRED (Color){139, 0, 0, 255}

namespace BossAttackConfig {
constexpr float EASY_FACTOR = 0.8f;
constexpr float NORMAL_FACTOR = 1.0f;
constexpr float HARD_FACTOR = 1.5f;

constexpr float SMALL_EXPLODE_TIME = 0.5f;
constexpr float MEDIUM_EXPLODE_TIME = 1.0f;
constexpr float LARGE_EXPLODE_TIME = 1.5f;

constexpr int SMALL_BULLET_COUNT = 8;
constexpr int MEDIUM_BULLET_COUNT = 10;
constexpr int LARGE_BULLET_COUNT = 12;

constexpr int SMALL_BULLET_SPEED = 6;
constexpr int MEDIUM_BULLET_SPEED = 4;
constexpr int LARGE_BULLET_SPEED = 3;
} // namespace BossAttackConfig

#endif
