#include "Constants.hpp"
#include "Difficulty.hpp"
#include "Game.hpp"
#include "raylib.h"
#include <string.h>
#define DEBUG_MODE 1

Game::Game():
    player(nullptr),
    gameState(MAIN_MENU),
    isMuted(false),
    bombTimer(0.f),
    attackTimer(0.f),
    timeStart(GetTime()),
    timeEnd(0.f),
    boss(nullptr)
{
}

Game::~Game() {
    if (player) delete player;
    if (boss) delete boss;

    UnloadTexture(bossTexture);
    UnloadTexture(playerTexture);
    UnloadTexture(bombTexture);
    UnloadTexture(lareiTexture);
    UnloadMusicStream(bgMusic);
    CloseAudioDevice();
    #ifdef __linux__
    DiscordRPC_shutdown(&discord);
    #endif
}

void Game::init() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Kurdistan Bombalayici");
    SetTargetFPS(GAME_FPS);
    SetExitKey(KEY_NULL); // disable ESC key
    InitAudioDevice();
    #ifndef PLATFORM_WEB
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

    if (!discord.connected) {
        TraceLog(LOG_ERROR, "Discord RPC failed to connect: %s", discord.last_error);
    } else {
        TraceLog(LOG_INFO, "Discord RPC connected successfully");
        discordActivity.largeImageText = "Kurdistani Bombala";
    }

    #endif

    bossTexture = LoadTexture("assets/boss.png");
    playerTexture = LoadTexture("assets/player.png");
    bombTexture = LoadTexture("assets/bomb.png");
    lareiTexture = LoadTexture("assets/larei.png");
    bgMusic = LoadMusicStream("assets/bg_music.mp3");

    player = new Player(playerTexture);
    boss = new Boss(bossTexture, lareiTexture);
}

void Game::reset() {
    gameState = MAIN_MENU;
    bombTimer = 0.f;
    attackTimer = 0.f;
    timeStart = GetTime();
    timeEnd = 0.f;
    bossAttacks.clear();
    bombs.clear();

    if (player) player->init();
    if (boss) boss->init();
}

void Game::update() {
    UpdateMusicStream(bgMusic);
    if (gameState == PLAYING) {
        if (lastGameState != PLAYING) {
            TraceLog(LOG_INFO, "Game started");
            #ifdef __linux__
            discordActivity.state = getDifficultyName(currentDifficulty);
            discordActivity.details = "Kurdistani Bombaliyor";
            discordActivity.startTimestamp = timeStart / 1000;
            if (discord.connected)
                DiscordRPC_setActivity(&discord, &discordActivity);
            #endif
        }

        updateTimers();
        player->update();
        boss->update();

        // update boss attacks
        for (size_t i = 0; i < bossAttacks.size(); ++i) {
            BossAttack& attack = bossAttacks[i];
            attack.update(*player);
            if (!attack.isAlive()) {
                bossAttacks.erase(bossAttacks.begin() + i);
                --i;
            }
        }

        // update bombs
        for (size_t i = 0; i < bombs.size(); ++i) {
            Bomb& bomb = bombs[i];
            bomb.update(*player, *boss);
            if (!bomb.isAlive()) {
                bombs.erase(bombs.begin() + i);
                --i;
            }
        }

        if (player->health <= 0) {
            setGameState(GAME_OVER);
        }
        if (boss->health <= 0) {
            setGameState(WIN);
        }
    }
    if (gameState == MAIN_MENU) {
        if (lastGameState != MAIN_MENU) {
            TraceLog(LOG_INFO, "Main menu");
            #ifdef __linux__
            discordActivity.state = "Ana menude";
            discordActivity.details = nullptr;
            discordActivity.startTimestamp = GetTime() / 1000;
            if (discord.connected)
                DiscordRPC_setActivity(&discord, &discordActivity);
            #endif
        }
    }
    if (gameState == WIN) {
        if (lastGameState != WIN) {
            TraceLog(LOG_INFO, "Game won");
            #ifdef __linux__
            discordActivity.state = getDifficultyName(currentDifficulty);
            discordActivity.details = "Ankara kurtarildi!";
            discordActivity.startTimestamp = GetTime() / 1000;
            if (discord.connected)
                DiscordRPC_setActivity(&discord, &discordActivity);
            #endif
        }
    }
    if (gameState == GAME_OVER) {
        if (lastGameState != GAME_OVER) {
            TraceLog(LOG_INFO, "Game over");
            #ifdef __linux__
            discordActivity.state = getDifficultyName(currentDifficulty);
            discordActivity.details = "Ankara dustu!";
            discordActivity.startTimestamp = GetTime() / 1000;
            if (discord.connected)
                DiscordRPC_setActivity(&discord, &discordActivity);
            #endif
        }
    }
    lastGameState = gameState;
}

