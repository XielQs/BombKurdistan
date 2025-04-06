#pragma once
#ifndef GLOBALBOUNDS_HPP
#define GLOBALBOUNDS_HPP

struct MovementBounds
{
    float left;
    float right;
    float top;
    float bottom;
};

extern MovementBounds movementBounds;

void InitMovementBounds(float screenWidth, float screenHeight);

#endif // GLOBALBOUNDS_HPP
