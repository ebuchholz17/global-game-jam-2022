#include <time.h>

#include "minesweeper_game.h"

typedef mine_cell *mine_cell_ptr;
#define LIST_TYPE mine_cell_ptr
#include "list.h"

void initMinesweeperGame (memory_arena *memory, minesweeper_game* game) {
    setRNGSeed((unsigned int)time(0));

    *game = {};

    int numBombs = 38;
    for (int i = 0; i < FIELD_HEIGHT; ++i) {
        for (int j = 0; j < FIELD_WIDTH; ++j) {
            int index = i * FIELD_WIDTH + j;
            mine_cell *cell = &game->cells[index];
            cell->row = i;
            cell->col = j;

            if (index < numBombs) {
                cell->hasBomb = true;
            }
        }
    }

    int numCellsLessLastRows = FIELD_WIDTH * (FIELD_HEIGHT - 2);
    for (int i = numCellsLessLastRows - 1; i >= 0; --i) {
        mine_cell *cell = &game->cells[i];

        int randomIndex = randomUint() % (numCellsLessLastRows);
        mine_cell *randomCell = &game->cells[randomIndex];
    
        bool temp = cell->hasBomb;
        cell->hasBomb = randomCell->hasBomb;
        randomCell->hasBomb = temp;
    }

    for (int cellIndex = 0; cellIndex < FIELD_WIDTH * FIELD_HEIGHT; ++cellIndex) {
        mine_cell *cell = &game->cells[cellIndex];

        for (int i = -1; i <= 1; ++i) {
            for (int j = -1; j <= 1; ++j) {
                int row = cell->row + i;
                int col = cell->col + j;

                if ((row == cell->row && col == cell->col )|| 
                    row < 0 || row >= FIELD_HEIGHT ||
                    col < 0 || col >= FIELD_WIDTH) 
                {
                    continue;
                }

                mine_cell *neighbor = &game->cells[row * FIELD_WIDTH + col];
                if (neighbor->hasBomb) {
                    cell->numAdjBombs++;
                }
            }
        }
    }
}

void openAllNearbyZeroValueCells (mine_cell *startCell, memory_arena *tempMemory, minesweeper_game *game) {

    unsigned int prevSize = tempMemory->size;
    mine_cell_ptr_list cellsToOpen = mine_cell_ptrListInit(tempMemory, FIELD_WIDTH * FIELD_HEIGHT);
    listPush(&cellsToOpen, startCell);

    for (int cellIndex = 0; cellIndex < FIELD_WIDTH * FIELD_HEIGHT; ++cellIndex) {
        mine_cell *cell = &game->cells[cellIndex];
        cell->seen = false;
    }

    while (cellsToOpen.numValues > 0) {
        mine_cell *currentCell = cellsToOpen.values[0];
        currentCell->open = true;
        listSplice(&cellsToOpen, 0);

        if (currentCell->numAdjBombs == 0) {
            for (int i = -1; i <= 1; ++i) {
                for (int j = -1; j <= 1; ++j) {
                    int row = currentCell->row + i;
                    int col = currentCell->col + j;

                    if ((row == currentCell->row && col == currentCell->col )|| 
                        row < 0 || row >= FIELD_HEIGHT ||
                        col < 0 || col >= FIELD_WIDTH) 
                    {
                        continue;
                    }

                    mine_cell *neighbor = &game->cells[row * FIELD_WIDTH + col];
                    if (!neighbor->seen) {
                        neighbor->seen = true;
                        listPush(&cellsToOpen, neighbor);
                    }
                }
            }
        }
    }

    tempMemory->size = prevSize;
}

