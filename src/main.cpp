#include "Game.hpp"
#ifdef PLATFORM_WEB
#include "Constants.hpp"
#include <emscripten/emscripten.h>
#endif

Game game;

Vector2 fakeGamepadAxis = {0.f, 0.f};

#ifdef PLATFORM_WEB
extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void js_set_axis(float x, float y) {
        if (fabs(x) > 0.1f || fabs(y) > 0.1f) {
            game.player->position.x += x * PLAYER_SPEED * GetFrameTime();
            game.player->position.y += y * PLAYER_SPEED * GetFrameTime();
        }
    }
    EMSCRIPTEN_KEEPALIVE
    void js_set_gamestate(int state) {
        game.setGameState((GameState)state);
    }
}
#endif

int main() {
    game.init();

    #ifdef PLATFORM_WEB
    emscripten_set_main_loop(
        []() {
            game.updateFrame();
        },
        0, 1);
    #else
    while (!game.shouldClose && !WindowShouldClose()) game.updateFrame();
    #endif

    game.cleanup();
    CloseWindow();
    return 0;
}
