#ifndef POWERUP_H
#define POWERUP_H

#include <windows.h>

// Типы усилений
typedef enum {
    PU_EXTRA_LIFE = 0,  // +1 жизнь (перманентно)          
    PU_SHIELD = 1,  // щит на время                    
    PU_SLOW = 2,  // замедление врагов               
    PU_FAST_SHOOT = 3,  // быстрые выстрелы игрока         
    PU_TRIPLE_SHOT = 4,  // тройной выстрел                 
    PU_BOMB = 5,  // взрыв при попадании пули        
    PU_COUNT = 6   // кол-во типов (служебное)        
} PowerUpType;

// Один падающий шар
#define MAX_POWERUPS 8

typedef struct {
    float x, y;
    float vy;         // скорость падения (px/s)  
    PowerUpType type;
    int active;
} PowerUp;

// Активные эффекты на игрока
typedef struct {
    // Длительность каждого эффекта (<=0 = неактивен) 
    float shieldTimer;
    float slowTimer;
    float fastShootTimer;
    float tripleTimer;
    float bombTimer;

    // Исходные значения скорости пуль/cooldown для восстановления 
    float basePlayerBulletSpeed;
    float basePlayerShootCooldown;
} ActiveEffects;

// Длительности (секунды)
#define PU_SHIELD_DURATION 10.0f
#define PU_SLOW_DURATION 8.0f
#define PU_FAST_SHOOT_DURATION 5.0f
#define PU_TRIPLE_DURATION 5.0f
#define PU_BOMB_DURATION 5.0f

// Размер шара
#define PU_W 22
#define PU_H 22

// Шанс дропа на каждое убийство (0..100)
#define PU_DROP_CHANCE 10   // 10% 

// Скорость падения шара
#define PU_FALL_SPEED 90.0f

#endif 