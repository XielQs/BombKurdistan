#include "Game.hpp"

Game game;

int main()
{
    game.init();

    while (!game.shouldClose && !WindowShouldClose())
        game.updateFrame();

    game.cleanup();
    CloseWindow();
    return 0;
}
