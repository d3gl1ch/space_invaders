#include "game.h"
#include "levels.h"
#include "powerup.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Прототипы
static void  initInvaders(GameState *gs);
static void  initBarriers(GameState *gs);
static void  initEffects(GameState *gs);
static void  updatePlayer(GameState *gs, float dt);
static void  updateSwarm(GameState *gs, float dt);
static void  updateEnemyShooting(GameState *gs, float dt);
static void  updateBullets(GameState *gs, float dt);
static void  updateUFO(GameState *gs, float dt);
static void  updatePowerUps(GameState *gs, float dt);
static void  updateEffects(GameState *gs, float dt);
static void  checkCollisions(GameState *gs);
static void  tryDropPowerUp(GameState *gs, float x, float y);
static void  applyPowerUp(GameState *gs, PowerUpType type);
static void  spawnEnemyBullet(GameState *gs, float fromX, float fromY);
static int   anyInvaderAlive(const GameState *gs);
static int   invaderReachedPlayer(const GameState *gs);
static int   rectsOverlap(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh);
static float clampf(float v, float lo, float hi);
static float randf(void);
static int   randRange(int lo, int hi);

// Математика
static float clampf(float v, float lo, float hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static float randf(void)
{
    return (float)rand() / ((float)RAND_MAX + 1.0f);
}

static int randRange(int lo, int hi)
{
    if (hi <= lo) return lo;
    return lo + rand() % (hi - lo + 1);
}

// Хай-скор
void Game_SaveHiScore(const GameState *gs)
{
    FILE *f = fopen("hiscore.dat", "wb");
    if (f) {
        fwrite(&gs->hiScore, sizeof(int), 1, f);
        fclose(f);
    }
}

void Game_LoadHiScore(GameState *gs)
{
    FILE *f = fopen("hiscore.dat", "rb");
    if (f) {
        if (fread(&gs->hiScore, sizeof(int), 1, f) != 1)
            gs->hiScore = 0;
        fclose(f);
    } else {
        gs->hiScore = 0;
    }
}

// Новая игра
void Game_Init(GameState *gs)
{
    memset(gs, 0, sizeof(GameState));
    gs->screen = GS_MENU;
    gs->currentLevel = 1;
    gs->score = 0;
    Game_LoadHiScore(gs);

    gs->player.x = (float)(WINDOW_WIDTH  / 2 - PLAYER_W / 2);
    gs->player.y = (float)(WINDOW_HEIGHT - 60);
    gs->player.lives = 3;
}

// Инициализация уровня
void Game_InitLevel(GameState *gs)
{
    Levels_GetParams(gs->currentLevel, &gs->lvl);

    memset(gs->playerBullets, 0, sizeof(gs->playerBullets));
    memset(gs->enemyBullets, 0, sizeof(gs->enemyBullets));
    memset(gs->powerups, 0, sizeof(gs->powerups));

    gs->player.x = (float)(WINDOW_WIDTH / 2 - PLAYER_W / 2);
    gs->player.y = (float)(WINDOW_HEIGHT - 60);
    gs->player.shootCooldown = 0.0f;
    gs->player.invincible = 0;
    gs->player.invincTimer = 0.0f;

    initInvaders(gs);
    initBarriers(gs);
    initEffects(gs);

    gs->swarmDir = 1.0f;
    gs->swarmDropLeft = 0.0f;

    gs->enemyShootTimer = gs->lvl.enemyShootInterval;

    gs->ufo.active = 0;
    gs->ufo.spawnTimer = 15.0f + randf() * 15.0f;
    gs->ufo.speed = 120.0f + (float)gs->currentLevel * 10.0f;
    gs->ufo.x = 0.0f;
    gs->ufo.y = 0.0f;
    gs->ufo.dir = 1;

    gs->animTimer = 0.0f;
    gs->animFrame = 0;
    gs->transitionTimer = 0.0f;
    gs->screen = GS_PLAYING;
}

// Инициализация эффектов (сбрасываем таймеры, но базовые значения берём из lvl)
static void initEffects(GameState *gs)
{
    memset(&gs->effects, 0, sizeof(ActiveEffects));
    gs->effects.basePlayerBulletSpeed = gs->lvl.playerBulletSpeed;
    gs->effects.basePlayerShootCooldown = gs->lvl.playerShootCooldown;
}

// Расстановка пришельцев
static void initInvaders(GameState *gs)
{
    int r, c, idx;
    int rows  = gs->lvl.rows;
    int cols  = gs->lvl.cols;
    int gridW = cols * (INVADER_W + 12);
    float startX = (float)((WINDOW_WIDTH - gridW) / 2);
    float startY = (float)(FIELD_TOP + 30);

    memset(gs->invaders, 0, sizeof(gs->invaders));
    gs->totalInvaders = rows * cols;
    gs->invaderCount = gs->totalInvaders;

    for (r = 0; r < rows; r++) {
        for (c = 0; c < cols; c++) {
            idx = r * cols + c;
            if (idx >= MAX_INVADERS) break;
            gs->invaders[idx].x = startX + (float)(c * (INVADER_W + 12));
            gs->invaders[idx].y = startY + (float)(r * (INVADER_H + 16));
            gs->invaders[idx].alive = 1;
            gs->invaders[idx].animFrame = 0;
            gs->invaders[idx].animTimer = 0.0f;
            if (r == 0) gs->invaders[idx].type = INV_TYPE_C;
            else if (r <= 2) gs->invaders[idx].type = INV_TYPE_B;
            else gs->invaders[idx].type = INV_TYPE_A;
        }
    }
}

// Барьеры
static void initBarriers(GameState *gs)
{
    int b, r, c, bi;
    int bw = 5;
    int bh = 3;
    int barrierPixW = bw * BLOCK_W;
    int gap = (WINDOW_WIDTH - MAX_BARRIERS * barrierPixW) / (MAX_BARRIERS + 1);
    int baseY = WINDOW_HEIGHT - 120;

    gs->barrierRowCols = bw;
    gs->barrierRowRows = bh;

    for (b = 0; b < MAX_BARRIERS; b++) {
        int bx = gap + b * (barrierPixW + gap);
        for (r = 0; r < bh; r++) {
            for (c = 0; c < bw; c++) {
                bi = r * bw + c;
                gs->barriers[b][bi].x = bx + c * BLOCK_W;
                gs->barriers[b][bi].y = baseY + r * BLOCK_H;
                gs->barriers[b][bi].health = 2;
            }
        }
    }
}

// Главный апдейт
void Game_Update(GameState *gs, float dt)
{
    switch (gs->screen) {

    case GS_MENU:
        break;

    case GS_PLAYING:
        updateEffects(gs, dt);
        updatePlayer(gs, dt);
        updateSwarm(gs, dt);
        updateEnemyShooting(gs, dt);
        updateBullets(gs, dt);
        updateUFO(gs, dt);
        updatePowerUps(gs, dt);
        checkCollisions(gs);

        gs->animTimer += dt;
        if (gs->animTimer >= 0.5f) {
            gs->animTimer = 0.0f;
            gs->animFrame = 1 - gs->animFrame;
        }

        if (!anyInvaderAlive(gs)) {
            if (gs->score > gs->hiScore) {
                gs->hiScore = gs->score;
                Game_SaveHiScore(gs);
            }
            if (gs->currentLevel >= MAX_LEVELS) {
                gs->screen = GS_VICTORY;
                gs->transitionTimer = 0.0f;
            } else {
                gs->screen = GS_LEVEL_WIN;
                gs->transitionTimer = 3.0f;
            }
        }

        if (gs->screen == GS_PLAYING && invaderReachedPlayer(gs)) {
            gs->player.lives = 0;
            gs->screen = GS_GAME_OVER;
            gs->transitionTimer = 4.0f;
            if (gs->score > gs->hiScore) {
                gs->hiScore = gs->score;
                Game_SaveHiScore(gs);
            }
        }
        break;

    case GS_PAUSED:
        break;

    case GS_LEVEL_WIN:
        gs->transitionTimer -= dt;
        if (gs->transitionTimer <= 0.0f) {
            gs->currentLevel++;
            Game_InitLevel(gs);
        }
        break;

    case GS_GAME_OVER:
        gs->transitionTimer -= dt;
        break;

    case GS_VICTORY:
        break;
    }
}

// Обновление активных эффектов
static void updateEffects(GameState *gs, float dt)
{
    ActiveEffects *e = &gs->effects;

    // Щит
    if (e->shieldTimer > 0.0f) {
        e->shieldTimer -= dt;
        if (e->shieldTimer <= 0.0f) {
            e->shieldTimer = 0.0f;
            gs->player.invincible = 0;
        }
    }

    // Замедление врагов
    if (e->slowTimer > 0.0f) {
        e->slowTimer -= dt;
        if (e->slowTimer <= 0.0f)
            e->slowTimer = 0.0f;
    }

    // Быстрые выстрелы
    if (e->fastShootTimer > 0.0f) {
        e->fastShootTimer -= dt;
        if (e->fastShootTimer <= 0.0f) {
            e->fastShootTimer = 0.0f;
            // Восстанавливаем базовые значения
            gs->lvl.playerBulletSpeed = e->basePlayerBulletSpeed;
            gs->lvl.playerShootCooldown = e->basePlayerShootCooldown;
        }
    }

    // Тройной выстрел
    if (e->tripleTimer > 0.0f) {
        e->tripleTimer -= dt;
        if (e->tripleTimer <= 0.0f)
            e->tripleTimer = 0.0f;
    }

    // Взрыв
    if (e->bombTimer > 0.0f) {
        e->bombTimer -= dt;
        if (e->bombTimer <= 0.0f)
            e->bombTimer = 0.0f;
    }
}

// Игрок
static void updatePlayer(GameState *gs, float dt)
{
    int   i;
    float newX = gs->player.x;

    if (gs->keyLeft) newX -= gs->lvl.playerSpeed * dt;
    if (gs->keyRight) newX += gs->lvl.playerSpeed * dt;
    gs->player.x = clampf(newX, 0.0f, (float)(WINDOW_WIDTH - PLAYER_W));

    if (gs->player.shootCooldown > 0.0f)
        gs->player.shootCooldown -= dt;

    if (gs->keyShoot && gs->player.shootCooldown <= 0.0f) {
        int tripleActive = (gs->effects.tripleTimer > 0.0f);
        int fired = 0;

        // Центральная пуля — всегда
        for (i = 0; i < MAX_PLAYER_BULLETS; i++) {
            if (!gs->playerBullets[i].active) {
                gs->playerBullets[i].active = 1;
                gs->playerBullets[i].isEnemy = 0;
                gs->playerBullets[i].x = gs->player.x + PLAYER_W / 2.0f - BULLET_W / 2.0f;
                gs->playerBullets[i].y = gs->player.y;
                gs->playerBullets[i].vx = 0.0f;
                gs->playerBullets[i].vy = -gs->lvl.playerBulletSpeed;
                gs->playerBullets[i].angle = -1.5707963f;
                fired = 1;
                break;
            }
        }

        // Боковые пули при тройном выстреле
        if (fired && tripleActive) {
            // Левая
            for (i = 0; i < MAX_PLAYER_BULLETS; i++) {
                if (!gs->playerBullets[i].active) {
                    float speed = gs->lvl.playerBulletSpeed;
                    gs->playerBullets[i].active = 1;
                    gs->playerBullets[i].isEnemy = 0;
                    gs->playerBullets[i].x = gs->player.x + PLAYER_W / 2.0f - BULLET_W / 2.0f;
                    gs->playerBullets[i].y = gs->player.y;
                    /* Угол ~15° влево */
                    gs->playerBullets[i].vx = -speed * 0.26f;
                    gs->playerBullets[i].vy = -speed * 0.97f;
                    gs->playerBullets[i].angle = -1.5707963f - 0.26f;
                    break;
                }
            }
            // Правая
            for (i = 0; i < MAX_PLAYER_BULLETS; i++) {
                if (!gs->playerBullets[i].active) {
                    float speed = gs->lvl.playerBulletSpeed;
                    gs->playerBullets[i].active = 1;
                    gs->playerBullets[i].isEnemy = 0;
                    gs->playerBullets[i].x = gs->player.x + PLAYER_W / 2.0f - BULLET_W / 2.0f;
                    gs->playerBullets[i].y = gs->player.y;
                    gs->playerBullets[i].vx =  speed * 0.26f;
                    gs->playerBullets[i].vy = -speed * 0.97f;
                    gs->playerBullets[i].angle = -1.5707963f + 0.26f;
                    break;
                }
            }
        }

        if (fired)
            gs->player.shootCooldown = gs->lvl.playerShootCooldown;
    }

    // Неуязвимость (если НЕ от щита — щит управляется отдельно) 
    if (gs->player.invincible && gs->effects.shieldTimer <= 0.0f) {
        gs->player.invincTimer -= dt;
        if (gs->player.invincTimer <= 0.0f) {
            gs->player.invincible = 0;
            gs->player.invincTimer = 0.0f;
        }
    }
}

// Движение колонны
static void updateSwarm(GameState *gs, float dt)
{
    int   i;
    float aliveRatio, baseSpeed, currentSpeed, move, minX, maxX;
    float slowMult;

    if (gs->totalInvaders <= 0) return;

    aliveRatio   = (float)gs->invaderCount / (float)gs->totalInvaders;
    baseSpeed    = gs->lvl.invaderSpeedBase + (1.0f - aliveRatio) * gs->lvl.invaderSpeedBase * (gs->lvl.invaderSpeedMult - 1.0f);

    // Бонус замедления
    slowMult = (gs->effects.slowTimer > 0.0f) ? 0.35f : 1.0f;
    currentSpeed = baseSpeed * slowMult;

    if (gs->swarmDropLeft > 0.0f) {
        float drop = currentSpeed * dt;
        if (drop > gs->swarmDropLeft) drop = gs->swarmDropLeft;
        gs->swarmDropLeft -= drop;
        for (i = 0; i < MAX_INVADERS; i++) {
            if (gs->invaders[i].alive)
                gs->invaders[i].y += drop;
        }
        return;
    }

    move = gs->swarmDir * currentSpeed * dt;
    for (i = 0; i < MAX_INVADERS; i++) {
        if (gs->invaders[i].alive)
            gs->invaders[i].x += move;
    }

    minX = (float)WINDOW_WIDTH;
    maxX = 0.0f;
    for (i = 0; i < MAX_INVADERS; i++) {
        if (!gs->invaders[i].alive) continue;
        if (gs->invaders[i].x < minX) minX = gs->invaders[i].x;
        if (gs->invaders[i].x + INVADER_W > maxX) maxX = gs->invaders[i].x + INVADER_W;
    }

    if (gs->swarmDir > 0.0f && maxX >= (float)(WINDOW_WIDTH - 4)) {
        gs->swarmDir = -1.0f;
        gs->swarmDropLeft = gs->lvl.invaderDropStep;
    } else if (gs->swarmDir < 0.0f && minX <= 4.0f) {
        gs->swarmDir =  1.0f;
        gs->swarmDropLeft = gs->lvl.invaderDropStep;
    }
}

// Стрельба врагов
static void updateEnemyShooting(GameState *gs, float dt)
{
    int col, r, found, idx, tries;
    int cols = gs->lvl.cols;
    int rows = gs->lvl.rows;

    gs->enemyShootTimer -= dt;
    if (gs->enemyShootTimer > 0.0f) return;
    gs->enemyShootTimer = gs->lvl.enemyShootInterval;

    tries = 0;
    while (tries < 20) {
        col = randRange(0, cols - 1);
        found = -1;
        for (r = rows - 1; r >= 0; r--) {
            idx = r * cols + col;
            if (idx < MAX_INVADERS && gs->invaders[idx].alive) {
                found = idx;
                break;
            }
        }
        if (found >= 0) {
            float bx = gs->invaders[found].x + INVADER_W / 2.0f - BULLET_W / 2.0f;
            float by = gs->invaders[found].y + (float)INVADER_H;
            spawnEnemyBullet(gs, bx, by);
            break;
        }
        tries++;
    }
}

// Вражеская пуля
static void spawnEnemyBullet(GameState *gs, float fromX, float fromY)
{
    int   i;
    float speed = gs->lvl.enemyBulletSpeed;
    float dx, dy, len, angle;

    for (i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (gs->enemyBullets[i].active) continue;

        gs->enemyBullets[i].active = 1;
        gs->enemyBullets[i].isEnemy = 1;
        gs->enemyBullets[i].x = fromX;
        gs->enemyBullets[i].y = fromY;

        switch (gs->lvl.shootMode) {
        case SHOOT_STRAIGHT:
            gs->enemyBullets[i].vx = 0.0f;
            gs->enemyBullets[i].vy = speed;
            gs->enemyBullets[i].angle = 1.5707963f;
            break;

        case SHOOT_AIMED:
        case SHOOT_HOMING_SLOW:
            dx = (gs->player.x + PLAYER_W / 2.0f) - fromX;
            dy = (gs->player.y + PLAYER_H / 2.0f) - fromY;
            len = sqrtf(dx * dx + dy * dy);
            if (len < 1.0f) len = 1.0f;
            angle = atan2f(dy, dx);
            gs->enemyBullets[i].vx = (dx / len) * speed;
            gs->enemyBullets[i].vy = (dy / len) * speed;
            gs->enemyBullets[i].angle = angle;
            break;
        }
        break;
    }
}

// Обновление пуль
static void updateBullets(GameState *gs, float dt)
{
    int i;

    for (i = 0; i < MAX_PLAYER_BULLETS; i++) {
        if (!gs->playerBullets[i].active) continue;
        gs->playerBullets[i].x += gs->playerBullets[i].vx * dt;
        gs->playerBullets[i].y += gs->playerBullets[i].vy * dt;
        if (gs->playerBullets[i].y + BULLET_H < 0.0f || gs->playerBullets[i].x < -20.0f || gs->playerBullets[i].x > (float)WINDOW_WIDTH + 20.0f)
        {
            gs->playerBullets[i].active = 0;
        }
    }

    for (i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!gs->enemyBullets[i].active) continue;

        if (gs->lvl.shootMode == SHOOT_HOMING_SLOW) {
            float px = gs->player.x + PLAYER_W / 2.0f;
            float py = gs->player.y + PLAYER_H / 2.0f;
            float bx = gs->enemyBullets[i].x + BULLET_W / 2.0f;
            float by = gs->enemyBullets[i].y + BULLET_H / 2.0f;
            float tgt = atan2f(py - by, px - bx);
            float cur = gs->enemyBullets[i].angle;
            float diff = tgt - cur;
            float maxT = gs->lvl.homingTurnRate * dt;

            while (diff >  3.14159265f) diff -= 6.28318530f;
            while (diff < -3.14159265f) diff += 6.28318530f;
            if (diff >  maxT) diff =  maxT;
            else if (diff < -maxT) diff = -maxT;

            cur += diff;
            gs->enemyBullets[i].angle = cur;
            gs->enemyBullets[i].vx = cosf(cur) * gs->lvl.enemyBulletSpeed;
            gs->enemyBullets[i].vy = sinf(cur) * gs->lvl.enemyBulletSpeed;
        }

        gs->enemyBullets[i].x += gs->enemyBullets[i].vx * dt;
        gs->enemyBullets[i].y += gs->enemyBullets[i].vy * dt;

        if (gs->enemyBullets[i].y > (float)WINDOW_HEIGHT + 20.0f || gs->enemyBullets[i].y < -20.0f || gs->enemyBullets[i].x > (float)WINDOW_WIDTH  + 20.0f || gs->enemyBullets[i].x < -20.0f)
        {
            gs->enemyBullets[i].active = 0;
        }
    }
}

