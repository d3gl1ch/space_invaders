#include "render.h"
#include "powerup.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

// Прототипы
static void drawStars(RenderContext *rc);
static void drawMenu(RenderContext *rc, const GameState *gs);
static void drawPlaying(RenderContext *rc, const GameState *gs);
static void drawHUD(RenderContext *rc, const GameState *gs);
static void drawEffectsBar(RenderContext *rc, const GameState *gs);
static void drawPaused(RenderContext *rc, const GameState *gs);
static void drawLevelWin(RenderContext *rc, const GameState *gs);
static void drawGameOver(RenderContext *rc, const GameState *gs);
static void drawVictory(RenderContext *rc, const GameState *gs);
static void drawPlayer(RenderContext *rc, const GameState *gs);
static void drawInvaders(RenderContext *rc, const GameState *gs);
static void drawBullets(RenderContext *rc, const GameState *gs);
static void drawBarriers(RenderContext *rc, const GameState *gs);
static void drawUFOSprite(RenderContext *rc, int x, int y);
static void drawPowerUps(RenderContext *rc, const GameState *gs);
static void drawPowerUpLegend(RenderContext *rc, const GameState *gs, int startY);
static void drawInvA(HDC dc, HBRUSH br, HPEN pn, int x, int y, int f);
static void drawInvB(HDC dc, HBRUSH br, HPEN pn, int x, int y, int f);
static void drawInvC(HDC dc, HBRUSH br, HPEN pn, int x, int y, int f);

// Примитивы
static void fillR(HDC dc, HBRUSH br, HPEN pn, int x, int y, int w, int h)
{
    HBRUSH ob = (HBRUSH)SelectObject(dc, br);
    HPEN op = (HPEN)SelectObject(dc, pn);
    Rectangle(dc, x, y, x + w, y + h);
    SelectObject(dc, ob);
    SelectObject(dc, op);
}

static void fillEllipseR(HDC dc, HBRUSH br, HPEN pn, int x, int y, int w, int h)
{
    HBRUSH ob = (HBRUSH)SelectObject(dc, br);
    HPEN op = (HPEN)SelectObject(dc, pn);
    Ellipse(dc, x, y, x + w, y + h);
    SelectObject(dc, ob);
    SelectObject(dc, op);
}

static void txtC(HDC dc, HFONT fnt, COLORREF col, int cx, int y, const char *s)
{
    SIZE sz;
    HFONT of = (HFONT)SelectObject(dc, fnt);
    SetTextColor(dc, col);
    SetBkMode(dc, TRANSPARENT);
    GetTextExtentPoint32A(dc, s, (int)strlen(s), &sz);
    TextOutA(dc, cx - sz.cx / 2, y, s, (int)strlen(s));
    SelectObject(dc, of);
}

static void txtL(HDC dc, HFONT fnt, COLORREF col, int x, int y, const char *s)
{
    HFONT of = (HFONT)SelectObject(dc, fnt);
    SetTextColor(dc, col);
    SetBkMode(dc, TRANSPARENT);
    TextOutA(dc, x, y, s, (int)strlen(s));
    SelectObject(dc, of);
}

static void txtR(HDC dc, HFONT fnt, COLORREF col, int rx, int y, const char *s)
{
    SIZE sz;
    HFONT of = (HFONT)SelectObject(dc, fnt);
    SetTextColor(dc, col);
    SetBkMode(dc, TRANSPARENT);
    GetTextExtentPoint32A(dc, s, (int)strlen(s), &sz);
    TextOutA(dc, rx - sz.cx, y, s, (int)strlen(s));
    SelectObject(dc, of);
}

static void drawDialog(HDC dc, HBRUSH br, HPEN pen,
                        int cx, int cy, int hw, int hh)
{
    HBRUSH ob = (HBRUSH)SelectObject(dc, br);
    HPEN op = (HPEN)  SelectObject(dc, pen);
    Rectangle(dc, cx - hw, cy - hh, cx + hw, cy + hh);
    SelectObject(dc, ob);
    SelectObject(dc, op);
}

// Цвета и метки для каждого типа усиления
static COLORREF puColors[PU_COUNT] = {0};
static const char *puLabels[PU_COUNT] = {
    "+1",   // PU_EXTRA_LIFE  
    "SH",   // PU_SHIELD      
    "SL",   // PU_SLOW        
    "FS",   // PU_FAST_SHOOT  
    "3X",   // PU_TRIPLE_SHOT 
    "BM"    // PU_BOMB        
};
static const char *puNames[PU_COUNT] = {
    "+1 LIFE (permanent)",
    "SHIELD  (10s)",
    "SLOW ENEMIES (8s)",
    "FAST FIRE (5s)",
    "TRIPLE SHOT (5s)",
    "BOMB SHOT (5s)"
};

