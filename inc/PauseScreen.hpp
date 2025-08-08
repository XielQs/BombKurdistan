#pragma once
#ifndef PAUSESCREEN_HPP
#define PAUSESCREEN_HPP

class PauseScreen
{
public:
    static void draw();
    static void handleInput();

private:
    static int selectedOption;
};

#endif // PAUSESCREEN_HPP
