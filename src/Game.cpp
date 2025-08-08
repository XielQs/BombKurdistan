#include "Game.hpp"

#include "Constants.hpp"
#include "Difficulty.hpp"
#include "GlobalBounds.hpp"
#include "Input.hpp"
#include "MainMenu.hpp"
#include "PauseScreen.hpp"
#include "Settings.hpp"
#include "raylib.h"
#include "raymath.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <memory>

Game::Game()
    : player(nullptr), shouldClose(false), gameState(GameState::MAIN_MENU), bombTimer(0.f),
      attackTimer(0.f), timeStart(GetTime()), timeEnd(0.f), boss(nullptr), isShaking(false)
{
}

void Game::init()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Kurdistan Bombalayici");
    SetExitKey(KEY_NULL); // disable ESC key
    InitAudioDevice();
    InitMovementBounds(GetScreenWidth(), GetScreenHeight());
    SetWindowIcon(LoadImage("assets/icon.png"));

    bossTexture = LoadTexture("assets/boss.png");
    playerTexture = LoadTexture("assets/player.png");
    bombTexture = LoadTexture("assets/bomb.png");
    lareiTexture = LoadTexture("assets/larei.png");
    bgMusic = LoadMusicStream("assets/bg_music.mp3");

    windowPos = GetWindowPosition();

    if (bossTexture.id <= 0 || playerTexture.id <= 0 || bombTexture.id <= 0 ||
        lareiTexture.id <= 0 || bgMusic.ctxType == 0) {
        TraceLog(LOG_ERROR, "Failed to load textures");
        setGameState(GameState::GAME_ERROR_TEXTURE);
    } else
        Settings::init();

    player = std::make_unique<Player>(playerTexture);
    boss = std::make_unique<Boss>(bossTexture, lareiTexture);
}

void Game::reset()
{
    gameState = GameState::MAIN_MENU;
    bombTimer = 0.f;
    attackTimer = 0.f;
    timeStart = GetTime();
    timeEnd = 0.f;
    bossAttacks.clear();
    bombs.clear();
    isShaking = false;

    if (player)
        player->init();
    if (boss)
        boss->init();

    StopMusicStream(bgMusic);
}

