#ifndef GAME_HPP
#define GAME_HPP

#define TEXT_HEIGHT 25.f
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define ATTACK_OFFSET 150
#define SCREEN_DRAW_X (SCREEN_WIDTH / 2.f)
#define SCREEN_DRAW_Y (SCREEN_HEIGHT / 2.f)
#define DARKRED (Color){ 139, 0, 0, 255 }

#include <vector>
#include <math.h>
#include <raylib.h>
#include <discordrpc.h>
#include "Player.hpp"
#include "Boss.hpp"
#include "Bomb.hpp"
#include "BossAttack.hpp"

enum GameState {
    PLAYING,
    GAME_OVER,
    MAIN_MENU,
    WIN
};

class Game {
public:
    Game();
    ~Game();

    void init();
    void reset();
    void update();
    void draw();
    void handleInput();
    void updateTimers();
    void setGameState(GameState newState);

private:
    GameState gameState;
    bool isMuted;
    float bombTimer;
    float attackTimer;
    float timeStart;
    float timeEnd;
    Player* player;
    Boss* boss;
    std::vector<BossAttack> bossAttacks;
    std::vector<Bomb> bombs;
    Texture2D bossTexture;
    Texture2D playerTexture;
    Texture2D bombTexture;
    Texture2D lareiTexture;
    Sound bgMusic;

    void createAttack();
    void drawTextCenter(const char* text, float x, float y, float fontSize, Color color);
    void drawTextCombined(float x, float y, float fontSize, const char* text1, Color color1, const char* text2, Color color2);
    const char* formatTime();
};

#endif // GAME_HPP
