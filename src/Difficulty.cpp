#include "Difficulty.hpp"

// we can't use auto in here because C++ is a stupid fucking moron,
// and you can't use auto with extern
Difficulty currentDifficulty = Difficulty::NORMAL;

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