// Инициализация
void Render_Init(RenderContext *rc, HDC windowDC, int width, int height)
{
    int i;
    memset(rc, 0, sizeof(RenderContext));
    rc->width = width;
    rc->height = height;

    rc->backDC = CreateCompatibleDC(windowDC);
    rc->backBmp = CreateCompatibleBitmap(windowDC, width, height);
    rc->oldBmp = (HBITMAP)SelectObject(rc->backDC, rc->backBmp);

    // Кисти 
    rc->brushBg = CreateSolidBrush(RGB(5, 5, 20));
    rc->brushPlayer = CreateSolidBrush(RGB(0, 230, 80));
    rc->brushEngine = CreateSolidBrush(RGB(220, 80, 80));
    rc->brushInvA = CreateSolidBrush(RGB(220, 80, 80));
    rc->brushInvB = CreateSolidBrush(RGB(220, 180, 50));
    rc->brushInvC = CreateSolidBrush(RGB(100, 180, 255));
    rc->brushUFO = CreateSolidBrush(RGB(255, 50, 220));
    rc->brushUFOWindow = CreateSolidBrush(RGB(255, 255, 200));
    rc->brushBulletPlayer = CreateSolidBrush(RGB(100, 255, 100));
    rc->brushBulletEnemy = CreateSolidBrush(RGB(255, 80, 50));
    rc->brushBarrier = CreateSolidBrush(RGB(0, 200, 80));
    rc->brushBarrierDmg1 = CreateSolidBrush(RGB(0, 140, 40));
    rc->brushBarrierDmg2 = CreateSolidBrush(RGB(0, 80, 20));
    rc->brushHUD = CreateSolidBrush(RGB(20, 20, 60));
    rc->brushOverlayDark = CreateSolidBrush(RGB(10, 0, 0));
    rc->brushOverlayWin = CreateSolidBrush(RGB(0, 20, 10));
    rc->brushPUBorder = CreateSolidBrush(RGB(255, 255, 255));

    // Power-up цвета 
    puColors[PU_EXTRA_LIFE] = RGB(255, 80, 80);   // красный   
    puColors[PU_SHIELD] = RGB(80, 180, 255);  // синий     
    puColors[PU_SLOW] = RGB(160, 80, 255);  // фиолет.   
    puColors[PU_FAST_SHOOT] = RGB(255, 220, 50);   // жёлтый    
    puColors[PU_TRIPLE_SHOT] = RGB(50, 255, 180);  // мятный    
    puColors[PU_BOMB] = RGB(255, 130, 30);   // оранжевый 

    for (i = 0; i < PU_COUNT; i++)
        rc->brushPU[i] = CreateSolidBrush(puColors[i]);

    // Перья 
    rc->penNull = CreatePen(PS_NULL,  0, 0);
    rc->penCyan = CreatePen(PS_SOLID, 1, RGB(0, 220, 255));
    rc->penGreen = CreatePen(PS_SOLID, 1, RGB(0, 230, 80));
    rc->penRed = CreatePen(PS_SOLID, 1, RGB(255, 50, 50));
    rc->penYellow = CreatePen(PS_SOLID, 1, RGB(255, 230, 50));
    rc->penWhite = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    rc->penBorderWin = CreatePen(PS_SOLID, 2, RGB(0, 230, 80));
    rc->penBorderLose = CreatePen(PS_SOLID, 2, RGB(255, 50, 50));
    rc->penBorderPause = CreatePen(PS_SOLID, 2, RGB(0, 220, 255));

    // Шрифты 
    rc->fontBig = CreateFontA(
        52, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY, FF_DONTCARE, "Courier New");
    rc->fontMed = CreateFontA(
        28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY, FF_DONTCARE, "Courier New");
    rc->fontSmall = CreateFontA(
        18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY, FF_DONTCARE, "Courier New");
    rc->fontTiny = CreateFontA(
        13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY, FF_DONTCARE, "Courier New");
}