void Game::draw() {
    BeginDrawing();
    ClearBackground(Color{ 10, 10, 10, 255 });

    if (gameState == PLAYING && !isMuted) {
        if (!IsMusicStreamPlaying(bgMusic)) {
            PlayMusicStream(bgMusic);
        }
    } else {
        StopMusicStream(bgMusic);
    }

    if (gameState == PLAYING) {
        for (const Bomb& bomb : bombs) {
            bomb.draw();
        }

        for (const BossAttack& attack : bossAttacks) {
            attack.draw();
        }

        boss->draw();
        player->draw();

        // we are using DrawText instead of drawTextCenter to avoid text scaling issues
        // draw timer
        DrawText(TextFormat("Time: %s", formatTime()), SCREEN_WIDTH - 150, TEXT_HEIGHT - 10, 20, WHITE);

        // draw health bars
        DrawText(TextFormat("Health: %.0f", player->health), 10, TEXT_HEIGHT * 0.5, 20, WHITE);
        DrawText(TextFormat("Boss Health: %.0f", boss->health), 10, TEXT_HEIGHT * 1.8, 20, BLACK);
    } else if (gameState == GAME_OVER) {
        drawTextCenter("Beceriksizsin", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -2, 20, RED);
        drawTextCenter("Senin yuzunden kurtler bagimsizlasip isyan cikartti ve", SCREEN_DRAW_X, SCREEN_DRAW_Y, 20, WHITE);
        drawTextCenter("baskent Ankara dustu, Allah belani versin.", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT, 20, WHITE);
        drawTextCenter("Adam gibi oynayacaksan R'ye bas", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 4, 20, WHITE);
        drawTextCenter("Kurt isen ESC atabilirsin", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 5, 20, WHITE);
    } else if (gameState == WIN) {
        drawTextCenter("Helal Olsun!", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * -2, 20, GREEN);
        drawTextCenter("Senin sayende xtinfirev ve kurtler dalgayi aldi!", SCREEN_DRAW_X, SCREEN_DRAW_Y, 20, WHITE);
        drawTextCenter("Artik Turkiye Cumhuriyeti cok daha mutlu bir devlet olabilir", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT, 20, WHITE);
        drawTextCenter("YASASIN TURKIYE CUMHURIYETI!", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 2, 20, RED);
        drawTextCenter("Kurdistani tekrar bombalamak icin R'ye bas", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 4, 20, WHITE);
        drawTextCenter("Mutlu bir sekilde ayrilmak icin ESC'ye bas", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 5, 20, WHITE);
        drawTextCenter(TextFormat("Bombalanan Sure: %s", formatTime()), SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 7, 20, WHITE);
        drawTextCombined(SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 8, 20, "Oyun Modu:", WHITE, getDifficultyName(currentDifficulty), (currentDifficulty == EASY) ? GREEN : (currentDifficulty == NORMAL) ? YELLOW : DARKRED);
    } else if (gameState == MAIN_MENU) {
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
        drawTextCenter("Yapimcilar ve ozel tesekkurler icin F7'ye bas", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 8, 20, GRAY);

        drawTextCenter("M ile sesi ac/kapat", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 10, 20, isMuted ? GRAY : WHITE);

        if (currentDifficulty == HARD) {
            drawTextCenter("sana guveniyoruz kaptan", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 11, 20, DARKRED);
        }
    } else if (gameState == MENU_CREDITS) {
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
    }

    EndDrawing();
}

void Game::handleInput() {
    // gamepad Square button
    if (IsKeyPressed(KEY_M) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT)) {
        isMuted = !isMuted;
        if (isMuted) {
            StopMusicStream(bgMusic);
        } else if (gameState == PLAYING) {
            PlayMusicStream(bgMusic);
        }
    }

    if (gameState == PLAYING) {
        // gamepad Circle button
        if (IsKeyPressed(KEY_ESCAPE) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) {
            TraceLog(LOG_INFO, "Game paused");
            setGameState(MAIN_MENU);
            return;
        }
        if (IsKeyPressed(KEY_P) && DEBUG_MODE) {
            setGameState(WIN);
        }
        if (IsKeyPressed(KEY_O) && DEBUG_MODE) {
            setGameState(GAME_OVER);
        }
    }

    if (gameState == MAIN_MENU) {
        if (IsKeyPressed(KEY_ONE)) currentDifficulty = EASY;
        if (IsKeyPressed(KEY_TWO)) currentDifficulty = NORMAL;
        if (IsKeyPressed(KEY_THREE)) currentDifficulty = HARD;
        // gamepad Circle button
        if (IsKeyPressed(KEY_ESCAPE) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) {
            CloseWindow();
            exit(0); // using exit(0) because if we don't, the game will crash idk
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
        // gamepad ps button
        if (IsKeyPressed(KEY_F7) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE)) {
            setGameState(MENU_CREDITS);
        }
    }

    if (gameState == MENU_CREDITS) {
        // gamepad Circle button
        if (IsKeyPressed(KEY_ESCAPE) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) {
            setGameState(MAIN_MENU);
        }
    }

    if (gameState == WIN || gameState == GAME_OVER) {
        // gamepad X button
        if (IsKeyPressed(KEY_R) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
            reset();
            setGameState(PLAYING);
        }
        // gamepad Circle button
        if (IsKeyPressed(KEY_ESCAPE) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) {
            CloseWindow();
            exit(0); // using exit(0) because if we don't, the game will crash idk
        }
    }
}

