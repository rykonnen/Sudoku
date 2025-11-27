#include "sudoku.h"
#include <string.h>
#include <stdlib.h>

u8 grid[CELLS] = {0};
u8 initial_grid[CELLS] = {0};
u8 solution_grid[CELLS] = {0};

int difficultyHoles = MEDIUM_HOLES;
int mistakes = 0;
const int maxMistakes = 3;
bool showSolution = false;
bool gameComplete = false;

bool is_valid(const u8 grid[CELLS], int row, int col, u8 val) {
    for (int i = 0; i < 9; i++) {
        if (grid[idx(row, i)] == val) return false;
        if (grid[idx(i, col)] == val) return false;
    }
    int br = (row / 3) * 3;
    int bc = (col / 3) * 3;
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 3; c++)
            if (grid[idx(br + r, bc + c)] == val) return false;
    return true;
}

bool find_empty(const u8 grid[CELLS], int *row, int *col) {
    for (int r = 0; r < 9; r++)
        for (int c = 0; c < 9; c++)
            if (grid[idx(r, c)] == 0) {
                *row = r;
                *col = c;
                return true;
            }
    return false;
}

void shuffle_u8(u8 *arr, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        u8 tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

bool generator_recursive(u8 grid[CELLS]) {
    int row, col;
    if (!find_empty(grid, &row, &col)) return true;
    u8 order[9];
    for (int i = 0; i < 9; i++) order[i] = i + 1;
    shuffle_u8(order, 9);
    for (int k = 0; k < 9; k++) {
        u8 v = order[k];
        if (is_valid(grid, row, col, v)) {
            grid[idx(row, col)] = v;
            if (generator_recursive(grid)) return true;
            grid[idx(row, col)] = 0;
        }
    }
    return false;
}

void make_puzzle(u8 current_grid[CELLS], u8 fixed_grid[CELLS], int holes) {
    memset(current_grid, 0, CELLS);
    memset(fixed_grid, 0, CELLS);

    u8 fullGrid[CELLS] = {0};
    generator_recursive(fullGrid);              // generate full solution
    memcpy(solution_grid, fullGrid, CELLS);    // store solution

    memcpy(current_grid, fullGrid, CELLS);     // start puzzle grid as full solution
    memcpy(fixed_grid, fullGrid, CELLS);       // fixed grid for static cells

    int removed = 0;
    while (removed < holes) {
        int pos = rand() % CELLS;
        if (current_grid[pos] != 0) {
            current_grid[pos] = 0;
            fixed_grid[pos] = 0;
            removed++;
        }
    }
}

bool is_complete() {
    for (int i = 0; i < CELLS; i++)
        if (grid[i] == 0) return false;
    return true;
}
