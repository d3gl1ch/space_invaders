#ifndef RENDER_H
#define RENDER_H

#include <windows.h>
#include "game.h"

typedef struct {
    HDC backDC;
    HBITMAP backBmp;
    HBITMAP oldBmp;
    int width;
    int height;

    // Кисти 
    HBRUSH  brushBg;
    HBRUSH  brushPlayer;
    HBRUSH  brushEngine;
    HBRUSH  brushInvA;
    HBRUSH  brushInvB;
    HBRUSH  brushInvC;
    HBRUSH  brushUFO;
    HBRUSH  brushUFOWindow;
    HBRUSH  brushBulletPlayer;
    HBRUSH  brushBulletEnemy;
    HBRUSH  brushBarrier;
    HBRUSH  brushBarrierDmg1;
    HBRUSH  brushBarrierDmg2;
    HBRUSH  brushHUD;
    HBRUSH  brushOverlayDark;
    HBRUSH  brushOverlayWin;
    // Power-up цвета шаров (по одной на тип) 
    HBRUSH  brushPU[PU_COUNT];   // индекс = PowerUpType 
    HBRUSH  brushPUBorder;

    // Перья 
    HPEN  penNull;
    HPEN  penCyan;
    HPEN  penGreen;
    HPEN  penRed;
    HPEN  penYellow;
    HPEN  penWhite;
    HPEN  penBorderWin;
    HPEN  penBorderLose;
    HPEN  penBorderPause;

    // Шрифты 
    HFONT fontBig;
    HFONT fontMed;
    HFONT fontSmall;
    HFONT fontTiny;   // для иконки на шаре 
} RenderContext;

void Render_Init(RenderContext *rc, HDC windowDC, int width, int height);
void Render_Destroy(RenderContext *rc);
void Render_Frame(RenderContext *rc, HDC windowDC, const GameState *gs);

#endif 
