#include "GlobalBounds.hpp"
#include "Difficulty.hpp"
#include "Constants.hpp"
#include "Game.hpp"
#include "raylib.h"
#include <algorithm>
#include <memory>
#include <string.h>

Game::Game():
    player(nullptr),
    shouldClose(false),
    gameState(MAIN_MENU),
    isMuted(false),
    bombTimer(0.f),
    attackTimer(0.f),
    timeStart(GetTime()),
    timeEnd(0.f),
    boss(nullptr),
    isShaking(false),
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
    #ifndef PLATFORM_WEB
    SetWindowState(FLAG_VSYNC_HINT);
    SetWindowIcon(LoadImage("assets/icon.png"));
    #endif

    #ifdef __linux__
    DiscordEventHandlers discordHandlers;

    memset(&discordHandlers, 0, sizeof(discordHandlers));
    memset(&discordActivity, 0, sizeof(discordActivity));

    discordHandlers.disconnected = [](bool wasError) {
        if (wasError) {
            TraceLog(LOG_ERROR, "Discord RPC disconnected with error");
        } else {
            TraceLog(LOG_INFO, "Discord RPC disconnected");
        }
    };

    discordHandlers.error = [](int errorCode, const char* message) {
        TraceLog(LOG_ERROR, "Discord RPC error: %d - %s", errorCode, message);
    };

    discordHandlers.ready = [](const DiscordUser* user) {
        TraceLog(LOG_INFO, "Discord RPC ready: %s", user->username);
    };
    DiscordRPC_init(&discord, "1356073834981097552", &discordHandlers);

    if (!discord.connected) TraceLog(LOG_ERROR, "Discord RPC failed to connect: %s", discord.last_error);    
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

    if (bossTexture.id <= 0 || playerTexture.id <= 0 || bombTexture.id <= 0 || lareiTexture.id <= 0) {
        TraceLog(LOG_ERROR, "Failed to load textures");
        setGameState(GAME_ERROR_TEXTURE);
        return;
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

    if (player) player->init();
    if (boss) boss->init();

    StopMusicStream(bgMusic);
}

void Game::update()
{
    if (isPaused) return; // do not update if paused

    bool shouldPlay = (gameState == PLAYING && !isMuted);

    if (shouldPlay && !IsMusicStreamPlaying(bgMusic)) {
        PlayMusicStream(bgMusic);
    } else if (!shouldPlay && IsMusicStreamPlaying(bgMusic)) {
        StopMusicStream(bgMusic);
    }

    UpdateMusicStream(bgMusic);

    if (gameState == PLAYING) {
        if (lastGameState != PLAYING) {
            TraceLog(LOG_INFO, "Game started");
            #ifdef __linux__
            discordActivity.state = getDifficultyName(currentDifficulty);
            discordActivity.details = "Kurdistani Bombaliyor";
            discordActivity.startTimestamp = timeStart / 1000;
            if (discord.connected) DiscordRPC_setActivity(&discord, &discordActivity);
            #endif
        }

        updateTimers();
        if (player) player->update();
        if (boss) boss->update(GetFrameTime());

        // we are not using elements.erase(elements.begin() + i) because it has O(n²) complexity
        // instead we are using std::remove_if to remove all inactive elements

        // update boss attacks
        for (size_t i = 0; i < bossAttacks.size(); ++i) {
            BossAttack& attack = *bossAttacks[i];
            attack.update(*player);
            if (!attack.isAlive()) {
                bossAttacks.erase(std::remove_if(bossAttacks.begin(), bossAttacks.end(), [](auto& attack) { return !attack->isAlive(); }), bossAttacks.end());
                --i;
            }
        }

        // update bombs
        for (size_t i = 0; i < bombs.size(); ++i) {
            Bomb& bomb = *bombs[i];
            bomb.update(*player, *boss, GetFrameTime());
            if (!bomb.isAlive()) {
                bombs.erase(std::remove_if(bombs.begin(), bombs.end(), [](auto& bomb) { return !bomb->isAlive(); }), bombs.end());
                --i;
            }
        }

        if (player->health <= 0.f) setGameState(GAME_OVER);
        if (boss->health <= 0.f)   setGameState(WIN);

        // shake the window
        if (isShaking) {
            const float remainingTime = shakeEndTime - GetTime();
            if (remainingTime > 0) {
                const float duration = shakeEndTime - (shakeEndTime - remainingTime); // shake duration
                const float progress = remainingTime / duration; // shake progress

                const float currentIntensity = shakeIntensity * progress; // intensity decreases over time

                // we are using a random angle to shake the window
                // shake the window in a circle
                float angle = GetRandomValue(0, 360) * DEG2RAD;
                float offsetX = cosf(angle) * currentIntensity;
                float offsetY = sinf(angle) * currentIntensity;

                SetWindowPosition(
                    windowPos.x + offsetX,
                    windowPos.y + offsetY
                );
            } else {
                // reset window position
                SetWindowPosition(windowPos.x, windowPos.y);
                isShaking = false;
            }
        }
    }
    if (gameState == MAIN_MENU && lastGameState != MAIN_MENU) {
        TraceLog(LOG_INFO, "Main menu");
        #ifdef __linux__
        discordActivity.state = "Ana menude";
        discordActivity.details = nullptr;
        discordActivity.startTimestamp = GetTime() / 1000;
        if (discord.connected) DiscordRPC_setActivity(&discord, &discordActivity);
        #endif
    }
    if (gameState == WIN && lastGameState != WIN) {
        TraceLog(LOG_INFO, "Game won");
        #ifdef __linux__
        discordActivity.state = getDifficultyName(currentDifficulty);
        discordActivity.details = "Ankara kurtarildi!";
        discordActivity.startTimestamp = GetTime() / 1000;
        if (discord.connected) DiscordRPC_setActivity(&discord, &discordActivity);
        #endif
    }
    if (gameState == GAME_OVER && lastGameState != GAME_OVER) {
        TraceLog(LOG_INFO, "Game over");
        #ifdef __linux__
        discordActivity.state = getDifficultyName(currentDifficulty);
        discordActivity.details = "Ankara dustu!";
        discordActivity.startTimestamp = GetTime() / 1000;
        if (discord.connected) DiscordRPC_setActivity(&discord, &discordActivity);
        #endif
    }
    lastGameState = gameState;
}

