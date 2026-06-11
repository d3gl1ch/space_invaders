#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "render.h"
#include "input.h"

// Модульные переменные 
static GameState g_state;
static RenderContext g_rc;
static BOOL g_rcInitialized = FALSE;
static BOOL g_running = TRUE;

// Оконная процедура
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_CREATE:
        return 0;

    case WM_KEYDOWN:
        Input_OnKeyDown(&g_state, wp);
        return 0;

    case WM_DESTROY:
        g_running = FALSE;
        PostQuitMessage(0);
        return 0;

    case WM_ERASEBKGND:
        // Подавляем стандартное стирание — двойная буферизация
        return 1;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if (g_rcInitialized) {
            Render_Frame(&g_rc, hdc, &g_state);
        }
        EndPaint(hwnd, &ps);
        return 0;
    }
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

// WinMain
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSA wc;
    HWND hwnd;
    RECT rect;
    HDC hdc;
    MSG msg;
    LARGE_INTEGER freq, prevTime, curTime;
    float accumulator;
    const float FIXED_DT = 1.0f / 60.0f;
    const float MAX_ACC = 0.25f;

    (void)hPrevInstance;
    (void)lpCmdLine;

    srand((unsigned int)time(NULL));

    // Регистрация класса окна
    ZeroMemory(&wc, sizeof(wc));
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorA(NULL, (LPCSTR)IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = "SpaceInvadersWnd";
    wc.hIcon = LoadIconA(NULL, (LPCSTR)IDI_APPLICATION);

    if (!RegisterClassA(&wc)) {
        MessageBoxA(NULL, "RegisterClass failed", "Error", MB_OK | MB_ICONERROR);
        return 1;
    } 

    // Вычисление размера окна с учётом рамки
    rect.left = 0;
    rect.top = 0;
    rect.right = WINDOW_WIDTH;
    rect.bottom = WINDOW_HEIGHT;
    AdjustWindowRect(&rect,
                     WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                     FALSE);

    // Создание окна
    hwnd = CreateWindowA(
        "SpaceInvadersWnd",
        "Space Invaders",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right  - rect.left,
        rect.bottom - rect.top,
        NULL, NULL, hInstance, NULL);

    if (!hwnd) {
        MessageBoxA(NULL, "CreateWindow failed", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Инициализация рендерера
    hdc = GetDC(hwnd);
    if (!hdc) {
        MessageBoxA(NULL, "GetDC failed", "Error", MB_OK | MB_ICONERROR);
        DestroyWindow(hwnd);
        return 1;
    }
    Render_Init(&g_rc, hdc, WINDOW_WIDTH, WINDOW_HEIGHT);
    ReleaseDC(hwnd, hdc);
    g_rcInitialized = TRUE;

    // Инициализация игровой логики
    Game_Init(&g_state);

    // Показ окна
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Таймер высокого разрешения
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&prevTime);
    accumulator = 0.0f;

    // Главный игровой цикл
    ZeroMemory(&msg, sizeof(msg));

    while (g_running) {
        // Обработка всех накопившихся сообщений
        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                g_running = FALSE;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        if (!g_running) break;

        // Delta time
        QueryPerformanceCounter(&curTime);
        {
            float dt = (float)(curTime.QuadPart - prevTime.QuadPart)
                     / (float)freq.QuadPart;
            if (dt > MAX_ACC) dt = MAX_ACC;
            accumulator += dt;
        }
        prevTime = curTime;

        // Опрос ввода 
        Input_Update(&g_state);

        // Фиксированный шаг логики 
        while (accumulator >= FIXED_DT) {
            Game_Update(&g_state, FIXED_DT);
            accumulator -= FIXED_DT;
        }

        // Рендер 
        hdc = GetDC(hwnd);
        if (hdc) {
            Render_Frame(&g_rc, hdc, &g_state);
            ReleaseDC(hwnd, hdc);
        }

        // Ограничение CPU: ждём до следующего кадра 
        {
            LARGE_INTEGER now;
            float elapsed;
            DWORD waitMs;
            QueryPerformanceCounter(&now);
            elapsed = (float)(now.QuadPart - prevTime.QuadPart)
                    / (float)freq.QuadPart;
            if (elapsed < FIXED_DT) {
                waitMs = (DWORD)((FIXED_DT - elapsed) * 900.0f);
                if (waitMs > 0)
                    MsgWaitForMultipleObjects(0, NULL, FALSE,
                                             waitMs, QS_ALLINPUT);
            }
        }
    }

    // Очистка 
    if (g_rcInitialized) {
        Render_Destroy(&g_rc);
        g_rcInitialized = FALSE;
    }
    DestroyWindow(hwnd);
    UnregisterClassA("SpaceInvadersWnd", hInstance);

    return (int)msg.wParam;
}