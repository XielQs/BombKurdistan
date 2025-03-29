#include "Difficulty.hpp"

Difficulty currentDifficulty = NORMAL;

const char* getDifficultyName(const Difficulty difficulty)
{
    switch (difficulty) {
        case EASY: return "Kurt vatandas";
        case NORMAL: return "Turk vatandas";
        case HARD: return "ULKUCU VATANDAS";
        default: return "Bilinmeyen";
    }
}
