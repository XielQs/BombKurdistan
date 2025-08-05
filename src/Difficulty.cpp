#include "Difficulty.hpp"

auto currentDifficulty = Difficulty::NORMAL;

const char *getDifficultyName(const Difficulty difficulty)
{
    switch (difficulty) {
        case Difficulty::EASY:
            return "Kurt vatandas";
        case Difficulty::NORMAL:
            return "Turk vatandas";
        case Difficulty::HARD:
            return "ULKUCU VATANDAS";
        default:
            return "Bilinmeyen";
    }
}