// Уничтожение — освобождаем ВСЁ
void Render_Destroy(RenderContext *rc)
{
    int i;

    if (rc->backDC && rc->oldBmp) SelectObject(rc->backDC, rc->oldBmp);
    if (rc->backBmp) { DeleteObject(rc->backBmp); rc->backBmp = NULL; }
    if (rc->backDC) { DeleteDC(rc->backDC); rc->backDC = NULL; }

#define DEL(x) if (rc->x) { DeleteObject(rc->x); rc->x = NULL; }
    DEL(brushBg)
    DEL(brushPlayer)
    DEL(brushEngine)
    DEL(brushInvA)
    DEL(brushInvB)
    DEL(brushInvC)
    DEL(brushUFO)
    DEL(brushUFOWindow)
    DEL(brushBulletPlayer)
    DEL(brushBulletEnemy)
    DEL(brushBarrier)
    DEL(brushBarrierDmg1)
    DEL(brushBarrierDmg2)
    DEL(brushHUD)
    DEL(brushOverlayDark)
    DEL(brushOverlayWin)
    DEL(brushPUBorder)
    DEL(penNull)
    DEL(penCyan)
    DEL(penGreen)
    DEL(penRed)
    DEL(penYellow)
    DEL(penWhite)
    DEL(penBorderWin)
    DEL(penBorderLose)
    DEL(penBorderPause)
    DEL(fontBig)
    DEL(fontMed)
    DEL(fontSmall)
    DEL(fontTiny)
#undef DEL

    for (i = 0; i < PU_COUNT; i++) {
        if (rc->brushPU[i]) { DeleteObject(rc->brushPU[i]); rc->brushPU[i] = NULL; }
    }
}

// Главный рендер
void Render_Frame(RenderContext *rc, HDC windowDC, const GameState *gs)
{
    HDC  dc = rc->backDC;
    RECT bg;
    bg.left = 0; bg.top = 0;
    bg.right = rc->width; bg.bottom = rc->height;
    FillRect(dc, &bg, rc->brushBg);

    drawStars(rc);

    switch (gs->screen) {
    case GS_MENU:
        drawMenu(rc, gs);
        break;
    case GS_PLAYING:
        drawPlaying(rc, gs);
        drawHUD(rc, gs);
        drawEffectsBar(rc, gs);
        break;
    case GS_PAUSED:
        drawPlaying(rc, gs);
        drawHUD(rc, gs);
        drawEffectsBar(rc, gs);
        drawPaused(rc, gs);
        break;
    case GS_LEVEL_WIN:
        drawPlaying(rc, gs);
        drawHUD(rc, gs);
        drawLevelWin(rc, gs);
        break;
    case GS_GAME_OVER:
        drawPlaying(rc, gs);
        drawHUD(rc, gs);
        drawGameOver(rc, gs);
        break;
    case GS_VICTORY:
        drawVictory(rc, gs);
        break;
    }

    BitBlt(windowDC, 0, 0, rc->width, rc->height, dc, 0, 0, SRCCOPY);
}

// Звёзды
static void drawStars(RenderContext *rc)
{
    unsigned int seed = 12345u;
    int i;
    HDC dc = rc->backDC;
    for (i = 0; i < 120; i++) {
        int sx, sy, bright;
        seed = seed * 1664525u + 1013904223u;
        sx = (int)(seed % (unsigned int)rc->width);
        seed = seed * 1664525u + 1013904223u;
        sy = (int)(seed % (unsigned int)rc->height);
        seed = seed * 1664525u + 1013904223u;
        bright = 80 + (int)(seed % 176u);
        SetPixel(dc, sx, sy, RGB(bright, bright, bright));
    }
}

// UFO спрайт
static void drawUFOSprite(RenderContext *rc, int x, int y)
{
    HDC dc = rc->backDC;
    fillR(dc, rc->brushUFO, rc->penNull, x + 8,  y + 8, 32, 12);
    fillR(dc, rc->brushUFO, rc->penNull, x + 4,  y + 4, 40,  8);
    fillR(dc, rc->brushUFO, rc->penNull, x + 14, y, 20,  6);
    fillR(dc, rc->brushUFOWindow, rc->penNull, x + 10, y + 10, 6,  6);
    fillR(dc, rc->brushUFOWindow, rc->penNull, x + 22, y + 10, 6,  6);
    fillR(dc, rc->brushUFOWindow, rc->penNull, x + 34, y + 10, 6,  6);
}

// Спрайты пришельцев
static void drawInvA(HDC dc, HBRUSH br, HPEN pn, int x, int y, int f)
{
    fillR(dc, br, pn, x + 6,  y + 4,  24, 14);
    if (f == 0) {
        fillR(dc, br, pn, x, y + 10, 6, 6);
        fillR(dc, br, pn, x + 30, y + 10, 6, 6);
        fillR(dc, br, pn, x + 2, y + 16, 4, 4);
        fillR(dc, br, pn, x + 30, y + 16, 4, 4);
    } else {
        fillR(dc, br, pn, x + 2, y + 8,  4, 8);
        fillR(dc, br, pn, x + 30, y + 8,  4, 8);
        fillR(dc, br, pn, x, y + 14, 6, 4);
        fillR(dc, br, pn, x + 30, y + 14, 6, 4);
    }
    fillR(dc, br, pn, x + 8, y, 4, 4);
    fillR(dc, br, pn, x + 24, y, 4, 4);
    fillR(dc, br, pn, x + 8, y + 18, 4, 6);
    fillR(dc, br, pn, x + 16, y + 18, 4, 6);
    fillR(dc, br, pn, x + 24, y + 18, 4, 6);
}

