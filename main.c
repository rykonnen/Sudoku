#define CLAY_IMPLEMENTATION
#include "raylib.h"
#include "clay.h"
#include "gui.h"
#include <stdlib.h>
#include <stdio.h>

int main(void) {
    InitWindow(600, 600, "SUDOKU WASM");
    SetTargetFPS(60);

    uint64_t memorySize = Clay_MinMemorySize();
    void *memory = malloc(memorySize);
    Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(memorySize, memory);
    Clay_Initialize(arena, (Clay_Dimensions){600, 600}, (Clay_ErrorHandler){NULL});

    render_game_loop(arena, 20);

    free(memory);
    CloseWindow();
    return 0;
}
