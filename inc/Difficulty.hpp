#pragma once
#ifndef DIFFICULTY_HPP
#define DIFFICULTY_HPP

enum class Difficulty { EASY, NORMAL, HARD };
extern Difficulty currentDifficulty;

const char *getDifficultyName(Difficulty difficulty);

#endif // DIFFICULTY_HPP
