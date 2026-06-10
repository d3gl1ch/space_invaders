#ifndef GAME_H
#define GAME_H

#include "entities.h"
#include "levels.h"
#include "powerup.h"

// Экраны игры
typedef enum {
    GS_MENU = 0,
    GS_PLAYING = 1,
    GS_PAUSED = 2,
    GS_LEVEL_WIN = 3,
    GS_GAME_OVER = 4,
    GS_VICTORY = 5
} GameScreen;

// Полное состояние игры. HWND / HDC сюда не попадают никогда.
typedef struct {
    GameScreen screen;
    int currentLevel;
    int score;
    int hiScore;

    // Игрок 
    Player player;

    // Пришельцы
    Invader invaders[MAX_INVADERS];
    int invaderCount;
    int totalInvaders;

    // Движение колонны
    float swarmDir;
    float swarmDropLeft;

    // Пули
    Bullet playerBullets[MAX_PLAYER_BULLETS];
    Bullet enemyBullets[MAX_ENEMY_BULLETS];
    float enemyShootTimer;

    // Барьеры
    BarrierBlock barriers[MAX_BARRIERS][BARRIER_BLOCKS];
    int barrierRowCols;
    int barrierRowRows;

    // UFO
    UFO ufo;

    // Параметры уровня
    LevelParams lvl;

    // Power-ups
    PowerUp powerups[MAX_POWERUPS];
    ActiveEffects effects;

    // Таймеры / анимация
    float transitionTimer;
    float animTimer;
    int animFrame;

    // Ввод (заполняет модуль input)
    int keyLeft;
    int keyRight;
    int keyShoot;
} GameState;

// API логики
void Game_Init(GameState *gs);
void Game_InitLevel(GameState *gs);
void Game_Update(GameState *gs, float dt);
void Game_SaveHiScore(const GameState *gs);
void Game_LoadHiScore(GameState *gs);

#endif