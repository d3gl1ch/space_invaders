cmake_minimum_required(VERSION 3.10)
project(SpaceInvaders C)

set(CMAKE_C_STANDARD 11)

# Источники
set(SOURCES
    src/main.c
    src/game.c
    src/render.c
    src/input.c
    src/levels.c
)

# Исполняемый файл (WIN32 = оконное приложение без консоли)
add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

# Флаги компилятора
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall -Wextra -O2
)

# Библиотеки
target_link_libraries(${PROJECT_NAME}
    gdi32
    msimg32
    winmm
    m
)

# Заголовки
target_include_directories(${PROJECT_NAME} PRIVATE src)