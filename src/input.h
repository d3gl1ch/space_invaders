#ifndef INPUT_H
#define INPUT_H

#include <windows.h>
#include "game.h"

// Опрос состояния клавиш (вызывается каждый кадр) 
void Input_Update(GameState *gs);

// Обработка одиночного нажатия WM_KEYDOWN 
void Input_OnKeyDown(GameState *gs, WPARAM vkCode);

#endif