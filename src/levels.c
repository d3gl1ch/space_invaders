#include "levels.h"
#include <string.h>

void Levels_GetParams(int levelNumber, LevelParams *out)
{
    if (levelNumber < 1) levelNumber = 1;
    if (levelNumber > MAX_LEVELS) levelNumber = MAX_LEVELS;

    memset(out, 0, sizeof(LevelParams));
    out->levelNumber = levelNumber;

    switch (levelNumber) {
    case 1:
        out->invaderSpeedBase = 40.0f;
        out->invaderSpeedMult = 1.6f;
        out->invaderDropStep = 16.0f;
        out->enemyBulletSpeed = 160.0f;
        out->enemyShootInterval = 2.5f;
        out->shootMode = SHOOT_STRAIGHT;
        out->homingTurnRate = 0.0f;
        out->rows = 3;
        out->cols = 9;
        out->hasUFO = 0;
        out->playerBulletSpeed = 420.0f;
        out->playerShootCooldown = 0.45f;
        out->playerSpeed = 220.0f;
        break;
    case 2:
        out->invaderSpeedBase = 50.0f;
        out->invaderSpeedMult = 1.7f;
        out->invaderDropStep = 16.0f;
        out->enemyBulletSpeed = 180.0f;
        out->enemyShootInterval = 2.2f;
        out->shootMode = SHOOT_STRAIGHT;
        out->homingTurnRate = 0.0f;
        out->rows = 4;
        out->cols = 10;
        out->hasUFO = 1;
        out->playerBulletSpeed = 430.0f;
        out->playerShootCooldown = 0.42f;
        out->playerSpeed = 225.0f;
        break;
    case 3:
        out->invaderSpeedBase = 60.0f;
        out->invaderSpeedMult = 1.8f;
        out->invaderDropStep = 18.0f;
        out->enemyBulletSpeed = 200.0f;
        out->enemyShootInterval = 2.0f;
        out->shootMode = SHOOT_STRAIGHT;
        out->homingTurnRate = 0.0f;
        out->rows = 4;
        out->cols = 11;
        out->hasUFO = 1;
        out->playerBulletSpeed = 440.0f;
        out->playerShootCooldown = 0.40f;
        out->playerSpeed = 230.0f;
        break;
    case 4:
        out->invaderSpeedBase = 70.0f;
        out->invaderSpeedMult = 1.9f;
        out->invaderDropStep = 18.0f;
        out->enemyBulletSpeed = 210.0f;
        out->enemyShootInterval = 1.8f;
        out->shootMode = SHOOT_AIMED;
        out->homingTurnRate = 0.0f;
        out->rows = 4;
        out->cols = 11;
        out->hasUFO = 1;
        out->playerBulletSpeed = 450.0f;
        out->playerShootCooldown = 0.38f;
        out->playerSpeed = 235.0f;
        break;
    case 5:
        out->invaderSpeedBase = 80.0f;
        out->invaderSpeedMult = 2.0f;
        out->invaderDropStep = 20.0f;
        out->enemyBulletSpeed = 230.0f;
        out->enemyShootInterval = 1.6f;
        out->shootMode = SHOOT_AIMED;
        out->homingTurnRate = 0.0f;
        out->rows = 5;
        out->cols = 11;
        out->hasUFO = 1;
        out->playerBulletSpeed = 460.0f;
        out->playerShootCooldown = 0.36f;
        out->playerSpeed = 240.0f;
        break;
    case 6:
        out->invaderSpeedBase = 90.0f;
        out->invaderSpeedMult = 2.1f;
        out->invaderDropStep = 20.0f;
        out->enemyBulletSpeed = 250.0f;
        out->enemyShootInterval = 1.4f;
        out->shootMode = SHOOT_AIMED;
        out->homingTurnRate = 0.0f;
        out->rows = 5;
        out->cols = 11;
        out->hasUFO = 1;
        out->playerBulletSpeed = 470.0f;
        out->playerShootCooldown = 0.34f;
        out->playerSpeed = 245.0f;
        break;
    case 7:
        out->invaderSpeedBase = 100.0f;
        out->invaderSpeedMult = 2.2f;
        out->invaderDropStep = 22.0f;
        out->enemyBulletSpeed = 260.0f;
        out->enemyShootInterval = 1.3f;
        out->shootMode = SHOOT_HOMING_SLOW;
        out->homingTurnRate = 0.6f;
        out->rows = 5;
        out->cols = 11;
        out->hasUFO = 1;
        out->playerBulletSpeed = 480.0f;
        out->playerShootCooldown = 0.32f;
        out->playerSpeed = 250.0f;
        break;
    case 8:
        out->invaderSpeedBase = 115.0f;
        out->invaderSpeedMult = 2.3f;
        out->invaderDropStep = 22.0f;
        out->enemyBulletSpeed = 275.0f;
        out->enemyShootInterval = 1.1f;
        out->shootMode = SHOOT_HOMING_SLOW;
        out->homingTurnRate = 0.9f;
        out->rows = 5;
        out->cols = 11;
        out->hasUFO = 1;
        out->playerBulletSpeed = 490.0f;
        out->playerShootCooldown = 0.30f;
        out->playerSpeed = 255.0f;
        break;
    case 9:
        out->invaderSpeedBase = 130.0f;
        out->invaderSpeedMult = 2.5f;
        out->invaderDropStep = 24.0f;
        out->enemyBulletSpeed = 295.0f;
        out->enemyShootInterval = 0.95f;
        out->shootMode = SHOOT_HOMING_SLOW;
        out->homingTurnRate = 1.2f;
        out->rows = 5;
        out->cols = 11;
        out->hasUFO = 1;
        out->playerBulletSpeed = 500.0f;
        out->playerShootCooldown = 0.28f;
        out->playerSpeed = 260.0f;
        break;
    case 10:
        out->invaderSpeedBase = 150.0;
        out->invaderSpeedMult = 2.8f;
        out->invaderDropStep = 24.0f;
        out->enemyBulletSpeed = 320.0f;
        out->enemyShootInterval = 0.80f;
        out->shootMode = SHOOT_HOMING_SLOW;
        out->homingTurnRate = 1.6f;
        out->rows = 5;
        out->cols = 11;
        out->hasUFO = 1;
        out->playerBulletSpeed = 510.0f;
        out->playerShootCooldown = 0.25f;
        out->playerSpeed = 265.0f;
        break;
    }
}