#include "gui.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

GameState gameState = GAME_MENU;
int activeCellIndex = -1;
int hoveredDifficulty = -1;

Clay_Color STATIC_CELL_COLOR   = {236, 255, 255, 255};
Clay_Color EDITABLE_CELL_COLOR = {255, 255, 255, 255};
Clay_Color HOVER_COLOR         = {182, 214, 255, 255};
Clay_Color ACTIVE_COLOR        = {109, 202, 209, 255};
Color GIVEN_NUM_COLOR          = {0, 0, 0, 255};
Color USER_NUM_COLOR           = {44, 53, 54, 255};

NumberButton numberButtons[NUM_COUNT];

bool flashError = false;
int flashFrames = 0;
const int FLASH_DURATION = 15;



//start clay layouts
Clay_RenderCommandArray CreateGridLayout(void) {
    Clay_BeginLayout();
    int cellSize = 30;
    int gap = 0;
    int padding = 10;

    CLAY(CLAY_ID("GridWrapper"), {
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childAlignment = {.x = CLAY_ALIGN_X_LEFT, .y = CLAY_ALIGN_Y_CENTER},
            .sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)}
        }
    }) {
        CLAY(CLAY_ID("GridContainer"), {
            .layout = {
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .childGap = gap,
                .padding = CLAY_PADDING_ALL(padding),
            },
            .cornerRadius = CLAY_CORNER_RADIUS(4),
        }) {
            for (int r = 0; r < 9; r++) {
                CLAY_AUTO_ID({
                    .layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT, .childGap = gap}
                }) {
                    for (int c = 0; c < 9; c++) {
                        int cellIndex = idx(r, c);
                        Clay_Color bg = (initial_grid[cellIndex] != 0) ? STATIC_CELL_COLOR : EDITABLE_CELL_COLOR;
                        CLAY_AUTO_ID({
                            .layout = {
                                .sizing = {.width = CLAY_SIZING_FIXED(cellSize),
                                           .height = CLAY_SIZING_FIXED(cellSize)}
                            },
                            .backgroundColor = bg,
                        }) {}
                    }
                }
            }
        }
    }
    return Clay_EndLayout();
}



Clay_RenderCommandArray CreateMenuLayout(void) {
    Clay_BeginLayout();
    CLAY(CLAY_ID("MenuWrapper"), {
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER},
            .childGap = 1,
            .padding = CLAY_PADDING_ALL(0),
            .sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)}
        }
    }) {
        CLAY(CLAY_ID("Title"), {.layout = {.sizing = {.width = CLAY_SIZING_FIT(0), .height = CLAY_SIZING_FIT(0)}}}) {}
        CLAY(CLAY_ID("EasyButton"), 
        {.layout = 
            {.sizing = 
                {.width = CLAY_SIZING_FIXED(110), .height = CLAY_SIZING_FIXED(50)}
            }, .backgroundColor = (Clay_Color){255,255,255,255}, .cornerRadius = CLAY_CORNER_RADIUS(8)
        }) {}
        CLAY(CLAY_ID("MedButton"),  
        {.layout = 
            {.sizing = 
                {.width = CLAY_SIZING_FIXED(110), .height = CLAY_SIZING_FIXED(50)}
            }, .backgroundColor = (Clay_Color){255,255,255,255}, .cornerRadius = CLAY_CORNER_RADIUS(8)
        }) {}
        CLAY(CLAY_ID("HardButton"), 
        {.layout = 
            {.sizing = 
                {.width = CLAY_SIZING_FIXED(110), .height = CLAY_SIZING_FIXED(50)}
            }, .backgroundColor = (Clay_Color){255,255,255,255}, .cornerRadius = CLAY_CORNER_RADIUS(8)
        }) {}
    }

    return Clay_EndLayout();
}

// Back button at top-left
Clay_RenderCommandArray CreateBackButton(void) {
    Clay_BeginLayout();

    CLAY(CLAY_ID("BackWrapper"), {
        .layout = {
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
            .childAlignment = {CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_TOP},
            .padding = CLAY_PADDING_ALL(5),
            .sizing = {.width =  CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)}
        }
    }) {
        CLAY(CLAY_ID("Back"), {
            .layout = {.sizing = {.width = CLAY_SIZING_FIXED(150), .height = CLAY_SIZING_FIXED(30)}},
            .backgroundColor = (Clay_Color){255, 255, 255, 255},
            .cornerRadius = CLAY_CORNER_RADIUS(8)
        }) {}
    }

    return Clay_EndLayout();
}