static void drawInvB(HDC dc, HBRUSH br, HPEN pn, int x, int y, int f)
{
    fillR(dc, br, pn, x + 4, y + 2, 28, 16);
    fillR(dc, br, pn, x + 2, y + 6, 32,  8);
    if (f == 0) {
        fillR(dc, br, pn, x, y + 12, 6, 6);
        fillR(dc, br, pn, x + 30, y + 12, 6, 6);
        fillR(dc, br, pn, x + 8, y + 18, 4, 6);
        fillR(dc, br, pn, x + 24, y + 18, 4, 6);
    } else {
        fillR(dc, br, pn, x + 2, y + 14, 6, 4);
        fillR(dc, br, pn, x + 28, y + 14, 6, 4);
        fillR(dc, br, pn, x + 6, y + 18, 6, 6);
        fillR(dc, br, pn, x + 24, y + 18, 6, 6);
    }
    fillR(dc, br, pn, x + 10, y, 16, 4);
}

static void drawInvC(HDC dc, HBRUSH br, HPEN pn, int x, int y, int f)
{
    fillR(dc, br, pn, x + 10, y + 4, 16, 12);
    if (f == 0) {
        fillR(dc, br, pn, x, y + 2, 10, 8);
        fillR(dc, br, pn, x + 26, y + 2,  10, 8);
        fillR(dc, br, pn, x + 2, y + 10, 6, 4);
        fillR(dc, br, pn, x + 28, y + 10, 6, 4);
    } else {
        fillR(dc, br, pn, x + 2, y + 4, 8, 10);
        fillR(dc, br, pn, x + 26, y + 4, 8, 10);
        fillR(dc, br, pn, x, y + 10, 4, 6);
        fillR(dc, br, pn, x + 32, y + 10, 4, 6);
    }
    fillR(dc, br, pn, x + 12, y, 12, 4);
    fillR(dc, br, pn, x + 14, y + 16, 8, 6);
}

// Падающий шар усиления
static void drawPowerUps(RenderContext *rc, const GameState *gs)
{
    HDC dc = rc->backDC;
    int i;

    for (i = 0; i < MAX_POWERUPS; i++) {
        const PowerUp *pu = &gs->powerups[i];
        if (!pu->active) continue;

        int t  = (int)pu->type;
        int px = (int)pu->x;
        int py = (int)pu->y;

        // Внешний круг (белый контур) 
        fillEllipseR(dc, rc->brushPUBorder, rc->penNull,
                     px - 1, py - 1, PU_W + 2, PU_H + 2);

        // Цветной круг 
        fillEllipseR(dc, rc->brushPU[t], rc->penNull,
                     px, py, PU_W, PU_H);

        // Текстовая метка по центру шара 
        txtC(dc, rc->fontTiny, RGB(0, 0, 0),
             px + PU_W / 2,
             py + PU_H / 2 - 6,
             puLabels[t]);
    }
}

