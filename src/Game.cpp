#include "Game.hpp"
#include "Constants.hpp"
#include "Difficulty.hpp"
#include "GlobalBounds.hpp"
#include "Input.hpp"
#include "raylib.h"
#include "raymath.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <memory>

Game::Game()
    : player(nullptr), shouldClose(false), gameState(MAIN_MENU), isMuted(false), bombTimer(0.f),
      attackTimer(0.f), timeStart(GetTime()), timeEnd(0.f), boss(nullptr), isShaking(false),
      isPaused(false)
{
}

void Game::init()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Kurdistan Bombalayici");
    SetTargetFPS(GAME_FPS);
    SetExitKey(KEY_NULL); // disable ESC key
    InitAudioDevice();
    InitMovementBounds(GetScreenWidth(), GetScreenHeight());
    SetWindowState(FLAG_VSYNC_HINT);
    SetWindowIcon(LoadImage("assets/icon.png"));

#ifdef DISCORD_RPC_ENABLED
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

    bossTexture = LoadTexture("assets/boss.png");
    playerTexture = LoadTexture("assets/player.png");
    bombTexture = LoadTexture("assets/bomb.png");
    lareiTexture = LoadTexture("assets/larei.png");
    bgMusic = LoadMusicStream("assets/bg_music.mp3");

    if (bossTexture.id <= 0 || playerTexture.id <= 0 || bombTexture.id <= 0 ||
        lareiTexture.id <= 0 || bgMusic.ctxType == 0) {
        TraceLog(LOG_ERROR, "Failed to load textures");
        setGameState(GAME_ERROR_TEXTURE);
    }

    windowPos = GetWindowPosition();

    player = std::make_unique<Player>(playerTexture);
    boss = std::make_unique<Boss>(bossTexture, lareiTexture);
}

void Game::reset()
{
    gameState = MAIN_MENU;
    bombTimer = 0.f;
    attackTimer = 0.f;
    timeStart = GetTime();
    timeEnd = 0.f;
    bossAttacks.clear();
    bombs.clear();
    isShaking = false;
    isPaused = false;

    if (player)
        player->init();
    if (boss)
        boss->init();

    StopMusicStream(bgMusic);
}

void Game::update()
{
    if (isPaused)
        return; // do not update if paused

    const bool shouldPlay = (gameState == PLAYING && !isMuted);

    if (shouldPlay && !IsMusicStreamPlaying(bgMusic)) {
        PlayMusicStream(bgMusic);
    } else if (!shouldPlay && IsMusicStreamPlaying(bgMusic)) {
        StopMusicStream(bgMusic);
    }

    UpdateMusicStream(bgMusic);

    if (gameState == PLAYING) {
        Input::unlockMouse();
        if (lastGameState != PLAYING) {
            TraceLog(LOG_INFO, "Game started");
            setDiscordActivity(getDifficultyName(currentDifficulty), "Kurdistani Bombaliyor",
                               timeStart / 1000);
        }

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
            setGameState(GAME_OVER);
        if (boss->health <= 0.f)
            setGameState(WIN);

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
    } else {
        player->resetMouseTarget(); // reset mouse target when not playing
    }
    if (gameState == MAIN_MENU && lastGameState != MAIN_MENU) {
        TraceLog(LOG_INFO, "Main menu");
        setDiscordActivity("Ana menude", nullptr, GetTime() / 1000);
    }
    if (gameState == WIN && lastGameState != WIN) {
        TraceLog(LOG_INFO, "Game won");
        setDiscordActivity(getDifficultyName(currentDifficulty), "Ankara kurtarildi!",
                           GetTime() / 1000);
    }
    if (gameState == GAME_OVER && lastGameState != GAME_OVER) {
        TraceLog(LOG_INFO, "Game over");
        setDiscordActivity(getDifficultyName(currentDifficulty), "Ankara dustu!", GetTime() / 1000);
    }
    lastGameState = gameState;
}

