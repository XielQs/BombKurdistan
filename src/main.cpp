#include "Game.hpp"

int main()
{
    Game game;
    game.init();

    while (!game.shouldClose && !WindowShouldClose())
        game.updateFrame();

    game.cleanup();
    CloseWindow();
    return 0;
}