void updateMinesweeperGame (memory_arena *memory, memory_arena *tempMemory, game_assets *assets, game_input *input, 
                      minesweeper_game *game, sprite_list *spriteList)
{
    matrix3x3 gameTransform = peekSpriteMatrix(spriteList);
    vector3 localPointerPos = Vector3((float)input->pointerX, (float)input->pointerY, 1.0f);
    localPointerPos = inverse(gameTransform) * localPointerPos;


    int mouseRow = (int)(localPointerPos.y / CELL_DIM);
    int mouseCol = (int)(localPointerPos.x / CELL_DIM);

    if (mouseRow >= 0 && mouseRow < FIELD_HEIGHT && mouseCol >= 0 && mouseCol < FIELD_WIDTH) {
        if (input->pointerJustDown) {
            mine_cell *cell = &game->cells[mouseRow * FIELD_WIDTH + mouseCol];
            if (!cell->open) {
                if (!cell->flagged) {
                    cell->open = true;

                    if (cell->hasBomb) {

                    }
                    else if (cell->numAdjBombs == 0) {
                        openAllNearbyZeroValueCells(cell, tempMemory, game);
                    }
                }
            }
            else {
                if (!cell->hasBomb && cell->numAdjBombs > 0) {
                    int numFlaggedNeighbors = 0;
                    for (int i = -1; i <= 1; ++i) {
                        for (int j = -1; j <= 1; ++j) {
                            int row = cell->row + i;
                            int col = cell->col + j;

                            if ((row == cell->row && col == cell->col )|| 
                                row < 0 || row >= FIELD_HEIGHT ||
                                col < 0 || col >= FIELD_WIDTH) 
                            {
                                continue;
                            }

                            mine_cell *neighbor = &game->cells[row * FIELD_WIDTH + col];
                            if (neighbor->flagged || (neighbor->open && neighbor->hasBomb)) {
                                numFlaggedNeighbors++;
                            }
                        }
                    }
                    if (numFlaggedNeighbors == cell->numAdjBombs) {
                        for (int i = -1; i <= 1; ++i) {
                            for (int j = -1; j <= 1; ++j) {
                                int row = cell->row + i;
                                int col = cell->col + j;

                                if ((row == cell->row && col == cell->col )|| 
                                    row < 0 || row >= FIELD_HEIGHT ||
                                    col < 0 || col >= FIELD_WIDTH) 
                                {
                                    continue;
                                }

                                mine_cell *neighbor = &game->cells[row * FIELD_WIDTH + col];
                                if (!neighbor->open && !neighbor->flagged) {
                                    neighbor->open = true;

                                    if (neighbor->hasBomb) {

                                    }
                                    else if (neighbor->numAdjBombs == 0) {
                                        openAllNearbyZeroValueCells(neighbor, tempMemory, game);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (input->pointer2JustDown) {
            mine_cell *cell = &game->cells[mouseRow * FIELD_WIDTH + mouseCol];
            if (!cell->open) {
                cell->flagged = !cell->flagged;
            }
        }
    }
}

void drawMinesweeperGame (memory_arena *memory, memory_arena *tempMemory, game_assets *assets, 
                    minesweeper_game *game, sprite_list *spriteList) 
{
    // memory for dynamically created strings
    memory_arena stringMemory = {};
    stringMemory.capacity = 512 * 1024;
    stringMemory.base = allocateMemorySize(tempMemory, stringMemory.capacity);


    for (int i = 0; i < FIELD_HEIGHT; ++i) {
        for (int j = 0; j < FIELD_WIDTH; ++j) {
            mine_cell *cell = &game->cells[i * FIELD_WIDTH + j];
            cell->row = i;
            cell->col = j;

            if (!cell->open) {
                sprite cellSprite = Sprite();
                cellSprite.pos.x = (float)(cell->col * CELL_DIM);
                cellSprite.pos.y = (float)(cell->row * CELL_DIM);
                addSprite(cellSprite, assets, "atlas", "closed_cell", spriteList);

                if (cell->flagged) {
                    addSprite(cellSprite, assets, "atlas", "flag", spriteList);
                }

                //if (cell->hasBomb) {
                //    addSprite(cellSprite, assets, "atlas", "flag", spriteList);
                //}
            }
            else {
                sprite cellSprite = Sprite();
                cellSprite.pos.x = (float)(cell->col * CELL_DIM);
                cellSprite.pos.y = (float)(cell->row * CELL_DIM);
                addSprite(cellSprite, assets, "atlas", "open_cell", spriteList);

                if (cell->hasBomb) {
                    addSprite(cellSprite, assets, "atlas", "bomb", spriteList);
                }
                else {
                    if (cell->numAdjBombs > 0) {
                        addSprite(cellSprite, assets, "atlas", numToString(cell->numAdjBombs, &stringMemory), spriteList);
                    }
                }
            }
        }
    }

    //pushSpriteTransform(spriteList, Vector2(GAME_WIDTH/2.0f, GAME_HEIGHT/2.0f));
    //popSpriteMatrix(spriteList);
}
