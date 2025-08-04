#include "GlobalBounds.hpp"
#include "Constants.hpp"

MovementBounds movementBounds;

void InitMovementBounds(const int screenWidth, const int screenHeight)
{
    movementBounds = {.left = SCREEN_PADDING + PLAYER_SIZE,
                      .right = static_cast<float>(screenWidth) - SCREEN_PADDING - PLAYER_SIZE,
                      .top = BOSS_HEIGHT + SCREEN_PADDING + PLAYER_SIZE,
                      .bottom = static_cast<float>(screenHeight) - SCREEN_PADDING - PLAYER_SIZE};
}
