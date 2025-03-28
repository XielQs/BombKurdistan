#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define ATTACK_OFFSET 150
#include <raylib.h>
#include "Player.h"
#include "Boss.h"
#include "BossAttack.h"
#include "Bomb.h"
#include <vector>
#include <math.h>

std::vector<BossAttack> bossAttacks;
std::vector<Bomb> bombs;
float bombTimer = 0.f;
float attackTimer = 0.f;
enum GameState {
    PLAYING,
    GAME_OVER,
    WIN
};
GameState gameState = PLAYING;
bool isMuted = false;

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
        if (gameState == PLAYING) {
            if (IsKeyPressed(KEY_M)) {
                isMuted = !isMuted;
                if (isMuted) {
                    StopSound(bgMusic);
                } else {
                    PlaySound(bgMusic);
                }
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
                    int attackCount = GetRandomValue(1, 2);
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
            DrawText("Beceriksizsin", GetScreenWidth() / 2 - 50, GetScreenHeight() / 2 - 50, 20, RED);
            DrawText("Senin yuzunden kurtler bagimsizlasip isyan cikartti ve", GetScreenWidth() / 2 - 250, GetScreenHeight() / 2, 20, WHITE);
            DrawText("baskent Ankara dustu, Allah belani versin.", GetScreenWidth() / 2 - 200, GetScreenHeight() / 2 + 20, 20, WHITE);
            DrawText("Adam gibi oynayacaksan R'ye bas", GetScreenWidth() / 2 - 150, GetScreenHeight() / 2 + 100, 20, WHITE);
            DrawText("Kurt isen ESC atabilirsin", GetScreenWidth() / 2 - 100, GetScreenHeight() / 2 + 140, 20, WHITE);
    
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
            DrawText("Helal Olsun!", GetScreenWidth() / 2 - 50, GetScreenHeight() / 2 - 50, 20, GREEN);
            DrawText("Senin sayende xtinfirev ve kurtler dalgayi aldi!", GetScreenWidth() / 2 - 200, GetScreenHeight() / 2, 20, WHITE);
            DrawText("Artik Turkiye Cumhuriyeti cok daha mutlu bir devlet olabilir", GetScreenWidth() / 2 - 250, GetScreenHeight() / 2 + 20, 20, WHITE);
            DrawText("YASASIN TURKIYE CUMHURIYETI", GetScreenWidth() / 2 - 150, GetScreenHeight() / 2 + 45, 20, RED);
            DrawText("Kurdistani tekrar bombalamak icin R'ye bas", GetScreenWidth() / 2 - 200, GetScreenHeight() / 2 + 100, 20, WHITE);
            DrawText("Mutlu bir sekilde ayrilmak icin ESC'ye bas", GetScreenWidth() / 2 - 200, GetScreenHeight() / 2 + 140, 20, WHITE);
    
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
        }
        
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