void Game::draw()
{
    BeginDrawing();
    ClearBackground(Color{ 10, 10, 10, 255 });

    switch (gameState) {
        case PLAYING:
            for (const auto& bomb : bombs) bomb->draw();
            for (const auto& attack : bossAttacks) attack->draw();

            boss->draw();
            player->draw();

            // we are using DrawText instead of drawTextCenter to avoid text scaling issues

            // draw menu items
            DrawText(TextFormat("Time: %s", formatTime()), GetScreenWidth() - TEXT_HEIGHT * 6, TEXT_HEIGHT * 0.5, 20, WHITE);
            DrawText(TextFormat("FPS: %d", GetFPS()), GetScreenWidth() - TEXT_HEIGHT * 3, GetScreenHeight() - TEXT_HEIGHT, 18, WHITE);
            if (isPaused) {
                // blink effect
                if (fmod(GetTime(), 1.0f) < 0.5f) drawTextCenter("PAUSED", SCREEN_DRAW_X, SCREEN_DRAW_Y, 40, WHITE);
            }
            break;
        case GAME_OVER:
            drawTextCenter("Beceriksizsin", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -2, 20, RED);
            drawTextCenter("Senin yuzunden kurtler bagimsizlasip isyan cikartti ve", SCREEN_DRAW_X, SCREEN_DRAW_Y, 20, WHITE);
            drawTextCenter("baskent Ankara dustu, Allah belani versin.", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT, 20, WHITE);
            drawTextCenter("Adam gibi oynayacaksan R'ye bas", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 4, 20, WHITE);
            drawTextCenter("Kurt isen ESC atabilirsin", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 5, 20, WHITE);
            break;
        case WIN:
            drawTextCenter("Helal Olsun!", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -2, 20, GREEN);
            drawTextCenter("Senin sayende xtinfirev ve kurtler dalgayi aldi!", SCREEN_DRAW_X, SCREEN_DRAW_Y, 20, WHITE);
            drawTextCenter("Artik Turkiye Cumhuriyeti cok daha mutlu bir devlet olabilir", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT, 20, WHITE);
            drawTextCenter("YASASIN TURKIYE CUMHURIYETI!", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 2, 20, RED);
            drawTextCenter("Kurdistani tekrar bombalamak icin R'ye bas", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 4, 20, WHITE);
            drawTextCenter("Mutlu bir sekilde ayrilmak icin ESC'ye bas", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 5, 20, WHITE);
            drawTextCenter(TextFormat("Bombalanan Sure: %s", formatTime()), SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 7, 20, WHITE);
            drawTextCombined(SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 8, 20, "Oyun Modu:", WHITE, getDifficultyName(currentDifficulty), (currentDifficulty == EASY) ? GREEN : (currentDifficulty == NORMAL) ? YELLOW : DARKRED);
            break;
        case MAIN_MENU:
            drawTextCenter("Kurdistan Bombalayici", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -4, 20, WHITE);
            drawTextCenter("Bu bir oyun projesidir tamamiyla eglence amaciyla uretilmistir", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -3, 20, GRAY);
            // select difficulty
            drawTextCenter("Zorluk Seçin", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -1, 20, WHITE);
            drawTextCenter("1. Kurt vatandas", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 1, 20, GREEN);
            drawTextCenter("2. Turk vatandas", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 2, 20, YELLOW);
            drawTextCenter("3. ULKUCU VATANDAS", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 3, 20, DARKRED);

            // draw a (-) to indicate the current difficulty
            if (currentDifficulty == EASY) {
                drawTextCenter("-", SCREEN_DRAW_X - 95, SCREEN_DRAW_Y + TEXT_HEIGHT * 1, 20, GREEN);
            } else if (currentDifficulty == NORMAL) {
                drawTextCenter("-", SCREEN_DRAW_X - 100, SCREEN_DRAW_Y + TEXT_HEIGHT * 2, 20, YELLOW);
            } else if (currentDifficulty == HARD) {
                drawTextCenter("-", SCREEN_DRAW_X - 125, SCREEN_DRAW_Y + TEXT_HEIGHT * 3, 20, DARKRED);
            }

            drawTextCenter("Cikmak icin ESC'ye bas", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 5, 20, WHITE);
            drawTextCenter("Secim yapmak icin ok tuslarini ya da 1, 2 veya 3'u kullanabilirsin", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 6, 20, GRAY);
            drawTextCenter("Oyuna baslamak icin SPACE veya ENTER'a bas", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 7, 20, GRAY);
            drawTextCenter("Yapimcilar ve ozel tesekkurler icin C'ye bas", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 8, 20, GRAY);

            drawTextCenter("M ile sesi ac/kapat", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 10, 20, isMuted ? GRAY : WHITE);

            if (currentDifficulty == HARD) {
                drawTextCenter("sana guveniyoruz kaptan", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 11, 20, DARKRED);
            }
            break;
        case MENU_CREDITS:
            marqueeText("TesekkurlerTesekkurlerTesekkurlerTesekkurlerTesekkurlerTesekkurlerTesekkurlerTesekkurler", SCREEN_DRAW_Y + TEXT_HEIGHT * -8, 20, WHITE, 80.f);
            drawTextCenter("Herkese ayri ayri tesekkurlerimi sunuyorum siz olmasaniz", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -6, 20, GRAY);
            drawTextCenter("BombKurdistan projesi bu kadar gelisemezdi <3", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -5, 20, GRAY);
            drawTextCenter("(dunyanin en iyi oyun projesi)", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -4, 20, GRAY);

            drawTextCenter("larei <3", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -2, 20, WHITE);
            drawTextCenter("kosero <3", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -1, 20, WHITE);
            drawTextCenter("yesil asya <3", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 0, 20, WHITE);
            drawTextCenter("toby fox <3", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 1, 20, WHITE);
            
            drawTextCenter("Dosya kaynakcasi:", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 3, 20, WHITE);
            drawTextCenter("boss.png: wikipedia (goruntude oynadim azcik)", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 4, 20, GRAY);
            drawTextCenter("bomb.png: stardew valley", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 5, 20, GRAY);
            drawTextCenter("player.png: undertale", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 6, 20, GRAY);
            drawTextCenter("bg_music.mp3: larei atmisti bir youtube videosundan alinti", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 7, 20, GRAY);
            drawTextCenter("larei.png: larei.", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 8, 20, GRAY);

            drawTextCenter("Ana menu icin ESC'ye bas", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 10, 20, WHITE);
            break;
        case GAME_ERROR_TEXTURE:
            drawTextCenter("Bir hata olustu", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -2, 20, RED);
            drawTextCenter("Gerekli dosyalar eksik veya bozuk", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 0, 20, WHITE);
            drawTextCenter("Lutfen dosyalari kontrol edin ve tekrar deneyin", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 1, 20, WHITE);
            drawTextCenter("Oyunun ve assets klasorunun ayni konumda oldugundan emin olun", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 2, 20, WHITE);
            break;
        default:
            break;
    }

    EndDrawing();
}