// Полоса активных эффектов (под HUD)
static void drawEffectsBar(RenderContext *rc, const GameState *gs)
{
    HDC dc  = rc->backDC;
    const ActiveEffects *e = &gs->effects;
    int x = rc->width - 10;  // правый край 
    int y = FIELD_TOP + 18;
    int gap = 28;
    char buf[16];

    // Рисуем активные иконки справа налево 
    // Каждая иконка: цветной кружок + таймер 

    if (e->shieldTimer > 0.0f) {
        x -= PU_W;
        fillEllipseR(dc, rc->brushPU[PU_SHIELD], rc->penNull, x, y, PU_W, PU_H);
        txtC(dc, rc->fontTiny, RGB(0,0,0), x + PU_W/2, y + PU_H/2 - 6, "SH");
        sprintf(buf, "%.0fs", e->shieldTimer);
        txtC(dc, rc->fontTiny, RGB(80, 180, 255), x + PU_W/2, y + PU_H + 1, buf);
        x -= gap;
    }
    if (e->slowTimer > 0.0f) {
        x -= PU_W;
        fillEllipseR(dc, rc->brushPU[PU_SLOW], rc->penNull, x, y, PU_W, PU_H);
        txtC(dc, rc->fontTiny, RGB(0,0,0), x + PU_W/2, y + PU_H/2 - 6, "SL");
        sprintf(buf, "%.0fs", e->slowTimer);
        txtC(dc, rc->fontTiny, RGB(160, 80, 255), x + PU_W/2, y + PU_H + 1, buf);
        x -= gap;
    }
    if (e->fastShootTimer > 0.0f) {
        x -= PU_W;
        fillEllipseR(dc, rc->brushPU[PU_FAST_SHOOT], rc->penNull, x, y, PU_W, PU_H);
        txtC(dc, rc->fontTiny, RGB(0,0,0), x + PU_W/2, y + PU_H/2 - 6, "FS");
        sprintf(buf, "%.0fs", e->fastShootTimer);
        txtC(dc, rc->fontTiny, RGB(255, 220, 50), x + PU_W/2, y + PU_H + 1, buf);
        x -= gap;
    }
    if (e->tripleTimer > 0.0f) {
        x -= PU_W;
        fillEllipseR(dc, rc->brushPU[PU_TRIPLE_SHOT], rc->penNull, x, y, PU_W, PU_H);
        txtC(dc, rc->fontTiny, RGB(0,0,0), x + PU_W/2, y + PU_H/2 - 6, "3X");
        sprintf(buf, "%.0fs", e->tripleTimer);
        txtC(dc, rc->fontTiny, RGB(50, 255, 180), x + PU_W/2, y + PU_H + 1, buf);
        x -= gap;
    }
    if (e->bombTimer > 0.0f) {
        x -= PU_W;
        fillEllipseR(dc, rc->brushPU[PU_BOMB], rc->penNull, x, y, PU_W, PU_H);
        txtC(dc, rc->fontTiny, RGB(0,0,0), x + PU_W/2, y + PU_H/2 - 6, "BM");
        sprintf(buf, "%.0fs", e->bombTimer);
        txtC(dc, rc->fontTiny, RGB(255, 130, 30), x + PU_W/2, y + PU_H + 1, buf);
        x -= gap;
    }
    (void)x;
}

// Легенда power-up (для меню)
static void drawPowerUpLegend(RenderContext *rc, const GameState *gs, int startY)
{
    HDC dc = rc->backDC;
    int cx = rc->width / 2;
    int i;
    int y = startY;
    (void)gs;

    txtC(dc, rc->fontSmall, RGB(200, 200, 200), cx, y, "= POWER-UPS (catch the falling orb!) =");
    y += 24;

    for (i = 0; i < PU_COUNT; i++) {
        int ox = cx - 140;
        // Кружок 
        fillEllipseR(dc, rc->brushPU[i], rc->penNull, ox, y, PU_W, PU_H);
        txtC(dc, rc->fontTiny, RGB(0,0,0), ox + PU_W/2, y + PU_H/2 - 6, puLabels[i]);
        // Описание 
        txtL(dc, rc->fontSmall, puColors[i], ox + PU_W + 8, y + 4, puNames[i]);
        y += PU_H + 4;
    }
}

// Игровая сцена
static void drawPlaying(RenderContext *rc, const GameState *gs)
{
    drawBarriers(rc, gs);
    drawInvaders(rc, gs);
    if (gs->ufo.active)
        drawUFOSprite(rc, (int)gs->ufo.x, (int)gs->ufo.y);
    drawPowerUps(rc, gs);
    drawBullets(rc, gs);
    drawPlayer(rc, gs);
}

// Пришельцы
static void drawInvaders(RenderContext *rc, const GameState *gs)
{
    HDC dc = rc->backDC;
    int i;
    for (i = 0; i < MAX_INVADERS; i++) {
        const Invader *inv = &gs->invaders[i];
        if (!inv->alive) continue;
        switch (inv->type) {
        case INV_TYPE_A:
            drawInvA(dc, rc->brushInvA, rc->penNull,
                     (int)inv->x, (int)inv->y, gs->animFrame);
            break;
        case INV_TYPE_B:
            drawInvB(dc, rc->brushInvB, rc->penNull,
                     (int)inv->x, (int)inv->y, gs->animFrame);
            break;
        case INV_TYPE_C:
            drawInvC(dc, rc->brushInvC, rc->penNull,
                     (int)inv->x, (int)inv->y, gs->animFrame);
            break;
        case INV_TYPE_UFO:
            break;
        }
    }
}

