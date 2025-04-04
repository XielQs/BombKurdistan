#ifndef GAME_HPP
#define GAME_HPP

#include <vector>
#include <math.h>
#include "raylib.h"
#ifdef __linux__
#include <discordrpc.h>
#endif
#include "Player.hpp"
#include "Boss.hpp"
#include "Bomb.hpp"
#include "BossAttack.hpp"

enum GameState {
    PLAYING,
    GAME_OVER,
    MAIN_MENU,
    MENU_CREDITS,
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
    void updateFrame();
    void setGameState(GameState newState);
    Player* player;

private:
    GameState gameState;
    // we set it to PLAYING as Game::update() needs to be called at least once with MAIN_MENU state
    GameState lastGameState = PLAYING;
    #ifdef __linux__
    DiscordRPC discord;
    DiscordActivity discordActivity;
    #endif
    bool isMuted;
    float bombTimer;
    float attackTimer;
    float timeStart;
    float timeEnd;
    Boss* boss;
    std::vector<BossAttack> bossAttacks;
    std::vector<Bomb> bombs;
    Texture2D bossTexture;
    Texture2D playerTexture;
    Texture2D bombTexture;
    Texture2D lareiTexture;
    Music bgMusic;

    void createAttack();
    void drawTextCenter(const char* text, float x, float y, float fontSize, Color color);
    void drawTextCombined(float x, float y, float fontSize, const char* text1, Color color1, const char* text2, Color color2);
    void marqueeText(const char* text, float y, float fontSize, Color color, float speed);
    const char* formatTime();
};

#endif // GAME_HPP