void Game::handleInput()
{
    // gamepad Square button
    if (IsKeyPressed(KEY_M) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT)) {
        isMuted = !isMuted;
        if (isMuted) StopMusicStream(bgMusic);
        else if (gameState == PLAYING) PlayMusicStream(bgMusic);
    }

    switch (gameState) {
        case PLAYING:
            // gamepad Circle button
            if (IsKeyPressed(KEY_ESCAPE) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) {
                TraceLog(LOG_INFO, "Game paused");
                setGameState(MAIN_MENU);
                return;
            }
            // gamepad L1 button
            if (IsKeyPressed(KEY_P) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_TRIGGER_1)) {
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
            if (IsKeyPressed(KEY_I)) setGameState(WIN);
            if (IsKeyPressed(KEY_O)) setGameState(GAME_OVER);
            if (IsKeyPressed(KEY_B)) spawnBomb();
            #endif
            break;
        case MAIN_MENU:
            if (IsKeyPressed(KEY_ONE)) currentDifficulty = EASY;
            if (IsKeyPressed(KEY_TWO)) currentDifficulty = NORMAL;
            if (IsKeyPressed(KEY_THREE)) currentDifficulty = HARD;
            // gamepad Circle button
            if (IsKeyPressed(KEY_ESCAPE) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) {
                cleanup();
                return;
            }
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
                reset();
                setGameState(PLAYING);
            }
            // gamepad Dpad up
            if (IsKeyPressed(KEY_UP) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_UP)) {
                currentDifficulty = (Difficulty)((currentDifficulty + 2) % 3);
            }
            // gamepad Dpad down
            if (IsKeyPressed(KEY_DOWN) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) {
                currentDifficulty = (Difficulty)((currentDifficulty + 1) % 3);
            }
            // gamepad Triangle button
            if (IsKeyPressed(KEY_C) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_UP)) {
                setGameState(MENU_CREDITS);
            }
            break;
        case MENU_CREDITS:
            // gamepad Circle button
            if (IsKeyPressed(KEY_ESCAPE) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) {
                setGameState(MAIN_MENU);
            }
            break;
        case WIN:
        case GAME_OVER:
            // gamepad X button
            if (IsKeyPressed(KEY_R) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
                reset();
                setGameState(PLAYING);
            }
            // gamepad Circle button
            if (IsKeyPressed(KEY_ESCAPE) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) {
                cleanup();
                return;
            }
            break;
        case GAME_ERROR_TEXTURE:
            // gamepad Circle button
            if (IsKeyPressed(KEY_ESCAPE) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) {
                cleanup();
                return;
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
    if (shouldClose) return;
    // player and boss are unique_ptrs, so we don't need to delete them

    TraceLog(LOG_INFO, "Cleaning up game resources");

    UnloadTexture(bossTexture);
    UnloadTexture(playerTexture);
    UnloadTexture(bombTexture);
    UnloadTexture(lareiTexture);
    UnloadMusicStream(bgMusic);
    CloseAudioDevice();
    #ifdef __linux__
    if (discord.connected) DiscordRPC_shutdown(&discord);
    #endif

    shouldClose = true;
}