// Игрок — с подсветкой щита
static void drawPlayer(RenderContext *rc, const GameState *gs)
{
    HDC dc = rc->backDC;
    int x = (int)gs->player.x;
    int y = (int)gs->player.y;

    // Щит — голубое кольцо вокруг корабля 
    if (gs->effects.shieldTimer > 0.0f) {
        // Мигание когда почти кончается (<3c) 
        int drawShield = 1;
        if (gs->effects.shieldTimer < 3.0f) {
            drawShield = (int)(gs->effects.shieldTimer * 6.0f) % 2;
        }
        if (drawShield) {
            HPEN op = (HPEN)SelectObject(dc, rc->penCyan);
            HBRUSH ob = (HBRUSH)SelectObject(dc, (HBRUSH)GetStockObject(NULL_BRUSH));
            Ellipse(dc, x - 6, y - 6, x + PLAYER_W + 6, y + PLAYER_H + 6);
            SelectObject(dc, op);
            SelectObject(dc, ob);
        }
        // При щите корабль не мигает 
        goto draw_ship;
    }

    // Обычное мигание неуязвимости 
    if (gs->player.invincible) {
        int blink = (int)(gs->player.invincTimer * 8.0f) % 2;
        if (blink) return;
    }

draw_ship:
    fillR(dc, rc->brushPlayer, rc->penNull, x + 2, y + 8, 36, 14);
    fillR(dc, rc->brushPlayer, rc->penNull, x + 12, y + 2, 16, 8);
    fillR(dc, rc->brushPlayer, rc->penNull, x, y + 14, 8, 8);
    fillR(dc, rc->brushPlayer, rc->penNull, x + 32, y + 14, 8, 8);
    fillR(dc, rc->brushPlayer, rc->penNull, x + 18, y, 4, 4);
    fillR(dc, rc->brushEngine, rc->penNull, x + 10, y + 20, 6, 4);
    fillR(dc, rc->brushEngine, rc->penNull, x + 24, y + 20, 6, 4);
}

// Пули
static void drawBullets(RenderContext *rc, const GameState *gs)
{
    HDC dc = rc->backDC;
    int i;

    for (i = 0; i < MAX_PLAYER_BULLETS; i++) {
        if (!gs->playerBullets[i].active) continue;
        // Если активен BOMB — пули рисуем оранжевыми 
        HBRUSH br = (gs->effects.bombTimer > 0.0f)
                    ? rc->brushPU[PU_BOMB]
                    : rc->brushBulletPlayer;
        fillR(dc, br, rc->penNull,
              (int)gs->playerBullets[i].x,
              (int)gs->playerBullets[i].y,
              BULLET_W, BULLET_H);
    }

    for (i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!gs->enemyBullets[i].active) continue;
        if (gs->lvl.shootMode == SHOOT_HOMING_SLOW) {
            int bx = (int)gs->enemyBullets[i].x;
            int by = (int)gs->enemyBullets[i].y;
            HBRUSH ob = (HBRUSH)SelectObject(dc, rc->brushBulletEnemy);
            HPEN op = (HPEN)SelectObject(dc, rc->penNull);
            POINT  pts[4];
            pts[0].x = bx + BULLET_W/2; 
            pts[0].y = by;
            pts[1].x = bx + BULLET_W; 
            pts[1].y = by + BULLET_H/2;
            pts[2].x = bx + BULLET_W/2; 
            pts[2].y = by + BULLET_H;
            pts[3].x = bx; 
            pts[3].y = by + BULLET_H/2;
            Polygon(dc, pts, 4);
            SelectObject(dc, ob);
            SelectObject(dc, op);
        } else {
            fillR(dc, rc->brushBulletEnemy, rc->penNull,
                  (int)gs->enemyBullets[i].x,
                  (int)gs->enemyBullets[i].y,
                  BULLET_W, BULLET_H);
        }
    }
}

// Барьеры
static void drawBarriers(RenderContext *rc, const GameState *gs)
{
    HDC dc = rc->backDC;
    int b, bi;
    HBRUSH br;
    for (b = 0; b < MAX_BARRIERS; b++) {
        for (bi = 0; bi < BARRIER_BLOCKS; bi++) {
            int hp = gs->barriers[b][bi].health;
            if (hp <= 0) continue;
            if (hp == 3) br = rc->brushBarrier;
            else if (hp == 2) br = rc->brushBarrierDmg1;
            else br = rc->brushBarrierDmg2;
            fillR(dc, br, rc->penNull,
                  gs->barriers[b][bi].x, gs->barriers[b][bi].y,
                  BLOCK_W, BLOCK_H);
        }
    }
}

