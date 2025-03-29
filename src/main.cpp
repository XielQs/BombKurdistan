#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define ATTACK_OFFSET 150
#include <raylib.h>
#include "Player.hpp"
#include "BossAttack.hpp"
#include "Difficulty.hpp"
#include "Boss.hpp"
#include "Bomb.hpp"
#include <vector>
#include <math.h>
#define DARKRED (Color){ 139, 0, 0, 255 }

std::vector<BossAttack> bossAttacks;
std::vector<Bomb> bombs;
float bombTimer = 0.f;
float attackTimer = 0.f;
enum GameState {
    PLAYING,
    GAME_OVER,
    MAIN_MENU,
    WIN
};
GameState gameState = MAIN_MENU;
bool isMuted = false;

void DrawTextCenter(const char* text, float x, float y, float fontSize, Color color) {
    Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, 1.0f);
    DrawText(text, x - textSize.x / 2, y - textSize.y / 2, fontSize, color);
}

void createAttack(Player& player) {
    AttackSize size = (AttackSize)(GetRandomValue(0, 2));

    Vector2 direction = player.velocity;

    float length = sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length > 0) {
        direction.x /= length;
        direction.y /= length;
    }

    float bias = ATTACK_OFFSET * 2.0f;
    Vector2 attackPos = {
        player.position.x + player.texture.width / 2.f + GetRandomValue(-ATTACK_OFFSET, ATTACK_OFFSET) + direction.x * bias,
        player.position.y + player.texture.height / 2.f + GetRandomValue(-ATTACK_OFFSET, ATTACK_OFFSET) + direction.y * bias
    };

    BossAttack attack(attackPos, size);
    TraceLog(LOG_INFO, "Attack created at position: (%f, %f)", attack.position.x, attack.position.y);
    bossAttacks.push_back(attack);
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Kurdistan Bombalayici");
    SetTargetFPS(60);
    SetExitKey(0) ; // disable ESC key
    InitAudioDevice();
    Texture2D bossTexture = LoadTexture("assets/boss.png");
    Texture2D playerTexture = LoadTexture("assets/player.png");
    Texture2D bombTexture = LoadTexture("assets/bomb.png");
    Texture2D lareiTexture = LoadTexture("assets/larei.png");
    Sound bgMusic = LoadSound("assets/bg_music.mp3");

    Player player(playerTexture);
    Boss boss(bossTexture, lareiTexture);
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(Color{ 10, 10, 10, 255 });
        if (gameState == PLAYING && !isMuted) {
            if (!IsSoundPlaying(bgMusic)) {
                PlaySound(bgMusic);
            }
        } else {
            StopSound(bgMusic);
        }
        float screenX = GetScreenWidth() / 2.f;
        float screenY = GetScreenHeight() / 2.f;
        if (gameState == PLAYING) {
            if (IsKeyPressed(KEY_M)) {
                isMuted = !isMuted;
                if (isMuted) {
                    StopSound(bgMusic);
                } else {
                    PlaySound(bgMusic);
                }
            }
            if (IsKeyPressed(KEY_ESCAPE)) {
                gameState = MAIN_MENU;
            }
            for (size_t i = 0; i < bossAttacks.size(); ++i) {
                BossAttack& attack = bossAttacks[i];
                attack.update(player);
                attack.draw();
                if (!attack.isAlive()) {
                    bossAttacks.erase(bossAttacks.begin() + i);
                    --i;
                }
            }
            // draw boss
            boss.draw();
            boss.update();
            bombTimer += GetFrameTime();
            attackTimer += GetFrameTime();
            if (attackTimer >= 0.5f) {
                if (GetRandomValue(0, 2) == 0) { // 50%
                    int attackCount = GetRandomValue(1, (currentDifficulty == EASY) ? 2 : (currentDifficulty == NORMAL) ? 3 : 5);
                    for (int j = 0; j < attackCount; ++j) {
                        createAttack(player);
                    }
                }
                attackTimer = 0.f;
            }
            // spawn bombs randomly
            if (bombTimer >= 5.0f) {
                if (GetRandomValue(0, 2) == 0) { // 50%
                    Bomb bomb(bombTexture, (Vector2){ GetRandomValue(SCREEN_PADDING, GetScreenWidth() - SCREEN_PADDING * 2) - 0.f, GetRandomValue(BOSS_HEIGHT + SCREEN_PADDING, SCREEN_HEIGHT - SCREEN_PADDING) - 0.f });
                    TraceLog(LOG_INFO, "Bomb created at position: (%f, %f)", bomb.position.x, bomb.position.y);
                    bombs.push_back(bomb);
                }
                bombTimer = 0.f;
            }
            for (size_t i = 0; i < bombs.size(); ++i) {
                Bomb& bomb = bombs[i];
                bomb.update(player, boss);
                bomb.draw();
                if (!bomb.isAlive()) {
                    bombs.erase(bombs.begin() + i);
                    --i;
                }
            }
            // draw player
            player.update();
            player.draw();
            if (player.health <= 0) {
                gameState = GAME_OVER;
            }
            if (boss.health <= 0) {
                gameState = WIN;
            }
        } else if (gameState == GAME_OVER) {
            DrawTextCenter("Beceriksizsin", screenX, screenY - 50, 20, RED);
            DrawTextCenter("Senin yuzunden kurtler bagimsizlasip isyan cikartti ve", screenX, screenY, 20, WHITE);
            DrawTextCenter("baskent Ankara dustu, Allah belani versin.", screenX, screenY + 25, 20, WHITE);
            DrawTextCenter("Adam gibi oynayacaksan R'ye bas", screenX, screenY + 100, 20, WHITE);
            DrawTextCenter("Kurt isen ESC atabilirsin", screenX, screenY + 125, 20, WHITE);
    
            if (IsKeyPressed(KEY_R)) {
                player.init();
                boss.init();
                bossAttacks.clear();

                gameState = PLAYING;
            }

            if (IsKeyPressed(KEY_ESCAPE)) {
                CloseWindow();
                return 0;
            }
        } else if (gameState == WIN) {
            DrawTextCenter("Helal Olsun!", screenX, screenY - 50, 20, GREEN);
            DrawTextCenter("Senin sayende xtinfirev ve kurtler dalgayi aldi!", screenX, screenY, 20, WHITE);
            DrawTextCenter("Artik Turkiye Cumhuriyeti cok daha mutlu bir devlet olabilir", screenX, screenY + 25, 20, WHITE);
            DrawTextCenter("YASASIN TURKIYE CUMHURIYETI", screenX, screenY + 50, 20, RED);
            DrawTextCenter("Kurdistani tekrar bombalamak icin R'ye bas", screenX, screenY + 100, 20, WHITE);
            DrawTextCenter("Mutlu bir sekilde ayrilmak icin ESC'ye bas", screenX, screenY + 125, 20, WHITE);

            if (IsKeyPressed(KEY_R)) {
                player.init();
                boss.init();
                bossAttacks.clear();

                gameState = PLAYING;
            }

            if (IsKeyPressed(KEY_ESCAPE)) {
                CloseWindow();
                return 0;
            }
        } else if (gameState == MAIN_MENU) {
            DrawTextCenter("Kurdistan Bombalayici", screenX, screenY - 100, 20, WHITE);
            DrawTextCenter("Bu bir oyun projesidir tamamiyla eglence amaciyla uretilmistir", screenX, screenY - 75, 20, GRAY);
            // select difficulty
            DrawTextCenter("Zorluk Seçin", screenX, screenY + 25, 20, WHITE);
            DrawTextCenter("1. Kurt vatandas", screenX, screenY + 50, 20, GREEN);
            DrawTextCenter("2. Turk vatandas", screenX, screenY + 75, 20, YELLOW);
            DrawTextCenter("3. ULKUCU VATANDAS", screenX, screenY + 100, 20, DARKRED);

            // draw a (-) to indicate the current difficulty
            if (currentDifficulty == EASY) {
                DrawTextCenter("-", screenX - 95, screenY + 50, 20, GREEN);
            } else if (currentDifficulty == NORMAL) {
                DrawTextCenter("-", screenX - 100, screenY + 75, 20, YELLOW);
            } else if (currentDifficulty == HARD) {
                DrawTextCenter("-", screenX - 125, screenY + 100, 20, DARKRED);
            }

            DrawTextCenter("Cikmak icin ESC'ye bas", screenX, screenY + 150, 20, WHITE);
            DrawTextCenter("Secim yapmak icin 1, 2 veya 3'e bas", screenX, screenY + 175, 20, GRAY);

            if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1) || IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2) || IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3)) {
                if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1)) {
                    currentDifficulty = EASY;
                } else if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2)) {
                    currentDifficulty = NORMAL;
                } else if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3)) {
                    currentDifficulty = HARD;
                }
                player.init();
                boss.init();
                bossAttacks.clear();

                gameState = PLAYING;
            }

            if (IsKeyPressed(KEY_ESCAPE)) {
                CloseWindow();
                return 0;
            }
        }
        
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