void Game::updateTimers()
{
    bombTimer += GetFrameTime();
    attackTimer += GetFrameTime();

    // spawn attacks
    if (attackTimer >= 0.5f) {
        if (GetRandomValue(0, 1) == 0 && (currentDifficulty != HARD ? bossAttacks.size() <= 3 : true)) { // 50%
            int attackCount = GetRandomValue(1, (currentDifficulty == EASY) ? 2 : (currentDifficulty == NORMAL) ? 3 : 5);
            for (int j = 0; j < attackCount; ++j) createAttack();
        }
        attackTimer = 0.f;
    }

    // spawn bombs randomly
    if (bombTimer >= 5.f) {
        if (GetRandomValue(0, 2) == 0) spawnBomb(); // 33%
        bombTimer = 0.f;
    }
}

void Game::createAttack()
{
    AttackSize size = static_cast<AttackSize>(GetRandomValue(0, 2));

    Vector2 direction = Vector2Normalize(player->velocity);

    const float attackAreaWidth = movementBounds.right - movementBounds.left;
    const float attackAreaHeight = movementBounds.top;

    Vector2 playerCenter = {
        player->position.x + player->texture.width / 2.f,
        player->position.y + player->texture.height / 2.f
    };

    Vector2 attackPos = {
        playerCenter.x + direction.x * attackAreaWidth / 2.f,
        playerCenter.y + direction.y * attackAreaHeight/ 2.f
    };

    attackPos.x = std::clamp(attackPos.x, playerCenter.x - attackAreaWidth / 2.f, playerCenter.x + attackAreaWidth / 2.f);
    attackPos.y = std::clamp(attackPos.y, playerCenter.y - attackAreaHeight / 2.f, playerCenter.y + attackAreaHeight / 2.f);

    attackPos.x += GetRandomValue(-ATTACK_OFFSET, ATTACK_OFFSET);
    attackPos.y += GetRandomValue(-ATTACK_OFFSET, ATTACK_OFFSET);

    bossAttacks.emplace_back(std::make_unique<BossAttack>(attackPos, size));

    TraceLog(LOG_INFO, "Attack created at position: (%f, %f)", attackPos.x, attackPos.y);
}