// UFO
static void updateUFO(GameState *gs, float dt)
{
    if (!gs->lvl.hasUFO) return;

    if (!gs->ufo.active) {
        gs->ufo.spawnTimer -= dt;
        if (gs->ufo.spawnTimer <= 0.0f) {
            gs->ufo.active = 1;
            gs->ufo.dir = (rand() & 1) ? 1 : -1;
            gs->ufo.y = (float)(FIELD_TOP - 10);
            gs->ufo.x = (gs->ufo.dir > 0) ? -50.0f : (float)(WINDOW_WIDTH + 10);
            gs->ufo.spawnTimer = 15.0f + randf() * 20.0f;
        }
        return;
    }

    gs->ufo.x += (float)gs->ufo.dir * gs->ufo.speed * dt;
    if (gs->ufo.x > (float)(WINDOW_WIDTH + 60) || gs->ufo.x < -60.0f)
        gs->ufo.active = 0;
}

// Power-up: обновление падающих шаров
static void updatePowerUps(GameState *gs, float dt)
{
    int i;
    for (i = 0; i < MAX_POWERUPS; i++) {
        if (!gs->powerups[i].active) continue;

        gs->powerups[i].y += gs->powerups[i].vy * dt;

        // Вышел за нижний край — пропал
        if (gs->powerups[i].y > (float)WINDOW_HEIGHT) {
            gs->powerups[i].active = 0;
        }
    }
}