void Game::update()
{
    const bool shouldPlay = (gameState == GameState::PLAYING && !Settings::config.muteMusic);

    if (shouldPlay && !IsMusicStreamPlaying(bgMusic)) {
        PlayMusicStream(bgMusic);
    } else if (!shouldPlay && IsMusicStreamPlaying(bgMusic)) {
        StopMusicStream(bgMusic);
    }

    UpdateMusicStream(bgMusic);

    switch (gameState) {
        case GameState::PLAYING:
            Input::unlockMouse();

            updateTimers();
            if (player)
                player->update();
            if (boss)
                boss->update(GetFrameTime());

            // we are not using elements.erase(elements.begin() + i) because it has O(n²) complexity
            // instead we are using std::erase_if to remove all inactive elements

            // update boss attacks
            for (size_t i = 0; i < bossAttacks.size(); ++i) {
                BossAttack &attack = *bossAttacks[i];
                attack.update(*player);
                if (!attack.isAlive()) {
                    std::erase_if(bossAttacks, [](auto &attk) { return !attk->isAlive(); });
                    --i;
                }
            }

            // update bombs
            for (size_t i = 0; i < bombs.size(); ++i) {
                Bomb &bomb = *bombs[i];
                bomb.update(*player, *boss, GetFrameTime());
                if (!bomb.isAlive()) {
                    std::erase_if(bombs, [](auto &bmb) { return !bmb->isAlive(); });
                    --i;
                }
            }

            if (player->health <= 0.f)
                setGameState(GameState::GAME_OVER);
            if (boss->health <= 0.f)
                setGameState(GameState::WIN);

            // shake the window
            if (isShaking) {
                const float remainingTime = shakeEndTime - GetTime();
                if (remainingTime > 0) {
                    const float duration =
                        shakeEndTime - (shakeEndTime - remainingTime); // shake duration
                    const float progress = remainingTime / duration;   // shake progress

                    const float currentIntensity =
                        shakeIntensity * progress; // intensity decreases over time

                    // we are using a random angle to shake the window
                    // shake the window in a circle
                    const float angle = GetRandomValue(0, 360) * DEG2RAD;
                    const float offsetX = cosf(angle) * currentIntensity;
                    const float offsetY = sinf(angle) * currentIntensity;

                    SetWindowPosition(windowPos.x + offsetX, windowPos.y + offsetY);
                } else {
                    // reset window position
                    SetWindowPosition(windowPos.x, windowPos.y);
                    isShaking = false;
                }
            }
            break;
        default:
            player->resetMouseTarget(); // reset mouse target when not playing
            break;
    }

    if (gameState != lastGameState) {
        switch (gameState) {
            case GameState::MAIN_MENU:
                TraceLog(LOG_INFO, "Main menu");
                setDiscordActivity("Ana menude", nullptr, GetTime() / 1000);
                break;
            case GameState::PLAYING:
                TraceLog(LOG_INFO, "Game started");
                setDiscordActivity(getDifficultyName(currentDifficulty), "Kurdistani Bombaliyor",
                                   timeStart / 1000);
                break;
            case GameState::WIN:
                TraceLog(LOG_INFO, "Game won");
                setDiscordActivity(getDifficultyName(currentDifficulty), "Ankara kurtarildi!",
                                   GetTime() / 1000);
                break;
            case GameState::GAME_OVER:
                TraceLog(LOG_INFO, "Game over");
                setDiscordActivity(getDifficultyName(currentDifficulty), "Ankara dustu!",
                                   GetTime() / 1000);
                break;
            default:
                break;
        }

        lastGameState = gameState;
    }
}

