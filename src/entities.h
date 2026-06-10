#ifndef ENTITIES_H
#define ENTITIES_H

#include <windows.h>

// Размеры поля
#define WINDOW_WIDTH    800
#define WINDOW_HEIGHT   650
#define FIELD_TOP       80

// Лимиты объектов
#define MAX_INVADERS       55
#define MAX_PLAYER_BULLETS  8
#define MAX_ENEMY_BULLETS  30
#define MAX_BARRIERS        4
#define BARRIER_BLOCKS     24   // 4 блока на барьер

// Размеры спрайтов (пиксели)
#define INVADER_W  36
#define INVADER_H  24
#define PLAYER_W   40
#define PLAYER_H   24
#define BULLET_W    4
#define BULLET_H   12
#define BLOCK_W     8
#define BLOCK_H     8

// Прямоугольник (для коллизий)
typedef struct {
    float x, y;   // позиция
    float w, h;   // ширина и высота
} Rect;

// Типы пришельцев
typedef enum {
    INV_TYPE_A = 0,   // нижние  — 10 очков 
    INV_TYPE_B = 1,   // средние — 20 очков 
    INV_TYPE_C = 2,   // верхние — 30 очков 
    INV_TYPE_UFO = 3    // летающая тарелка   
} InvaderType;

// Режим стрельбы врагов
typedef enum {
    SHOOT_STRAIGHT = 0,  // прямо вниз
    SHOOT_AIMED = 1,  // в текущую позицию игрока
    SHOOT_HOMING_SLOW = 2   // медленное самонаведение 
} EnemyShootMode;

// Пришелец
typedef struct {
    float x, y;
    InvaderType type;
    int alive;
    int animFrame;
    float animTimer;
} Invader;

// Пуля
typedef struct {
    float x, y;
    float vx, vy;
    float angle;    // текущий угол (рад) — используется для homing
    int active;
    int isEnemy;
} Bullet;

// Блок барьера
typedef struct {
    int x, y;
    int health;     // 3 = целый, 0 = разрушен 
} BarrierBlock;

// Игрок
typedef struct {
    float x, y;
    int lives;
    float shootCooldown;
    int invincible;
    float invincTimer;
} Player;

// UFO
typedef struct {
    float x, y;
    float speed;
    float spawnTimer;
    int active;
    int dir;       // +1 вправо, -1 влево 
} UFO;

// Параметры уровня
typedef struct {
    int levelNumber;
    float invaderSpeedBase;
    float invaderSpeedMult;
    float invaderDropStep;
    float enemyBulletSpeed;
    float enemyShootInterval;
    EnemyShootMode shootMode;
    float homingTurnRate;
    int rows;
    int cols;
    int hasUFO;
    float playerBulletSpeed;
    float playerShootCooldown;
    float playerSpeed;
} LevelParams;

#endif 