// Solution + Reset buttons below the grid
Clay_RenderCommandArray CreateControlButtons(void) {
    Clay_BeginLayout();

    CLAY(CLAY_ID("ControlWrapper"), {
        .layout = {
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
            .childAlignment = {CLAY_ALIGN_X_RIGHT, CLAY_ALIGN_Y_BOTTOM},
            .childGap = 10,
            .padding = CLAY_PADDING_ALL(0),
            .sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)}
        }
    }) {
        CLAY(CLAY_ID("Solution"), {
            .layout = {.sizing = {.width = CLAY_SIZING_FIXED(230), .height = CLAY_SIZING_FIXED(30)}},
            .backgroundColor = (Clay_Color){255, 255, 255, 255},
            .cornerRadius = CLAY_CORNER_RADIUS(8)
        }) {}
        CLAY(CLAY_ID("Reset"), {
            .layout = {.sizing = {.width = CLAY_SIZING_FIXED(150), .height = CLAY_SIZING_FIXED(30)}},
            .backgroundColor = (Clay_Color){255, 255, 255, 255},
            .cornerRadius = CLAY_CORNER_RADIUS(8)
        }) {}
    }

    return Clay_EndLayout();
}



Clay_RenderCommandArray CreateNumberPadLayout(int screenWidth, int screenHeight) {
    Clay_BeginLayout();

    int buttonSize = 40; // Size of each numpad button
    int gap = 3;

    CLAY(CLAY_ID("NumberPadWrapper"), {
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childGap = gap,
            .childAlignment = {.x = CLAY_ALIGN_X_LEFT, .y = CLAY_ALIGN_Y_CENTER},
            .sizing = {.width = CLAY_SIZING_FIXED(buttonSize*3 + gap*2), .height = CLAY_SIZING_GROW(0)}
        }
    }) {
        int n = 1;
        for (int r = 0; r < 3; r++) {
            CLAY_AUTO_ID({
                .layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT, .childGap = gap}
            }) {
                for (int c = 0; c < 3; c++) {
                    numberButtons[n-1].value = n;

                    CLAY_AUTO_ID({
                        .layout = {
                            .sizing = {.width = CLAY_SIZING_FIXED(buttonSize),
                                       .height = CLAY_SIZING_FIXED(buttonSize)}
                        },
                        .backgroundColor = (Clay_Color){200, 200, 255, 255},
                        .cornerRadius = CLAY_CORNER_RADIUS(5)
                    }) {}
                    n++;
                }
            }
        }
    }

    return Clay_EndLayout();
}



void number_pad(Clay_RenderCommandArray commands, Vector2 mouse, int *activeCellIndex,
                unsigned char grid[CELLS], unsigned char initial_grid[CELLS]) {
    int n = 0;

    for (int i = 0; i < commands.length; i++) {
        Clay_RenderCommand *cmd = Clay_RenderCommandArray_Get(&commands, i);

        if (cmd->commandType != CLAY_RENDER_COMMAND_TYPE_RECTANGLE) continue;

        // Copy Clay_BoundingBox into a Raylib Rectangle
        Rectangle btnRect = {
            cmd->boundingBox.x,
            cmd->boundingBox.y,
            cmd->boundingBox.width,
            cmd->boundingBox.height
        };

        numberButtons[n].rect = btnRect;

        bool hovered = CheckCollisionPointRec(mouse, btnRect);
        Color fill = hovered ? (Color){150, 200, 255, 255} : (Color){200, 200, 255, 255};

        DrawRectangleRec(btnRect, fill);
        DrawRectangleLinesEx(btnRect, 1, BLACK);

        int ts = MeasureText(TextFormat("%d", numberButtons[n].value), 20);
        DrawText(TextFormat("%d", numberButtons[n].value),
                 btnRect.x + (btnRect.width - ts)/2,
                 btnRect.y + (btnRect.height - 20)/2,
                 20, BLACK);

        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (*activeCellIndex != -1 && initial_grid[*activeCellIndex] == 0) {
                grid[*activeCellIndex] = numberButtons[n].value;
            }
        }

        n++;
        if (n >= NUM_COUNT) break;
    }
}