void Game::draw() const
{
    BeginDrawing();
    ClearBackground(Color{10, 10, 10, 255});

    switch (gameState) {
        case GameState::PLAYING:
            for (const auto &bomb : bombs)
                bomb->draw();
            for (const auto &attack : bossAttacks)
                attack->draw();

            boss->draw();
            player->draw();

            // we are using DrawText instead of drawTextCenter to avoid text scaling issues

            // draw menu items
            DrawText(TextFormat("Zaman: %s", formatTime()), GetScreenWidth() - TEXT_HEIGHT * 6.5f,
                     TEXT_HEIGHT * 0.5, 20, WHITE);
            DrawText(TextFormat("FPS: %d", GetFPS()), GetScreenWidth() - TEXT_HEIGHT * 3,
                     GetScreenHeight() - TEXT_HEIGHT, 18, WHITE);
            break;
        case GameState::GAME_OVER:
            drawTextCenter("Beceriksizsin", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -2, 20,
                           RED);
            drawTextCenter("Senin yuzunden kurtler bagimsizlasip isyan cikartti ve", SCREEN_DRAW_X,
                           SCREEN_DRAW_Y, 20, WHITE);
            drawTextCenter("baskent Ankara dustu, Allah belani versin.", SCREEN_DRAW_X,
                           SCREEN_DRAW_Y + TEXT_HEIGHT, 20, WHITE);
            drawTextCenter("Adam gibi oynayacaksan R'ye bas", SCREEN_DRAW_X,
                           SCREEN_DRAW_Y + TEXT_HEIGHT * 4, 20, WHITE);
            drawTextCenter("Kurt isen ESC atabilirsin", SCREEN_DRAW_X,
                           SCREEN_DRAW_Y + TEXT_HEIGHT * 5, 20, WHITE);
            break;
        case GameState::WIN:
            drawTextCenter("Helal Olsun!", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -2, 20,
                           GREEN);
            drawTextCenter("Senin sayende xtinfirev ve kurtler dalgayi aldi!", SCREEN_DRAW_X,
                           SCREEN_DRAW_Y, 20, WHITE);
            drawTextCenter("Artik Turkiye Cumhuriyeti cok daha mutlu bir devlet olabilir",
                           SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT, 20, WHITE);
            drawTextCenter("YASASIN TURKIYE CUMHURIYETI!", SCREEN_DRAW_X,
                           SCREEN_DRAW_Y + TEXT_HEIGHT * 2, 20, RED);
            drawTextCenter("Kurdistani tekrar bombalamak icin R'ye bas", SCREEN_DRAW_X,
                           SCREEN_DRAW_Y + TEXT_HEIGHT * 4, 20, WHITE);
            drawTextCenter("Mutlu bir sekilde ayrilmak icin ESC'ye bas", SCREEN_DRAW_X,
                           SCREEN_DRAW_Y + TEXT_HEIGHT * 5, 20, WHITE);
            drawTextCenter(TextFormat("Bombalanan Sure: %s", formatTime()), SCREEN_DRAW_X,
                           SCREEN_DRAW_Y + TEXT_HEIGHT * 7, 20, WHITE);
            drawTextCombined(SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 8, 20,
                             {{"Oyun Modu:", WHITE},
                              {getDifficultyName(currentDifficulty),
                               currentDifficulty == Difficulty::EASY     ? GREEN
                               : currentDifficulty == Difficulty::NORMAL ? YELLOW
                                                                         : DARKRED}});
            break;
        case GameState::MAIN_MENU:
            MainMenu::draw();
            break;
        case GameState::PAUSED:
            PauseScreen::draw();
            break;
        case GameState::GAME_ERROR_TEXTURE:
            drawTextCenter("Bir hata olustu", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -2, 20,
                           RED);
            drawTextCenter("Gerekli dosyalar eksik veya bozuk", SCREEN_DRAW_X,
                           SCREEN_DRAW_Y + TEXT_HEIGHT * 0, 20, WHITE);
            drawTextCenter("Lutfen dosyalari kontrol edin ve tekrar deneyin", SCREEN_DRAW_X,
                           SCREEN_DRAW_Y + TEXT_HEIGHT * 1, 20, WHITE);
            drawTextCenter("Oyunun ve assets klasorunun ayni konumda oldugundan emin olun",
                           SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 2, 20, WHITE);
            break;
        default:
            break;
    }

    EndDrawing();
}

void Game::handleInput()
{
    switch (gameState) {
        case GameState::PLAYING:
            // gamepad Circle button
            if (Input::isEscapeKey()) {
                TraceLog(LOG_INFO, "Game paused");
                setGameState(GameState::PAUSED);
                return;
            }
#ifdef DEBUG_MODE
            if (Input::isKeyPressed(KEY_I))
                setGameState(GameState::WIN);
            if (Input::isKeyPressed(KEY_O))
                setGameState(GameState::GAME_OVER);
            if (Input::isKeyPressed(KEY_B))
                spawnBomb();
#endif
            break;
        case GameState::MAIN_MENU:
            MainMenu::handleInput();
            break;
        case GameState::PAUSED:
            PauseScreen::handleInput();
            break;
        case GameState::WIN:
        case GameState::GAME_OVER:
            if (Input::isResetKey()) {
                reset();
                setGameState(GameState::PLAYING);
            }
            if (Input::isEscapeKey()) {
                cleanup();
            }
            break;
        case GameState::GAME_ERROR_TEXTURE:
            if (Input::isEscapeKey()) {
                cleanup();
            }
            break;
        default:
            TraceLog(LOG_ERROR, "Unknown game state: %d", gameState);
            break;
    }
}

void Game::updateFrame()
{
    handleInput();
    update();
    draw();
}

void Game::cleanup()
{
    if (shouldClose)
        return;
    // player and boss are unique_ptrs, so we don't need to delete them

    TraceLog(LOG_INFO, "Cleaning up game resources");

    UnloadTexture(bossTexture);
    UnloadTexture(playerTexture);
    UnloadTexture(bombTexture);
    UnloadTexture(lareiTexture);
    StopMusicStream(bgMusic);
    UnloadMusicStream(bgMusic);
    if (IsAudioDeviceReady())
        CloseAudioDevice();
    disconnectDiscord();

    shouldClose = true;
}