// HUD
static void drawHUD(RenderContext *rc, const GameState *gs)
{
    HDC dc = rc->backDC;
    char buf[64];
    HPEN op;
    int i;

    fillR(dc, rc->brushHUD, rc->penNull, 0, 0, rc->width, FIELD_TOP - 4);

    op = (HPEN)SelectObject(dc, rc->penCyan);
    MoveToEx(dc, 0, FIELD_TOP - 4, NULL);
    LineTo(dc, rc->width, FIELD_TOP - 4);
    SelectObject(dc, op);

    sprintf(buf, "SCORE: %06d", gs->score);
    txtL(dc, rc->fontMed, RGB(0, 230, 255), 10, 10, buf);

    sprintf(buf, "HI: %06d", gs->hiScore);
    txtC(dc, rc->fontMed, RGB(255, 220, 50), rc->width / 2, 10, buf);

    sprintf(buf, "LVL: %02d/10", gs->currentLevel);
    txtR(dc, rc->fontMed, RGB(255, 120, 50), rc->width - 10, 10, buf);

    for (i = 0; i < gs->player.lives && i < 8; i++) {
        int lx = 10 + i * 28;
        int ly = FIELD_TOP - 22;
        fillR(dc, rc->brushPlayer, rc->penNull, lx + 2, ly + 6, 20, 10);
        fillR(dc, rc->brushPlayer, rc->penNull, lx + 8, ly + 2, 8, 6);
        fillR(dc, rc->brushPlayer, rc->penNull, lx, ly + 10, 5, 6);
        fillR(dc, rc->brushPlayer, rc->penNull, lx + 19, ly + 10, 5, 6);
        fillR(dc, rc->brushPlayer, rc->penNull, lx + 10, ly, 3, 3);
    }
        // Если жизней > 8 показываем число 
    if (gs->player.lives > 8) {
        char lb[32];
        snprintf(lb, sizeof(lb), "x%d", gs->player.lives);
        txtL(dc, rc->fontSmall, RGB(0, 230, 80), 10 + 8 * 28, FIELD_TOP - 20, lb);
    }

    op = (HPEN)SelectObject(dc, rc->penGreen);
    MoveToEx(dc, 0, rc->height - 40, NULL);
    LineTo(dc, rc->width, rc->height - 40);
    SelectObject(dc, op);

    {
        const char *ms = "";
        switch (gs->lvl.shootMode) {
        case SHOOT_STRAIGHT: ms = "[ STRAIGHT FIRE ]"; break;
        case SHOOT_AIMED: ms = "[ AIMED FIRE ]"; break;
        case SHOOT_HOMING_SLOW: ms = "[ HOMING FIRE ]"; break;
        }
        txtC(dc, rc->fontSmall, RGB(180, 100, 255),
             rc->width / 2, FIELD_TOP + 2, ms);
    }
}

// Меню — со списком power-up
static void drawMenu(RenderContext *rc, const GameState *gs)
{
    HDC dc = rc->backDC;
    int cx = rc->width / 2;
    char buf[64];
    HPEN op;
    int ty;
    static int blinkTick = 0;
    static int blink = 0;

    // Заголовок 
    txtC(dc, rc->fontBig, RGB(0,  60, 120), cx + 2, 52, "SPACE INVADERS");
    txtC(dc, rc->fontBig, RGB(0, 180, 255), cx, 50, "SPACE INVADERS");

    op = (HPEN)SelectObject(dc, rc->penCyan);
    MoveToEx(dc, cx - 200, 110, NULL);
    LineTo(dc, cx + 200, 110);
    SelectObject(dc, op);

    // Таблица очков — компактно 
    ty = 118;
    txtC(dc, rc->fontSmall, RGB(200, 200, 200), cx, ty, "SCORE TABLE:");
    ty += 20;

    drawInvC(dc, rc->brushInvC, rc->penNull, cx - 160, ty, 0);
    txtL(dc, rc->fontSmall, RGB(100,180,255), cx - 115, ty + 5, "= 30");
    drawInvB(dc, rc->brushInvB, rc->penNull, cx - 60,  ty, 0);
    txtL(dc, rc->fontSmall, RGB(220,180, 50), cx - 15,  ty + 5, "= 20");
    drawInvA(dc, rc->brushInvA, rc->penNull, cx + 40,  ty, 0);
    txtL(dc, rc->fontSmall, RGB(220, 80, 80), cx + 85,  ty + 5, "= 10");
    drawUFOSprite(rc, cx + 120, ty);
    txtL(dc, rc->fontSmall, RGB(255,50,220), cx + 170, ty + 5, "=???");

    ty += 36;
    op = (HPEN)SelectObject(dc, rc->penCyan);
    MoveToEx(dc, cx - 200, ty, NULL);
    LineTo(dc, cx + 200, ty);
    SelectObject(dc, op);

    // Power-up легенда 
    ty += 6;
    drawPowerUpLegend(rc, gs, ty);

    // Отступ после легенды: PU_COUNT строк по (PU_H+4) + заголовок 24 
    ty += 24 + PU_COUNT * (PU_H + 4) + 4;

    op = (HPEN)SelectObject(dc, rc->penCyan);
    MoveToEx(dc, cx - 200, ty, NULL);
    LineTo(dc, cx + 200, ty);
    SelectObject(dc, op);

    // Управление 
    ty += 6;
    txtC(dc, rc->fontSmall, RGB(150,150,150), cx, ty, "ARROWS/AD - Move   SPACE/W - Shoot   P/ESC - Pause");
    ty += 22;

    // Хай-скор 
    sprintf(buf, "HI-SCORE: %06d", gs->hiScore);
    txtC(dc, rc->fontMed, RGB(255, 220, 50), cx, ty, buf);
    ty += 36;

    // ENTER 
    blinkTick++;
    if (blinkTick > 30) { blink = !blink; blinkTick = 0; }
    if (blink)
        txtC(dc, rc->fontMed, RGB(0, 255, 120), cx, ty, ">> PRESS ENTER TO START <<");
}

