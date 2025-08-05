#pragma once
#ifndef GAME_HPP
#define GAME_HPP

#include "Bomb.hpp"
#include "Boss.hpp"
#include "BossAttack.hpp"
#include "Player.hpp"
#include "Settings.hpp"
#include "raylib.h"
#include <memory>
#include <vector>
#ifdef DISCORD_RPC_ENABLED
#include "discordrpc.h"
#endif

enum class GameState {
    PLAYING,
    GAME_OVER,
    MAIN_MENU,
    MENU_CREDITS,
    MENU_SETTINGS,
    GAME_ERROR_TEXTURE,
    WIN
};

class Game
{
public:
    Game();
    ~Game() = default;

    std::unique_ptr<Player> player;
    bool shouldClose;

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
    static int drawTextCenter(const char *text, float x, float y, float fontSize, Color color);
    static int drawTextCombined(float x,
                                float y,
                                float fontSize,
                                const char *text1,
                                Color color1,
                                const char *text2,
                                Color color2);
    static void marqueeText(const char *text, float y, float fontSize, Color color, float speed);
    void disconnectDiscord();
    void connectDiscord();

private:
    Settings settings;
    GameState gameState;
    // we set it to PLAYING as Game::update() needs to be called at least once with MAIN_MENU state
    GameState lastGameState = GameState::PLAYING;
#ifdef DISCORD_RPC_ENABLED
    DiscordRPC discord{};
    DiscordActivity discordActivity{};
#endif
    float bombTimer;
    float attackTimer;
    float timeStart;
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
    bool isPaused;

    void createAttack();
    void spawnBomb();
    [[nodiscard]] const char *formatTime() const;
    void setDiscordActivity(const char *state, const char *details, float startTimestamp);
};

#endif // GAME_HPP