void Game::setGameState(GameState newState)
{
    if (newState != GameState::PLAYING && isShaking) {
        isShaking = false;
        SetWindowPosition(windowPos.x, windowPos.y);
    }

    if (newState == GameState::GAME_OVER || newState == GameState::WIN ||
        newState == GameState::PAUSED) {
        // stop the game timer
        timeEnd = GetTime();
    }

    if (newState == GameState::PLAYING && gameState == GameState::PAUSED) {
        timeStart += GetTime() - timeEnd;
        timeEnd = 0.f;
    }

    gameState = newState;
}

void Game::updateTimers()
{
    bombTimer += GetFrameTime();
    attackTimer += GetFrameTime();

    // spawn attacks
    if (attackTimer >= 0.5f) {
        if (GetRandomValue(0, 1) == 0 &&
            (currentDifficulty != Difficulty::HARD ? bossAttacks.size() <= 3 : true)) { // 50%
            const int max = (currentDifficulty == Difficulty::EASY)     ? 2
                            : (currentDifficulty == Difficulty::NORMAL) ? 3
                                                                        : 5;
            const int attackCount = GetRandomValue(1, max);
            for (int j = 0; j < attackCount; ++j)
                createAttack();
        }
        attackTimer = 0.f;
    }

    // spawn bombs randomly
    if (bombTimer >= 5.f) {
        if (GetRandomValue(0, 2) == 0)
            spawnBomb(); // 33%
        bombTimer = 0.f;
    }
}

void Game::createAttack()
{
    auto size = static_cast<AttackSize>(GetRandomValue(0, 2));

    const auto [x, y] = Vector2Normalize(player->velocity);

    const float attackAreaWidth = movementBounds.right - movementBounds.left;
    const float attackAreaHeight = movementBounds.top;

    const Vector2 playerCenter = {player->position.x + player->texture.width / 2.f,
                                  player->position.y + player->texture.height / 2.f};

    Vector2 attackPos = {playerCenter.x + x * attackAreaWidth / 2.f,
                         playerCenter.y + y * attackAreaHeight / 2.f};

    attackPos.x = std::clamp(attackPos.x, playerCenter.x - attackAreaWidth / 2.f,
                             playerCenter.x + attackAreaWidth / 2.f);
    attackPos.y = std::clamp(attackPos.y, playerCenter.y - attackAreaHeight / 2.f,
                             playerCenter.y + attackAreaHeight / 2.f);

    attackPos.x += GetRandomValue(-ATTACK_OFFSET, ATTACK_OFFSET);
    attackPos.y += GetRandomValue(-ATTACK_OFFSET, ATTACK_OFFSET);

    bossAttacks.emplace_back(std::make_unique<BossAttack>(attackPos, size));

    TraceLog(LOG_INFO, "Attack created at position: (%f, %f)", attackPos.x, attackPos.y);
}

void Game::spawnBomb()
{
    const Vector2 bombPos = {
        static_cast<float>(GetRandomValue(movementBounds.left, movementBounds.right)),
        static_cast<float>(GetRandomValue(movementBounds.top, movementBounds.bottom))};

    bombs.emplace_back(std::make_unique<Bomb>(
        bombTexture, Vector2{static_cast<float>(bombPos.x), static_cast<float>(bombPos.y)}));

    TraceLog(LOG_INFO, "Bomb spawned at position: (%f, %f)", bombPos.x, bombPos.y);
}

void Game::shakeWindow(float duration, float intensity)
{
    windowPos = GetWindowPosition();
    shakeEndTime = GetTime() + duration;
    shakeIntensity = intensity;
    isShaking = true;
}

float Game::drawTextCenter(const char *text, float x, float y, float fontSize, Color color)
{
    const float spacing = fontSize / 10;
    const Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, spacing);
    const Vector2 position = {x - textSize.x / 2.f, y - textSize.y / 2.f};
    DrawTextEx(GetFontDefault(), text, position, fontSize, spacing, color);
    return position.x + textSize.x;
}

