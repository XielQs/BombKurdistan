#include "Game.hpp"

int main() {
    Game game;
    game.init();

    while (!WindowShouldClose()) {
        game.handleInput();
        game.update();
        game.draw();
    }

    return 0;
}
