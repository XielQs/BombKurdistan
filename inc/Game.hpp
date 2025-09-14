#pragma once
#ifndef GAME_HPP
#define GAME_HPP

#include "Bomb.hpp"
#include "Boss.hpp"
#include "BossAttack.hpp"
#include "Player.hpp"
#include "raylib.h"
#include <memory>
#include <vector>
#ifdef DISCORD_RPC_ENABLED
#include "discordrpc.h"
#endif

enum class GameState { PLAYING, GAME_OVER, WIN, PAUSED, MAIN_MENU, GAME_ERROR_TEXTURE };

struct TextSegment
{
    const char *text;
    Color color;

    TextSegment(const char *t, Color c) : text(t), color(c) {}
};

class Game
{
public:
    Game();
    ~Game() = default;

    std::unique_ptr<Player> player;
    bool shouldClose;
    bool shouldRestart;
    static float gameTime;

    void init();
    void reset();
    void update();
    void draw() const;
    void handleInput();
    void updateTimers();
    void updateFrame();
    void cleanup();
    void setGameState(GameState newState);
    void shakeWindow(float duration, float intensity);
    static float drawTextCenter(const char *text, float x, float y, float fontSize, Color color);
    static float
    drawTextCombined(float x, float y, float fontSize, std::initializer_list<TextSegment> segments);
    static void marqueeText(const char *text, float y, float fontSize, Color color, float speed);
    void disconnectDiscord();
    void connectDiscord();
    Music getBGMusic() const;

private:
    GameState gameState;
    // we set it to PLAYING as Game::update() needs to be called at least once with MAIN_MENU state
    GameState lastGameState = GameState::PLAYING;
#ifdef DISCORD_RPC_ENABLED
    DiscordRPC discord{};
    DiscordActivity discordActivity{};
#endif
    float bombTimer;
    float attackTimer;
    float timeEnd;
    std::unique_ptr<Boss> boss;
    std::vector<std::unique_ptr<BossAttack>> bossAttacks;
    std::vector<std::unique_ptr<Bomb>> bombs;
    Texture2D bossTexture{};
    Texture2D playerTexture{};
    Texture2D bombTexture{};
    Texture2D lareiTexture{};
    Music bgMusic{};
    bool isShaking;
    float shakeEndTime{};
    float shakeIntensity{};
    Vector2 windowPos{};
    int currentFPS = 0;
    float fpsTimer = 0.f;
    int framesThisSecond = 0;

    void createAttack();
    void spawnBomb();
    [[nodiscard]] const char *formatTime() const;
    void setDiscordActivity(const char *state, const char *details, float startTimestamp);
};

#endif // GAME_HPP