void Game::updateFrame() {
    handleInput();
    update();
    draw();
}

void Game::updateTimers() {
    bombTimer += GetFrameTime();
    attackTimer += GetFrameTime();

    // spawn attacks
    if (attackTimer >= 0.5f) {
        if (GetRandomValue(0, 2) == 0 && (currentDifficulty != HARD ? bossAttacks.size() <= 3 : true)) { // 50%
            int attackCount = GetRandomValue(1, (currentDifficulty == EASY) ? 2 : (currentDifficulty == NORMAL) ? 3 : 5);
            for (int j = 0; j < attackCount; ++j) {
                createAttack();
            }
        }
        attackTimer = 0.f;
    }

    // spawn bombs randomly
    if (bombTimer >= 5.0f) {
        if (GetRandomValue(0, 2) == 0) { // 50%
            int offset = 10;
            Bomb bomb(bombTexture, (Vector2){
                GetRandomValue(SCREEN_PADDING + offset, SCREEN_WIDTH - SCREEN_PADDING * 2 - offset) - 0.f,
                GetRandomValue(BOSS_HEIGHT + SCREEN_PADDING + offset, SCREEN_HEIGHT - SCREEN_PADDING - offset) - 0.f
            });
            TraceLog(LOG_INFO, "Bomb created at position: (%f, %f)", bomb.position.x, bomb.position.y);
            bombs.push_back(bomb);
        }
        bombTimer = 0.f;
    }
}

void Game::createAttack() {
    AttackSize size = (AttackSize)(GetRandomValue(0, 2));

    Vector2 direction = player->velocity;

    float length = sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length > 0) {
        direction.x /= length;
        direction.y /= length;
    }

    float bias = ATTACK_OFFSET * 2.0f;
    Vector2 attackPos = {
        player->position.x + player->texture.width / 2.f + GetRandomValue(-ATTACK_OFFSET, ATTACK_OFFSET) + direction.x * bias,
        player->position.y + player->texture.height / 2.f + GetRandomValue(-ATTACK_OFFSET, ATTACK_OFFSET) + direction.y * bias
    };

    BossAttack attack(attackPos, size);
    TraceLog(LOG_INFO, "Attack created at position: (%f, %f)", attack.position.x, attack.position.y);
    bossAttacks.push_back(attack);
}

void Game::drawTextCenter(const char* text, float x, float y, float fontSize, Color color) {
    Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, 1.0f);
    DrawText(text, x - textSize.x / 2, y - textSize.y / 2, fontSize, color);
}

void Game::drawTextCombined(float x, float y, float fontSize, const char* text1, Color color1, const char* text2, Color color2) {
    int size1 = MeasureText(text1, fontSize);
    int size2 = MeasureText(text2, fontSize);

    int spaceWidth = 10;
    int totalWidth = size1 + spaceWidth + size2;

    float startX = x - (totalWidth / 2.f);
    int startY = y - (fontSize / 2);

    DrawText(text1, startX, startY, fontSize, color1);
    DrawText(text2, startX + size1 + spaceWidth, startY, fontSize, color2);
}

void Game::marqueeText(const char* text, float y, float fontSize, Color color, float speed) {
    static float x = 0;
    int textWidth = MeasureText(text, fontSize);

    x -= speed * GetFrameTime();

    if (x < -textWidth) {
        x += textWidth;
    }

    DrawText(text, (int)x, (int)y, (int)fontSize, color);
    DrawText(text, (int)x + textWidth, (int)y, (int)fontSize, color);
}

const char* Game::formatTime() {
    float timeElapsed = (timeEnd > 0 ? timeEnd : GetTime()) - timeStart;
    int minutes = (int)(timeElapsed / 60);
    int seconds = (int)(timeElapsed) % 60;
    int milliseconds = (int)(timeElapsed * 1000) % 1000 / 10;
    return TextFormat("%02d:%02d.%02d", minutes, seconds, milliseconds);
}

void Game::setGameState(GameState newState) {
    gameState = newState;
    if (newState == GAME_OVER || newState == WIN) {
        // stop the game timer
        timeEnd = GetTime();
    }
}
