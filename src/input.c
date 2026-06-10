#include "input.h"

// Непрерывный ввод (движение, стрельба)
void Input_Update(GameState *gs)
{
    gs->keyLeft  = ((GetAsyncKeyState(VK_LEFT) & 0x8000) ||
                    (GetAsyncKeyState('A') & 0x8000)) ? 1 : 0;
    gs->keyRight = ((GetAsyncKeyState(VK_RIGHT) & 0x8000) ||
                    (GetAsyncKeyState('D') & 0x8000)) ? 1 : 0;
    gs->keyShoot = ((GetAsyncKeyState(VK_SPACE) & 0x8000) ||
                    (GetAsyncKeyState('W') & 0x8000)) ? 1 : 0;
}

// Одиночные нажатия
void Input_OnKeyDown(GameState *gs, WPARAM vkCode)
{
    switch ((int)vkCode) {

    case VK_RETURN:
        if (gs->screen == GS_MENU) {
            Game_InitLevel(gs);
        } else if (gs->screen == GS_GAME_OVER) {
            int saved = gs->hiScore;
            Game_Init(gs);
            gs->hiScore = saved;
            gs->screen = GS_MENU;
        } else if (gs->screen == GS_VICTORY) {
            int saved = gs->hiScore;
            Game_Init(gs);
            gs->hiScore = saved;
            gs->screen = GS_MENU;
        }
        break;

    case 'P':
    case VK_ESCAPE:
        if (gs->screen == GS_PLAYING) {
            gs->screen = GS_PAUSED;
        } else if (gs->screen == GS_PAUSED) {
            gs->screen = GS_PLAYING;
        } else if (gs->screen == GS_MENU) {
            PostQuitMessage(0);
        }
        break;

    default:
        break;
    }
}