// Попытка создать дроп после убийства
static void tryDropPowerUp(GameState *gs, float x, float y)
{
    int i, slot;

    // Шанс дропа
    if ((rand() % 100) >= PU_DROP_CHANCE) return;

    // Ищем свободный слот
    slot = -1;
    for (i = 0; i < MAX_POWERUPS; i++) {
        if (!gs->powerups[i].active) { slot = i; break; }
    }
    if (slot < 0) return; // нет места

    gs->powerups[slot].active = 1;
    gs->powerups[slot].x = x - PU_W / 2.0f;
    gs->powerups[slot].y = y;
    gs->powerups[slot].vy = PU_FALL_SPEED;
    gs->powerups[slot].type = (PowerUpType)(rand() % PU_COUNT);
}

// Применение усиления
static void applyPowerUp(GameState *gs, PowerUpType type)
{
    ActiveEffects *e = &gs->effects;

    switch (type) {

    case PU_EXTRA_LIFE:
        gs->player.lives++;
        break;

    case PU_SHIELD:
        e->shieldTimer = PU_SHIELD_DURATION;
        gs->player.invincible = 1;
        gs->player.invincTimer = PU_SHIELD_DURATION; // для мигания не нужно
        break;

    case PU_SLOW:
        e->slowTimer = PU_SLOW_DURATION;
        break;

    case PU_FAST_SHOOT:
        // Сохраняем базу только если эффект ещё не активен
        if (e->fastShootTimer <= 0.0f) {
            e->basePlayerBulletSpeed = gs->lvl.playerBulletSpeed;
            e->basePlayerShootCooldown = gs->lvl.playerShootCooldown;
        }
        gs->lvl.playerBulletSpeed *= 1.7f;
        gs->lvl.playerShootCooldown *= 0.4f;
        e->fastShootTimer = PU_FAST_SHOOT_DURATION;
        break;

    case PU_TRIPLE_SHOT:
        e->tripleTimer = PU_TRIPLE_DURATION;
        break;

    case PU_BOMB:
        e->bombTimer = PU_BOMB_DURATION;
        break;

    default:
        break;
    }
}