float Game::drawTextCombined(float x,
                             float y,
                             float fontSize,
                             std::initializer_list<TextSegment> segments)
{
    if (segments.size() == 0)
        return x;
    const float spacing = fontSize / 10;
    const float spaceWidth = fontSize / 2.0f;

    std::vector<Vector2> sizes;
    float totalWidth = 0;
    float maxHeight = 0;
    for (const auto &segment : segments) {
        Vector2 size = MeasureTextEx(GetFontDefault(), segment.text, fontSize, spacing);
        sizes.push_back(size);
        totalWidth += size.x;
        maxHeight = std::max(maxHeight, size.y);
    }

    totalWidth += (segments.size() - 1) * spaceWidth;

    const float startX = x - totalWidth / 2.0f;
    const float startY = y - maxHeight / 2.0f;

    float currentX = startX;
    int i = 0;
    for (const auto &segment : segments) {
        DrawTextEx(GetFontDefault(), segment.text, {currentX, startY}, fontSize, spacing,
                   segment.color);
        currentX += sizes[i].x + spaceWidth;
        i++;
    }

    return startX + totalWidth;
}

void Game::marqueeText(const char *text, float y, float fontSize, Color color, float speed)
{
    static float x = 0;
    const int textWidth = MeasureText(text, fontSize);

    x -= speed * GetFrameTime();

    if (x < -textWidth)
        x += textWidth;

    DrawText(text, x, y, fontSize, color);
    DrawText(text, x + textWidth, y, fontSize, color);
}

void Game::disconnectDiscord()
{
#ifdef DISCORD_RPC_ENABLED
    if (discord.connected)
        DiscordRPC_shutdown(&discord);
#endif
}

void Game::connectDiscord()
{
#ifdef DISCORD_RPC_ENABLED
    if (discord.connected) {
        TraceLog(LOG_INFO, "Discord RPC already connected");
        return;
    }
    DiscordEventHandlers discordHandlers = {};

    discordHandlers.disconnected = [](const bool wasError) {
        if (wasError) {
            TraceLog(LOG_ERROR, "Discord RPC disconnected with error");
        } else {
            TraceLog(LOG_INFO, "Discord RPC disconnected");
        }
    };

    discordHandlers.error = [](const int errorCode, const char *message) {
        TraceLog(LOG_ERROR, "Discord RPC error: %d - %s", errorCode, message);
    };

    discordHandlers.ready = [](const DiscordUser *user) {
        TraceLog(LOG_INFO, "Discord RPC ready: %s", user->username);
    };

    DiscordRPC_init(&discord, "1356073834981097552", &discordHandlers);
    if (!discord.connected)
        TraceLog(LOG_ERROR, "Discord RPC failed to connect: %s", discord.last_error);
    else {
        TraceLog(LOG_INFO, "Discord RPC connected successfully");
        discordActivity.largeImageText = "Kurdistani Bombala";
    }

#endif
}

// TODO: fix time issue when user pauses the game
const char *Game::formatTime() const
{
    const float timeElapsed = (timeEnd > 0 ? timeEnd : GetTime()) - timeStart;
    const int minutes = static_cast<int>(timeElapsed / 60);
    const int seconds = static_cast<int>(timeElapsed) % 60;
    const int milliseconds = static_cast<int>(timeElapsed * 1000) % 1000 / 10;
    return TextFormat("%02d:%02d.%02d", minutes, seconds, milliseconds);
}

void Game::setDiscordActivity(const char *state, const char *details, const float startTimestamp)
{
#ifdef DISCORD_RPC_ENABLED
    if (discord.connected) {
        discordActivity.state = state;
        discordActivity.details = details;
        discordActivity.startTimestamp = static_cast<int64_t>(startTimestamp);
        DiscordRPC_setActivity(&discord, &discordActivity);
    }
#endif
}
