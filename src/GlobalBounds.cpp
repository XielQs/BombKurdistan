#include "GlobalBounds.hpp"
#include "Constants.hpp"

MovementBounds movementBounds;

void InitMovementBounds(float screenWidth, float screenHeight)
{
    movementBounds = {
        .left = SCREEN_PADDING + PLAYER_SIZE,
        .right = screenWidth - SCREEN_PADDING - PLAYER_SIZE,
        .top = BOSS_HEIGHT + SCREEN_PADDING + PLAYER_SIZE,
        .bottom = screenHeight - SCREEN_PADDING - PLAYER_SIZE
    };
}