// Взрыв: убиваем всех пришельцев в радиусе
#define BOMB_RADIUS 80.0f

static void triggerBomb(GameState *gs, float bx, float by)
{
    int   j;
    float cx, cy, dx, dy;

    for (j = 0; j < MAX_INVADERS; j++) {
        if (!gs->invaders[j].alive) continue;
        cx = gs->invaders[j].x + INVADER_W / 2.0f;
        cy = gs->invaders[j].y + INVADER_H / 2.0f;
        dx = cx - bx;
        dy = cy - by;
        if (dx * dx + dy * dy <= BOMB_RADIUS * BOMB_RADIUS) {
            gs->invaders[j].alive = 0;
            gs->invaderCount--;
            switch (gs->invaders[j].type) {
            case INV_TYPE_A: gs->score += 10;  break;
            case INV_TYPE_B: gs->score += 20;  break;
            case INV_TYPE_C: gs->score += 30;  break;
            case INV_TYPE_UFO: gs->score += 100; break;
            }
        }
    }
}

// AABB
static int rectsOverlap(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh)
{
    return (ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by);
}

// Коллизии
static void checkCollisions(GameState *gs)
{
    int i, j, b, bi;

    // Пули игрока vs пришельцы
    for (i = 0; i < MAX_PLAYER_BULLETS; i++) {
        if (!gs->playerBullets[i].active) continue;
        for (j = 0; j < MAX_INVADERS; j++) {
            if (!gs->invaders[j].alive) continue;
            if (!rectsOverlap(
                    gs->playerBullets[i].x, gs->playerBullets[i].y,
                    (float)BULLET_W, (float)BULLET_H,
                    gs->invaders[j].x, gs->invaders[j].y, (float)INVADER_W, (float)INVADER_H)) continue;

            // Взрыв активен?
            if (gs->effects.bombTimer > 0.0f) {
                float cx = gs->invaders[j].x + INVADER_W / 2.0f;
                float cy = gs->invaders[j].y + INVADER_H / 2.0f;
                triggerBomb(gs, cx, cy);
            } else {
                gs->invaders[j].alive = 0;
                gs->invaderCount--;
                switch (gs->invaders[j].type) {
                case INV_TYPE_A: gs->score += 10;  break;
                case INV_TYPE_B: gs->score += 20;  break;
                case INV_TYPE_C: gs->score += 30;  break;
                case INV_TYPE_UFO: gs->score += 100; break;
                }
            }

            // Дроп
            tryDropPowerUp(gs,
                gs->invaders[j].x + INVADER_W / 2.0f,
                gs->invaders[j].y + INVADER_H);

            gs->playerBullets[i].active = 0;
            break;
        }
    }

    // Пули игрока vs UFO
    if (gs->ufo.active) {
        for (i = 0; i < MAX_PLAYER_BULLETS; i++) {
            if (!gs->playerBullets[i].active) continue;
            if (!rectsOverlap(
                    gs->playerBullets[i].x, gs->playerBullets[i].y,
                    (float)BULLET_W, (float)BULLET_H,
                    gs->ufo.x, gs->ufo.y, 48.0f, 20.0f)) continue;

            gs->playerBullets[i].active = 0;
            gs->ufo.active = 0;
            gs->score += randRange(1, 6) * 50;
            /* UFO тоже может дропнуть усиление */
            tryDropPowerUp(gs, gs->ufo.x + 24.0f, gs->ufo.y + 20.0f);
            break;
        }
    }

    // Пули игрока vs барьеры
    for (i = 0; i < MAX_PLAYER_BULLETS; i++) {
        if (!gs->playerBullets[i].active) continue;
        for (b = 0; b < MAX_BARRIERS; b++) {
            for (bi = 0; bi < BARRIER_BLOCKS; bi++) {
                if (gs->barriers[b][bi].health <= 0) continue;
                if (!rectsOverlap(
                        gs->playerBullets[i].x, gs->playerBullets[i].y,
                        (float)BULLET_W, (float)BULLET_H,
                        (float)gs->barriers[b][bi].x, (float)gs->barriers[b][bi].y,
                        (float)BLOCK_W, (float)BLOCK_H)) continue;
                gs->playerBullets[i].active = 0;
                gs->barriers[b][bi].health--;
            }
        }
    }

    // Вражеские пули vs игрок
    if (!gs->player.invincible) {
        for (i = 0; i < MAX_ENEMY_BULLETS; i++) {
            if (!gs->enemyBullets[i].active) continue;
            if (!rectsOverlap(
                    gs->enemyBullets[i].x, gs->enemyBullets[i].y,
                    (float)BULLET_W, (float)BULLET_H,
                    gs->player.x, gs->player.y,
                    (float)PLAYER_W, (float)PLAYER_H)) continue;

            gs->enemyBullets[i].active = 0;
            gs->player.lives--;
            gs->player.invincible  = 1;
            gs->player.invincTimer = 2.0f;

            if (gs->player.lives <= 0) {
                if (gs->score > gs->hiScore) {
                    gs->hiScore = gs->score;
                    Game_SaveHiScore(gs);
                }
                gs->screen = GS_GAME_OVER;
                gs->transitionTimer = 4.0f;
            }
            break;
        }
    }

    // Вражеские пули vs барьеры
    for (i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!gs->enemyBullets[i].active) continue;
        for (b = 0; b < MAX_BARRIERS; b++) {
            for (bi = 0; bi < BARRIER_BLOCKS; bi++) {
                if (gs->barriers[b][bi].health <= 0) continue;
                if (!rectsOverlap(
                        gs->enemyBullets[i].x, gs->enemyBullets[i].y,
                        (float)BULLET_W, (float)BULLET_H,
                        (float)gs->barriers[b][bi].x, (float)gs->barriers[b][bi].y,
                        (float)BLOCK_W, (float)BLOCK_H)) continue;
                gs->enemyBullets[i].active = 0;
                gs->barriers[b][bi].health--;
            }
        }
    }

    // Пришельцы vs барьеры
    for (j = 0; j < MAX_INVADERS; j++) {
        if (!gs->invaders[j].alive) continue;
        for (b = 0; b < MAX_BARRIERS; b++) {
            for (bi = 0; bi < BARRIER_BLOCKS; bi++) {
                if (gs->barriers[b][bi].health <= 0) continue;
                if (!rectsOverlap(
                        gs->invaders[j].x, gs->invaders[j].y,
                        (float)INVADER_W, (float)INVADER_H,
                        (float)gs->barriers[b][bi].x, (float)gs->barriers[b][bi].y,
                        (float)BLOCK_W, (float)BLOCK_H)) continue;
                gs->barriers[b][bi].health = 0;
            }
        }
    }

    // Power-ups vs игрок
    for (i = 0; i < MAX_POWERUPS; i++) {
        if (!gs->powerups[i].active) continue;
        if (!rectsOverlap(
                gs->powerups[i].x, gs->powerups[i].y,
                (float)PU_W, (float)PU_H,
                gs->player.x, gs->player.y,
                (float)PLAYER_W, (float)PLAYER_H)) continue;

        gs->powerups[i].active = 0;
        applyPowerUp(gs, gs->powerups[i].type);
    }
}

// Вспомогательные
static int anyInvaderAlive(const GameState *gs)
{
    int i;
    for (i = 0; i < MAX_INVADERS; i++)
        if (gs->invaders[i].alive) return 1;
    return 0;
}

static int invaderReachedPlayer(const GameState *gs)
{
    int i;
    for (i = 0; i < MAX_INVADERS; i++) {
        if (!gs->invaders[i].alive) continue;
        if (gs->invaders[i].y + INVADER_H >= gs->player.y)
            return 1;
    }
    return 0;
}