// Пауза
static void drawPaused(RenderContext *rc, const GameState *gs)
{
    HDC dc = rc->backDC;
    int cx = rc->width  / 2;
    int cy = rc->height / 2;
    (void)gs;
    drawDialog(dc, rc->brushHUD, rc->penBorderPause, cx, cy, 180, 70);
    txtC(dc, rc->fontBig, RGB(255, 220,  50), cx, cy - 50, "PAUSED");
    txtC(dc, rc->fontSmall, RGB(200, 200, 200), cx, cy + 10, "P / ESC  to continue");
}

// Level Win
static void drawLevelWin(RenderContext *rc, const GameState *gs)
{
    HDC dc = rc->backDC;
    int cx = rc->width  / 2;
    int cy = rc->height / 2;
    char buf[64];
    drawDialog(dc, rc->brushOverlayWin, rc->penBorderWin, cx, cy, 220, 80);
    txtC(dc, rc->fontBig,   RGB(0,   255, 120), cx, cy - 60, "LEVEL CLEAR!");
    sprintf(buf, "NEXT: LEVEL %d", gs->currentLevel + 1);
    txtC(dc, rc->fontMed,   RGB(255, 220,  50), cx, cy - 10, buf);
    sprintf(buf, "SCORE: %06d", gs->score);
    txtC(dc, rc->fontMed,   RGB(0,   220, 255), cx, cy + 28, buf);
    sprintf(buf, "Starting in %.0f...", gs->transitionTimer + 0.9f);
    txtC(dc, rc->fontSmall, RGB(200, 200, 200), cx, cy + 58, buf);
}

// Game Over
static void drawGameOver(RenderContext *rc, const GameState *gs)
{
    HDC dc = rc->backDC;
    int cx = rc->width  / 2;
    int cy = rc->height / 2;
    char buf[64];
    drawDialog(dc, rc->brushOverlayDark, rc->penBorderLose, cx, cy, 220, 90);
    txtC(dc, rc->fontBig,   RGB(255,  50,  50), cx, cy - 70, "GAME OVER");
    sprintf(buf, "SCORE: %06d", gs->score);
    txtC(dc, rc->fontMed,   RGB(255, 220,  50), cx, cy - 10, buf);
    sprintf(buf, "HI: %06d", gs->hiScore);
    txtC(dc, rc->fontMed,   RGB(0,   220, 255), cx, cy + 26, buf);
    txtC(dc, rc->fontSmall, RGB(200, 200, 200), cx, cy + 62, "ENTER - Play Again");
}

// Победа
static void drawVictory(RenderContext *rc, const GameState *gs)
{
    HDC dc  = rc->backDC;
    int cx  = rc->width / 2;
    char buf[64];
    static unsigned int seed = 77777u;
    static const COLORREF pal[5] = {
        RGB(255,200,50), RGB(0,255,120),
        RGB(100,180,255), RGB(255,80,220), RGB(255,255,255)
    };
    int i;

    txtC(dc, rc->fontBig, RGB(255, 220, 0), cx, 120, "CONGRATULATIONS!");
    txtC(dc, rc->fontBig, RGB(0, 255,120), cx, 190, "YOU WIN!");
    txtC(dc, rc->fontMed, RGB(200, 200, 200), cx, 270,
         "Earth is saved from the alien invasion!");
    sprintf(buf, "FINAL SCORE: %06d", gs->score);
    txtC(dc, rc->fontBig, RGB(0, 220, 255), cx, 340, buf);
    sprintf(buf, "HI-SCORE: %06d", gs->hiScore);
    txtC(dc, rc->fontMed, RGB(255,220, 50), cx, 400, buf);

    for (i = 0; i < 60; i++) {
        int sx, sy;
        seed = seed * 1664525u + 1013904223u; sx = (int)(seed % (unsigned int)rc->width);
        seed = seed * 1664525u + 1013904223u; sy = 460 + (int)(seed % 160u);
        seed = seed * 1664525u + 1013904223u;
        SetPixel(dc, sx, sy, pal[seed % 5]);
    }

    txtC(dc, rc->fontMed, RGB(255,100,100), cx, 490, "ENTER - Main Menu");
}