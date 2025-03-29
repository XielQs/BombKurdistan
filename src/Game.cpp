#include "Game.hpp"
#include "Difficulty.hpp"
#include "raylib.h"
#define DEBUG_MODE 1

Game::Game():
    gameState(MAIN_MENU), 
    isMuted(false), 
    bombTimer(0.f), 
    attackTimer(0.f), 
    timeStart(GetTime()), 
    timeEnd(0.f),
    player(nullptr), 
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
    UnloadSound(bgMusic);
    CloseAudioDevice();
}

void Game::init() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Kurdistan Bombalayici");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL); // disable ESC key
    InitAudioDevice();

    bossTexture = LoadTexture("assets/boss.png");
    playerTexture = LoadTexture("assets/player.png");
    bombTexture = LoadTexture("assets/bomb.png");
    lareiTexture = LoadTexture("assets/larei.png");
    bgMusic = LoadSound("assets/bg_music.mp3");

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
    if (gameState == PLAYING) {
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
}

void Game::draw() {
    BeginDrawing();
    ClearBackground(Color{ 10, 10, 10, 255 });

    if (gameState == PLAYING && !isMuted) {
        if (!IsSoundPlaying(bgMusic)) {
            PlaySound(bgMusic);
        }
    } else {
        StopSound(bgMusic);
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

        // draw timer
        // we are using DrawText instead of drawTextCenter to avoid text scaling issues
        DrawText(TextFormat("Time: %s", formatTime()), SCREEN_WIDTH - 150, TEXT_HEIGHT - 10, 20, WHITE);

        // draw health bars
        DrawText(TextFormat("Health: %.0f", player->health), 10, TEXT_HEIGHT * 0.5, 20, WHITE);
        DrawText(TextFormat("Boss Health: %.0f", boss->health), 10, TEXT_HEIGHT * 1.8, 20, BLACK);
    } else if (gameState == GAME_OVER) {
        drawTextCenter("Beceriksizsin", SCREEN_DRAW_X, SCREEN_DRAW_Y - TEXT_HEIGHT * 2, 20, RED);
        drawTextCenter("Senin yuzunden kurtler bagimsizlasip isyan cikartti ve", SCREEN_DRAW_X, SCREEN_DRAW_Y, 20, WHITE);
        drawTextCenter("baskent Ankara dustu, Allah belani versin.", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT, 20, WHITE);
        drawTextCenter("Adam gibi oynayacaksan R'ye bas", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 4, 20, WHITE);
        drawTextCenter("Kurt isen ESC atabilirsin", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 5, 20, WHITE);
    } else if (gameState == WIN) {
        drawTextCenter("Helal Olsun!", SCREEN_DRAW_X, SCREEN_DRAW_Y - TEXT_HEIGHT * 2, 20, GREEN);
        drawTextCenter("Senin sayende xtinfirev ve kurtler dalgayi aldi!", SCREEN_DRAW_X, SCREEN_DRAW_Y, 20, WHITE);
        drawTextCenter("Artik Turkiye Cumhuriyeti cok daha mutlu bir devlet olabilir", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT, 20, WHITE);
        drawTextCenter("YASASIN TURKIYE CUMHURIYETI!", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 2, 20, RED);
        drawTextCenter("Kurdistani tekrar bombalamak icin R'ye bas", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 4, 20, WHITE);
        drawTextCenter("Mutlu bir sekilde ayrilmak icin ESC'ye bas", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 5, 20, WHITE);
        drawTextCenter(TextFormat("Bombalanan Sure: %s", formatTime()), SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 7, 20, WHITE);
        drawTextCombined(SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 8, 20, "Oyun Modu:", WHITE, getDifficultyName(currentDifficulty), (currentDifficulty == EASY) ? GREEN : (currentDifficulty == NORMAL) ? YELLOW : DARKRED);
    } else if (gameState == MAIN_MENU) {
        drawTextCenter("Kurdistan Bombalayici", SCREEN_DRAW_X, SCREEN_DRAW_Y - TEXT_HEIGHT * 4, 20, WHITE);
        drawTextCenter("Bu bir oyun projesidir tamamiyla eglence amaciyla uretilmistir", SCREEN_DRAW_X, SCREEN_DRAW_Y - 75, 20, GRAY);
        // select difficulty
        drawTextCenter("Zorluk Seçin", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT, 20, WHITE);
        drawTextCenter("1. Kurt vatandas", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 2, 20, GREEN);
        drawTextCenter("2. Turk vatandas", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 3, 20, YELLOW);
        drawTextCenter("3. ULKUCU VATANDAS", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 4, 20, DARKRED);

        // draw a (-) to indicate the current difficulty
        if (currentDifficulty == EASY) {
            drawTextCenter("-", SCREEN_DRAW_X - 95, SCREEN_DRAW_Y + TEXT_HEIGHT * 2, 20, GREEN);
        } else if (currentDifficulty == NORMAL) {
            drawTextCenter("-", SCREEN_DRAW_X - 100, SCREEN_DRAW_Y + TEXT_HEIGHT * 3, 20, YELLOW);
        } else if (currentDifficulty == HARD) {
            drawTextCenter("-", SCREEN_DRAW_X - 125, SCREEN_DRAW_Y + TEXT_HEIGHT * 4, 20, DARKRED);
        }

        drawTextCenter("Cikmak icin ESC'ye bas", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 6, 20, WHITE);
        drawTextCenter("Secim yapmak icin ok tuslarini ya da 1, 2 veya 3'u kullanabilirsin", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 7, 20, GRAY);
        drawTextCenter("Oyuna baslamak icin SPACE veya ENTER'a bas", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 8, 20, GRAY);

        if (isMuted) {
            drawTextCenter("M ile sesi ac/kapat", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 10, 20, GRAY);
        } else {
            drawTextCenter("M ile sesi ac/kapat", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 10, 20, WHITE);
        }

        if (currentDifficulty == HARD) {
            drawTextCenter("sana guveniyoruz kaptan", SCREEN_DRAW_X, SCREEN_DRAW_Y + TEXT_HEIGHT * 11, 20, DARKRED);
        }
    }

    EndDrawing();
}

void Game::handleInput() {
    if (IsKeyPressed(KEY_M)) {
        isMuted = !isMuted;
        if (isMuted) {
            StopSound(bgMusic);
        } else if (gameState == PLAYING) {
            PlaySound(bgMusic);
        }
    }

    if (gameState == PLAYING) {
        if (IsKeyPressed(KEY_ESCAPE)) {
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
        if (IsKeyPressed(KEY_ESCAPE)) {
            CloseWindow();
            exit(0); // using exit(0) because if we don't, the game will crash idk
        }
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            reset();
            setGameState(PLAYING);
        }
        if (IsKeyPressed(KEY_UP)) {
            currentDifficulty = (Difficulty)((currentDifficulty + 2) % 3);
        }
        if (IsKeyPressed(KEY_DOWN)) {
            currentDifficulty = (Difficulty)((currentDifficulty + 1) % 3);
        }
    }

    if (gameState == WIN || gameState == GAME_OVER) {
        if (IsKeyPressed(KEY_R)) {
            reset();
            setGameState(PLAYING);
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            CloseWindow();
            exit(0); // using exit(0) because if we don't, the game will crash idk
        }
    }
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