void Game::draw() const
{
    BeginDrawing();
    ClearBackground(Color{10, 10, 10, 255});

    const char *infoSection[] = {
        "Cikmak icin ESC'ye bas",
        "Secim yapmak icin ok tuslarini ya da 1, 2 veya 3'u kullanabilirsin",
        "Oyuna baslamak icin SPACE veya ENTER'a bas",
        "Yapimcilar ve ozel tesekkurler icin C'ye bas",
    };

    switch (gameState) {
        case PLAYING:
            for (const auto &bomb : bombs)
                bomb->draw();
            for (const auto &attack : bossAttacks)
                attack->draw();

            boss->draw();
            player->draw();

            // we are using DrawText instead of drawTextCenter to avoid text scaling issues

            // draw menu items
            DrawText(TextFormat("Time: %s", formatTime()), GetScreenWidth() - TEXT_HEIGHT * 6,
                     TEXT_HEIGHT * 0.5, 20, WHITE);
            DrawText(TextFormat("FPS: %d", GetFPS()), GetScreenWidth() - TEXT_HEIGHT * 3,
                     GetScreenHeight() - TEXT_HEIGHT, 18, WHITE);
            if (isPaused) {
                // blink effect
                if (fmod(GetTime(), 1.0f) < 0.5f)
                    drawTextCenter("PAUSED", SCREEN_DRAW_X, SCREEN_DRAW_Y, 40, WHITE);
            }
            break;
        case GAME_OVER:
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
        case WIN:
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
                             "Oyun Modu:", WHITE, getDifficultyName(currentDifficulty),
                             (currentDifficulty == EASY)     ? GREEN
                             : (currentDifficulty == NORMAL) ? YELLOW
                                                             : DARKRED);
            break;
        case MAIN_MENU:
            drawTextCenter("Kurdistan Bombalayici", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -5,
                           20, WHITE);
            drawTextCenter("Bu bir oyun projesidir tamamiyla eglence amaciyla uretilmistir",
                           SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -4, 20, GRAY);
            // select difficulty
            drawTextCenter("Zorluk Secin", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -2, 20,
                           WHITE);
            drawTextCenter("1. Kurt vatandas", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 0, 20,
                           GREEN);
            drawTextCenter("2. Turk vatandas", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 1, 20,
                           YELLOW);
            drawTextCenter("3. ULKUCU VATANDAS", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 2, 20,
                           DARKRED);

            // draw a (-) to indicate the current difficulty
            if (currentDifficulty == EASY) {
                drawTextCenter("-", SCREEN_DRAW_X - 95, SCREEN_DRAW_Y + TEXT_HEIGHT * 0, 20, GREEN);
            } else if (currentDifficulty == NORMAL) {
                drawTextCenter("-", SCREEN_DRAW_X - 100, SCREEN_DRAW_Y + TEXT_HEIGHT * 1, 20,
                               YELLOW);
            } else if (currentDifficulty == HARD) {
                drawTextCenter("-", SCREEN_DRAW_X - 125, SCREEN_DRAW_Y + TEXT_HEIGHT * 2, 20,
                               DARKRED);
            }

            for (size_t i = 0; i < std::size(infoSection); ++i) {
                drawTextCenter(infoSection[i], SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * (i + 4),
                               20, GRAY);
            }

            drawTextCenter("M ile sesi ac/kapat", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 10,
                           20, isMuted ? GRAY : WHITE);

            if (currentDifficulty == HARD) {
                drawTextCenter("sana guveniyoruz kaptan", SCREEN_DRAW_X,
                               SCREEN_DRAW_Y + TEXT_HEIGHT * 11, 20, DARKRED);
            }
            break;
        case MENU_CREDITS:
            marqueeText("TesekkurlerTesekkurlerTesekkurlerTesekkurlerTesekkurlerTesekkurlerTesekkur"
                        "lerTesekkurler",
                        SCREEN_DRAW_Y + TEXT_HEIGHT * -8, 20, WHITE, 80.f);
            drawTextCenter("Herkese ayri ayri tesekkurlerimi sunuyorum siz olmasaniz",
                           SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -6, 20, GRAY);
            drawTextCenter("BombKurdistan projesi bu kadar gelisemezdi <3", SCREEN_DRAW_X,
                           SCREEN_DRAW_Y + TEXT_HEIGHT * -5, 20, GRAY);
            drawTextCenter("(dunyanin en iyi oyun projesi)", SCREEN_DRAW_X,
                           SCREEN_DRAW_Y + TEXT_HEIGHT * -4, 20, GRAY);

            drawTextCenter("larei <3", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -2, 20, WHITE);
            drawTextCenter("kosero <3", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -1, 20, WHITE);
            drawTextCenter("yesil asya <3", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 0, 20,
                           WHITE);
            drawTextCenter("toby fox <3", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 1, 20,
                           WHITE);

            drawTextCenter("Dosya kaynakcasi:", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 3, 20,
                           WHITE);
            drawTextCenter("boss.png: wikipedia (goruntude oynadim azcik)", SCREEN_DRAW_X,
                           SCREEN_DRAW_Y + TEXT_HEIGHT * 4, 20, GRAY);
            drawTextCenter("bomb.png: stardew valley", SCREEN_DRAW_X,
                           SCREEN_DRAW_Y + TEXT_HEIGHT * 5, 20, GRAY);
            drawTextCenter("player.png: undertale", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 6,
                           20, GRAY);
            drawTextCenter("bg_music.mp3: larei atmisti bir youtube videosundan alinti",
                           SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 7, 20, GRAY);
            drawTextCenter("larei.png: larei.", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 8, 20,
                           GRAY);

            drawTextCenter("Ana menu icin ESC'ye bas", SCREEN_DRAW_X,
                           SCREEN_DRAW_Y + TEXT_HEIGHT * 10, 20, WHITE);
            break;
        case GAME_ERROR_TEXTURE:
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
    // gamepad Square button
    if (Input::isMusicMuted()) {
        isMuted = !isMuted;
        if (isMuted)
            StopMusicStream(bgMusic);
        else if (gameState == PLAYING)
            PlayMusicStream(bgMusic);
    }

    switch (gameState) {
        case PLAYING:
            // gamepad Circle button
            if (Input::isEscapeKey()) {
                TraceLog(LOG_INFO, "Game paused");
                setGameState(MAIN_MENU);
                return;
            }
            // gamepad L1 button
            if (Input::isPauseKey()) {
                isPaused = !isPaused;
                if (isPaused) {
                    timeEnd = GetTime();
                    TraceLog(LOG_INFO, "Game paused");
                    PauseMusicStream(bgMusic);
                } else {
                    timeStart += GetTime() - timeEnd;
                    timeEnd = 0.f;
                    TraceLog(LOG_INFO, "Game resumed");
                    PlayMusicStream(bgMusic);
                }
            }
#ifdef DEBUG_MODE
            if (Input::isKeyPressed(KEY_I))
                setGameState(WIN);
            if (Input::isKeyPressed(KEY_O))
                setGameState(GAME_OVER);
            if (Input::isKeyPressed(KEY_B))
                spawnBomb();
#endif
            break;
        case MAIN_MENU:
            if (Input::isKeyPressed(KEY_ONE))
                currentDifficulty = EASY;
            if (Input::isKeyPressed(KEY_TWO))
                currentDifficulty = NORMAL;
            if (Input::isKeyPressed(KEY_THREE))
                currentDifficulty = HARD;
            if (Input::isEscapeKey()) {
                cleanup();
            }
            if (Input::isEnterOrSpace()) {
                reset();
                setGameState(PLAYING);
            }
            if (Input::isArrowUp()) {
                currentDifficulty = static_cast<Difficulty>((currentDifficulty + 2) % 3);
            }
            if (Input::isArrowDown()) {
                currentDifficulty = static_cast<Difficulty>((currentDifficulty + 1) % 3);
            }
            if (Input::isCreditsKey()) {
                setGameState(MENU_CREDITS);
            }
            // select difficulty with mouse
            if (Input::isLeftButton()) {
                const auto [x, y] = GetMousePosition();
                if (x >= SCREEN_DRAW_X - 100 && x <= SCREEN_DRAW_X + 100) {
                    if (y >= SCREEN_DRAW_Y + TEXT_HEIGHT * 1 &&
                        y <= SCREEN_DRAW_Y + TEXT_HEIGHT * 2)
                        currentDifficulty = EASY;
                    else if (y >= SCREEN_DRAW_Y + TEXT_HEIGHT * 2 &&
                             y <= SCREEN_DRAW_Y + TEXT_HEIGHT * 3)
                        currentDifficulty = NORMAL;
                    else if (y >= SCREEN_DRAW_Y + TEXT_HEIGHT * 3 &&
                             y <= SCREEN_DRAW_Y + TEXT_HEIGHT * 4)
                        currentDifficulty = HARD;
                    Input::lockMouse();
                    reset();
                    setGameState(PLAYING);
                }
            }
            break;
        case MENU_CREDITS:
            if (Input::isEscapeKey()) {
                setGameState(MAIN_MENU);
            }
            break;
        case WIN:
        case GAME_OVER:
            if (Input::isResetKey()) {
                reset();
                setGameState(PLAYING);
            }
            if (Input::isEscapeKey()) {
                cleanup();
            }
            break;
        case GAME_ERROR_TEXTURE:
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
    UnloadMusicStream(bgMusic);
    CloseAudioDevice();
#ifdef DISCORD_RPC_ENABLED
    if (discord.connected)
        DiscordRPC_shutdown(&discord);
#endif

    shouldClose = true;
}

void Game::setGameState(GameState newState)
{
    gameState = newState;
    if (gameState != PLAYING) {
        if (isShaking) {
            isShaking = false;
            SetWindowPosition(windowPos.x, windowPos.y);
        }
        isPaused = false;
    }
    if (gameState == GAME_OVER || gameState == WIN) {
        // stop the game timer
        timeEnd = GetTime();
    }
}

void Game::updateTimers()
{
    bombTimer += GetFrameTime();
    attackTimer += GetFrameTime();

    // spawn attacks
    if (attackTimer >= 0.5f) {
        if (GetRandomValue(0, 1) == 0 &&
            (currentDifficulty != HARD ? bossAttacks.size() <= 3 : true)) { // 50%
            const int max = (currentDifficulty == EASY) ? 2 : (currentDifficulty == NORMAL) ? 3 : 5;
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

void Game::drawTextCenter(const char *text, float x, float y, float fontSize, Color color)
{
    const Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, 1.f);
    DrawText(text, x - textSize.x / 2, y - textSize.y / 2, fontSize, color);
}

void Game::drawTextCombined(float x,
                            float y,
                            float fontSize,
                            const char *text1,
                            Color color1,
                            const char *text2,
                            Color color2)
{
    const Vector2 text1Vector = MeasureTextEx(GetFontDefault(), text1, fontSize, 1.f);
    const Vector2 text2Vector = MeasureTextEx(GetFontDefault(), text2, fontSize, 1.f);

    constexpr int spaceWidth = 15;
    const int totalWidth = text1Vector.x + spaceWidth + text2Vector.x;

    const float startX = x - totalWidth / 2.f;
    const int startY = y - text1Vector.y / 2;

    DrawText(text1, startX, startY, fontSize, color1);
    DrawText(text2, startX + text1Vector.x + spaceWidth, startY, fontSize, color2);
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