void Game::spawnBomb()
{
    Vector2 bombPos = {
        static_cast<float>(GetRandomValue(movementBounds.left, movementBounds.right)),
        static_cast<float>(GetRandomValue(movementBounds.top, movementBounds.bottom))
    };

    bombs.emplace_back(std::make_unique<Bomb>(
        bombTexture,
        Vector2{
            static_cast<float>(bombPos.x),
            static_cast<float>(bombPos.y)
        }
    ));

    TraceLog(LOG_INFO, "Bomb spawned at position: (%f, %f)", bombPos.x, bombPos.y);
}

void Game::shakeWindow(float duration, float intensity) 
{
    windowPos = GetWindowPosition();
    shakeEndTime = GetTime() + duration;
    shakeIntensity = intensity;
    isShaking = true;
}

void Game::drawTextCenter(const char* text, float x, float y, float fontSize, Color color)
{
    Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, 1.f);
    DrawText(text, x - textSize.x / 2, y - textSize.y / 2, fontSize, color);
}

void Game::drawTextCombined(float x, float y, float fontSize, const char* text1, Color color1, const char* text2, Color color2)
{
    Vector2 text1Vector = MeasureTextEx(GetFontDefault(), text1, fontSize, 1.f);
    Vector2 text2Vector = MeasureTextEx(GetFontDefault(), text2, fontSize, 1.f);

    int spaceWidth = 15;
    int totalWidth = text1Vector.x + spaceWidth + text2Vector.x;

    float startX = x - totalWidth / 2.f;
    int startY = y - text1Vector.y / 2;

    DrawText(text1, startX, startY, fontSize, color1);
    DrawText(text2, startX + text1Vector.x + spaceWidth, startY, fontSize, color2);
}

void Game::marqueeText(const char* text, float y, float fontSize, Color color, float speed)
{
    static float x = 0;
    int textWidth = MeasureText(text, fontSize);

    x -= speed * GetFrameTime();

    if (x < -textWidth) x += textWidth;

    DrawText(text, x, y, fontSize, color);
    DrawText(text, x + textWidth, y, fontSize, color);
}

const char* Game::formatTime()
{
    float timeElapsed = (timeEnd > 0 ? timeEnd : GetTime()) - timeStart;
    int minutes = static_cast<int>(timeElapsed / 60);
    int seconds = static_cast<int>(timeElapsed) % 60;
    int milliseconds = static_cast<int>(timeElapsed * 1000) % 1000 / 10;
    return TextFormat("%02d:%02d.%02d", minutes, seconds, milliseconds);
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