void render_game_loop(Clay_Arena arena, int fontSize) {
    float gridX = (GetScreenWidth() - 30*9)/2;
    float gridY = (GetScreenWidth() - 30*9)/2;

    Clay_RenderCommandArray numPadCommands = CreateNumberPadLayout(GetScreenWidth(), GetScreenHeight());
    Clay_RenderCommandArray controlButtons = CreateControlButtons();
    Clay_RenderCommandArray backButton = CreateBackButton();

    while (!WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();
        Vector2 mistakesPos = {gridX + 260, gridY+10};
        Vector2 finishedGame = {gridX + 120, gridY + 30};

        // --- MENU ---
        if (gameState == GAME_MENU) {
            hoveredDifficulty = -1;

            Clay_RenderCommandArray menuCommands = CreateMenuLayout();

            Rectangle btnRects[3];
            int btnCount = 0;

            // Collect button rectangles
            for (int i = 0; i < menuCommands.length; i++) {
                Clay_RenderCommand *cmd = Clay_RenderCommandArray_Get(&menuCommands, i);
                if (cmd->commandType == CLAY_RENDER_COMMAND_TYPE_RECTANGLE &&
                    (int)cmd->boundingBox.width == 110 && (int)cmd->boundingBox.height == 50 &&
                    btnCount < 3) {
                    btnRects[btnCount].x = cmd->boundingBox.x;
                    btnRects[btnCount].y = cmd->boundingBox.y;
                    btnRects[btnCount].width = cmd->boundingBox.width;
                    btnRects[btnCount].height = cmd->boundingBox.height;
                    btnCount++;
                }
            }

            // Mouse hover & click detection
            bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
            int clickedIndex = -1;
            for (int b = 0; b < btnCount; b++) {
                if (CheckCollisionPointRec(mouse, btnRects[b])) {
                    hoveredDifficulty = b;
                    if (clicked) clickedIndex = b;
                }
            }

            // Handle button click
            if (clickedIndex != -1) {
                if (clickedIndex == 0) difficultyHoles = EASY_HOLES;
                else if (clickedIndex == 1) difficultyHoles = MEDIUM_HOLES;
                else difficultyHoles = HARD_HOLES;

                make_puzzle(grid, initial_grid, difficultyHoles);
                activeCellIndex = -1;
                flashError = false;
                flashFrames = 0;
                mistakes = 0;
                showSolution = false;
                gameState = GAME_PLAY;
            }

            // --- Draw Menu ---
            BeginDrawing();
            ClearBackground(WHITE);

            // Title
            DrawText("SUDOKU", GetScreenWidth()/2 - 100, btnRects[0].y - 60, 48, MAROON);

            // Buttons & text
            const char *labels[3] = {"EASY", "MEDIUM", "HARD"};
            for (int b = 0; b < btnCount; b++) {
                Color col = (hoveredDifficulty == b) ? (Color){HOVER_COLOR.r, HOVER_COLOR.g, HOVER_COLOR.b, HOVER_COLOR.a} : 
                (Color){255,255,255,255};
                DrawRectangleRec(btnRects[b], col);
                DrawRectangleLinesEx(btnRects[b], 2, BLACK);

                int textSize = MeasureText("EASY", 24);
                DrawText(labels[b],
                    btnRects[b].x + (btnRects[b].width - textSize)/2-15.0f,
                    btnRects[b].y + (btnRects[b].height - textSize)/2+22.0f,
                    24, BLACK);
            }

            EndDrawing();
            continue;
        }

        // --- GAMEPLAY ---
        Clay_RenderCommandArray gridCommands = CreateGridLayout();

        // Active cell selection
        if (!showSolution && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            activeCellIndex = -1;
            flashError = false;
            flashFrames = 0;
            float gridOffsetX = 70; // same as drawing
            float gridOffsetY = 0;  // should match the drawing offset

            for (int i = 0; i < gridCommands.length; i++) {
                Clay_RenderCommand *cmd = Clay_RenderCommandArray_Get(&gridCommands, i);
                Rectangle cellRect = {
                    cmd->boundingBox.x + gridOffsetX,
                    cmd->boundingBox.y + gridOffsetY,
                    cmd->boundingBox.width,
                    cmd->boundingBox.height
                };
                if ((int)cellRect.width == 30 && (int)cellRect.height == 30 &&
                    CheckCollisionPointRec(mouse, cellRect)) {
                    activeCellIndex = i;
                    break;
                }
            }
        }

        // Input handling
        if (!showSolution && activeCellIndex != -1 && initial_grid[activeCellIndex] == 0 && !gameComplete) {
            int key = GetKeyPressed();
            if (key >= KEY_ONE && key <= KEY_NINE) {
                u8 val = (u8)(key - KEY_ZERO);

                if (val == solution_grid[activeCellIndex]) {
                    grid[activeCellIndex] = val;
                    flashError = false;
                } else {
                    flashError = true;
                    flashFrames = FLASH_DURATION;
                    mistakes++;
                    if (mistakes >= 3) showSolution = true;
                }
            } else if (key == KEY_BACKSPACE || key == KEY_DELETE) {
                grid[activeCellIndex] = 0;
                flashError = false;
            }
        }

        if (flashFrames > 0) flashFrames--;
        else flashError = false;

        // --- DRAW GAME ---
        BeginDrawing();
        ClearBackground(WHITE); // background color set to white

        // Grid cells
        for (int i = 0; i < gridCommands.length; i++) {
            Clay_RenderCommand *cmd = Clay_RenderCommandArray_Get(&gridCommands, i);
            if ((int)cmd->boundingBox.width != 30 || (int)cmd->boundingBox.height != 30) continue;

            Rectangle cellRect = {cmd->boundingBox.x, cmd->boundingBox.y, cmd->boundingBox.width, cmd->boundingBox.height};
            bool hovered = CheckCollisionPointRec(mouse, cellRect);
            Color fillColor;

            if (showSolution) { 
                fillColor = (Color){STATIC_CELL_COLOR.r, STATIC_CELL_COLOR.g, STATIC_CELL_COLOR.b, STATIC_CELL_COLOR.a}; 
            } 
            else if (i == activeCellIndex && flashFrames > 0) { 
                fillColor = (Color){255, 100, 100, 255}; 
            } 
            else if (i == activeCellIndex) {
                fillColor = (Color){ACTIVE_COLOR.r, ACTIVE_COLOR.g, ACTIVE_COLOR.b, ACTIVE_COLOR.a}; 
            } 
            else if (hovered) {
                fillColor = (Color){HOVER_COLOR.r, HOVER_COLOR.g, HOVER_COLOR.b, HOVER_COLOR.a}; 
            } 
            else { 
                fillColor = (initial_grid[i] != 0) ? (Color){STATIC_CELL_COLOR.r, STATIC_CELL_COLOR.g, STATIC_CELL_COLOR.b, 
                    STATIC_CELL_COLOR.a} : (Color){EDITABLE_CELL_COLOR.r, EDITABLE_CELL_COLOR.g, EDITABLE_CELL_COLOR.b, 
                    EDITABLE_CELL_COLOR.a}; 
            }

            DrawRectangleRec(cellRect, fillColor);
            DrawRectangleLinesEx(cellRect, 1, BLACK);

            // Numbers
            if (grid[i] != 0 || showSolution) {
                char label[3];
                snprintf(label, sizeof(label), "%d", showSolution ? solution_grid[i] : grid[i]);
                int ts = MeasureText(label, fontSize);
                Color textColor = (initial_grid[i] != 0) ? GIVEN_NUM_COLOR : USER_NUM_COLOR;
                DrawText(label, 
                    cellRect.x + (cellRect.width - ts)/2, cellRect.y + (cellRect.height - ts)/2-3.0f,
                    fontSize, textColor);
            }
        }

        // --- Draw back button ---
        if (backButton.length > 0) {
            Clay_RenderCommand *cmd = Clay_RenderCommandArray_Get(&backButton, 0);
            Rectangle rect = {cmd->boundingBox.x, cmd->boundingBox.y, cmd->boundingBox.width+100, cmd->boundingBox.height};
            Color col = CheckCollisionPointRec(mouse, rect) ? (Color){180,220,255,255} : WHITE;
            DrawRectangleRec(rect, col);
            DrawRectangleLinesEx(rect, 1, BLACK);
            int ts = MeasureText("BACK", 12);
            DrawText("BACK", rect.x + (rect.width - ts)/2, rect.y + (rect.height - 12)/2, 12, BLACK);

            if (CheckCollisionPointRec(mouse, rect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                gameState = GAME_MENU;
            }
        }

        // --- Draw control buttons (Solution + Reset) ---
        for (int i = 0; i < controlButtons.length; i++) {
            Clay_RenderCommand *cmd = Clay_RenderCommandArray_Get(&controlButtons, i);
            Rectangle rect = {cmd->boundingBox.x + (i*80) + 320, cmd->boundingBox.y, cmd->boundingBox.width, cmd->boundingBox.height};
            Color col = CheckCollisionPointRec(mouse, rect) ? (Color){180,220,255,255} : WHITE;
            DrawRectangleRec(rect, col);
            DrawRectangleLinesEx(rect, 1, BLACK);

            const char *label = (i == 0) ? "SOLUTION" : "RESET";
            int ts = MeasureText(label, 12);
            DrawText(label, rect.x + (rect.width - ts)/2, rect.y + (rect.height - 12)/2, 12, BLACK);

            if (CheckCollisionPointRec(mouse, rect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (i == 0){
                    showSolution = true;
                }
                else {
                    make_puzzle(grid, initial_grid, difficultyHoles);
                    activeCellIndex = -1;
                    flashError = false;
                    flashFrames = 0;
                    mistakes = 0;
                    showSolution = false;
                }
            }
        }
        


        // Mistakes counter
        DrawText(
            TextFormat("Mistakes: %d/%d", mistakes, maxMistakes),
            mistakesPos.x,
            mistakesPos.y,
            16,
            RED
        );

        if (is_complete() && !gameComplete) {
            gameComplete = true;
            showSolution = true; 

        }

        if (gameComplete) {

            DrawText(
                "Sudoku \n Complete! 🎉",
                finishedGame.x,
                finishedGame.y,
                20,
                GREEN
            );

            
        }
        
        // Draw number pad
        number_pad(numPadCommands, mouse, &activeCellIndex, grid, initial_grid);


        EndDrawing();
    }
}


