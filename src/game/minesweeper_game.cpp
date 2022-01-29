#include <time.h>

#include "minesweeper_game.h"

typedef mine_cell *mine_cell_ptr;
#define LIST_TYPE mine_cell_ptr
#include "list.h"


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
            // TODO(ebuchholz): condense all these -1 to 1 searches somehow
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

void initMinesweeperGame (memory_arena *memory, memory_arena *tempMemory, minesweeper_game* game) {
    setRNGSeed((unsigned int)time(0));

    *game = {};

    game->currentView = VIEW_MODE_FIELD;

    game->screenRow = FIELD_HEIGHT - 3;
    game->screenCol = (FIELD_WIDTH / 2) - 1;
    game->screenX = 42.0f;
    game->screenY = 42.0f;

    game->ship.row = FIELD_HEIGHT -1;
    game->ship.col = FIELD_WIDTH / 2;
    game->ship.x = 42.0f;
    game->ship.y = 42.0f;

    int numBombs = 136;
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

    int numCellsLessLastRows = FIELD_WIDTH * (FIELD_HEIGHT - 4);
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

    mine_cell *firstCell = &game->cells[(FIELD_HEIGHT - 1) * FIELD_WIDTH + (FIELD_WIDTH / 2)];
    firstCell->open = true;
    openAllNearbyZeroValueCells(firstCell, tempMemory, game);
}

void updateMinesweeperGame (memory_arena *memory, memory_arena *tempMemory, game_assets *assets, game_input *input, 
                      minesweeper_game *game, sprite_list *spriteList)
{
    int fieldScreenRow = (game->screenRow + 3) - 18;
    float fieldScreenY = game->screenY / (float)SPACE_SHIP_ZOOM;
    if (fieldScreenRow < 0) {
        fieldScreenRow = 0;
        fieldScreenY = 0.0f;
    }

    pushSpriteTransform(spriteList, Vector2(0.0f, -(fieldScreenRow * CELL_DIM + fieldScreenY)));

    matrix3x3 gameTransform = peekSpriteMatrix(spriteList);
    vector3 localPointerPos = Vector3((float)input->pointerX, (float)input->pointerY, 1.0f);
    localPointerPos = inverse(gameTransform) * localPointerPos;

    popSpriteMatrix(spriteList);

    int mouseRow = (int)(localPointerPos.y / CELL_DIM);
    int mouseCol = (int)(localPointerPos.x / CELL_DIM);

    if (game->currentView == VIEW_MODE_FIELD) {
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

    gameTransform = peekSpriteMatrix(spriteList);
    localPointerPos = Vector3((float)input->pointerX, (float)input->pointerY, 1.0f);
    localPointerPos = inverse(gameTransform) * localPointerPos;

    if (localPointerPos.x >= 200.0f && localPointerPos.x < 280.0f && 
        localPointerPos.y >= 180.0f && localPointerPos.y < 207.0f)
    {
        if (input->pointerJustDown) {
            game->buttonDown = true;
        }
        else if (!input->pointerDown) {
            if (game->buttonDown) {
                game->buttonDown = false;

                if (game->currentView == VIEW_MODE_SHIP) {
                    game->currentView = VIEW_MODE_FIELD;
                }
                else if (game->currentView == VIEW_MODE_FIELD) {
                    game->currentView = VIEW_MODE_SHIP;
                }
            }
        }
    }
    else {
        game->buttonDown = false;
    }

    // move ship
    float shipSpeed = 84.0f;

    if (game->currentView == VIEW_MODE_SHIP) {
        if (input->leftKey.down || input->aKey.down) {
            game->ship.x -= shipSpeed * DELTA_TIME;
        }
        if (input->rightKey.down || input->dKey.down) {
            game->ship.x += shipSpeed * DELTA_TIME;
        }
        if (input->upKey.down || input->wKey.down) {
            game->ship.y -= shipSpeed * DELTA_TIME;
        }
        if (input->downKey.down || input->sKey.down) {
            game->ship.y += shipSpeed * DELTA_TIME;
        }
    }

    game->ship.y += -21.0f * DELTA_TIME;
    if (game->ship.y < 0.0f) {
        game->ship.row--;
        game->ship.y += SPACE_SHIP_CELL_DIM;
    }
    if (game->ship.y >= SPACE_SHIP_CELL_DIM) {
        game->ship.row++;
        game->ship.y -= SPACE_SHIP_CELL_DIM;
    }
    if (game->ship.x < 0.0f) {
        game->ship.col--;
        game->ship.x += SPACE_SHIP_CELL_DIM;
    }
    if (game->ship.x >= SPACE_SHIP_CELL_DIM) {
        game->ship.col++;
        game->ship.x -= SPACE_SHIP_CELL_DIM;
    }


    game->screenY += -21.0f * DELTA_TIME;
    if (game->screenY < 0.0f) {
        game->screenRow--;
        game->screenY += SPACE_SHIP_CELL_DIM;
    }


    float movementWindow = 80.0f;
    float shipScreenX = (game->ship.col - game->screenCol) * SPACE_SHIP_CELL_DIM + game->ship.x;
    float distToLeftEdge = shipScreenX - game->screenX;
    if (distToLeftEdge < movementWindow) {
        game->screenX = shipScreenX - movementWindow;
        while (game->screenX < 0.0f) {
            game->screenCol--;
            game->screenX += SPACE_SHIP_CELL_DIM;
        }

        if (game->screenCol < 0) {
            game->screenCol = 0;
            game->screenX = 0.0f;
        }
    }

    float distToRightEdge = (game->screenX + 192.0f) - shipScreenX;
    if (distToRightEdge < movementWindow) {
        game->screenX = shipScreenX + movementWindow - 192.0f;
        while (game->screenX >= SPACE_SHIP_CELL_DIM) {
            game->screenCol++;
            game->screenX -= SPACE_SHIP_CELL_DIM;
        }
    }

    float screenXToRightEdge = (FIELD_WIDTH - game->screenCol) * SPACE_SHIP_CELL_DIM - game->screenX;
    if (screenXToRightEdge < 192.0f) {
        game->screenCol = FIELD_WIDTH;
        game->screenX = -192.0f;
        while (game->screenX < 0.0f) {
            game->screenCol--;
            game->screenX += SPACE_SHIP_CELL_DIM;
        }
    }


    if (game->screenX < 0.0f) {
        game->screenCol--;
        game->screenX += SPACE_SHIP_CELL_DIM;
    }
    if (game->screenX >= SPACE_SHIP_CELL_DIM) {
        game->screenCol++;
        game->screenX -= SPACE_SHIP_CELL_DIM;
    }

    game->leftHandState = HAND_STATE_MIDDLE;
    game->rightHandState = HAND_STATE_MIDDLE;

    if (game->currentView == VIEW_MODE_SHIP) {
        if (input->leftKey.down || input->aKey.down) {
            game->leftHandState = HAND_STATE_LEFT;
        }
        if (input->rightKey.down || input->dKey.down) {
            game->leftHandState = HAND_STATE_RIGHT;
        }
    }
    else {
        if (localPointerPos.x < 64.0f) {
            game->rightHandState = HAND_STATE_LEFT;
        }
        else if (localPointerPos.x >= 128.0f) {
            game->rightHandState = HAND_STATE_RIGHT;
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

    if (game->currentView == VIEW_MODE_FIELD) {

        int fieldScreenRow = (game->screenRow + 3) - 18;
        float fieldScreenY = game->screenY / (float)SPACE_SHIP_ZOOM;
    if (fieldScreenRow < 0) {
        fieldScreenRow = 0;
        fieldScreenY = 0.0f;
    }

        pushSpriteTransform(spriteList, Vector2(0.0f, -(fieldScreenRow * CELL_DIM + fieldScreenY)));

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

        sprite shipSprite = Sprite();
        shipSprite.pos.x = (float)(game->ship.col * CELL_DIM + (game->ship.x / ((float)SPACE_SHIP_ZOOM)));
        shipSprite.pos.y = (float)(game->ship.row * CELL_DIM + (game->ship.y / ((float)SPACE_SHIP_ZOOM)));
        shipSprite.anchor.x = 0.5f;
        shipSprite.anchor.y = 0.5f;
        addSprite(shipSprite, assets, "atlas", "ship_icon", spriteList);

        popSpriteMatrix(spriteList);
    }
    else if (game->currentView == VIEW_MODE_SHIP) {
        pushSpriteTransform(spriteList, Vector2(-game->screenX, -game->screenY));

        static float bombRotation = 0.0f;
        bombRotation -= (1.0f / 4.0f * PI) * DELTA_TIME;

        for (int i = 0; i < 5; ++i) {
            for (int j = 0; j < 5; ++j) {
                int row = game->screenRow + i;
                int col = game->screenCol + j;
                if (row < 0 || row >= FIELD_HEIGHT ||
                    col < 0 || col >= FIELD_WIDTH) 
                {
                    continue;
                }

                sprite cellSprite = Sprite();
                cellSprite.pos.x = (float)(j * SPACE_SHIP_ZOOM * CELL_DIM);
                cellSprite.pos.y = (float)(i * SPACE_SHIP_ZOOM * CELL_DIM);

                mine_cell *cell = &game->cells[row * FIELD_WIDTH + col];
                if  (cell->open) {
                    addSprite(cellSprite, assets, "atlas", "cell_outline", spriteList);

                    if (cell->hasBomb) {
                        sprite bombSprite = Sprite();
                        bombSprite.pos.x = (float)(j * SPACE_SHIP_ZOOM * CELL_DIM + 0.5f * (SPACE_SHIP_ZOOM * CELL_DIM));
                        bombSprite.pos.y = (float)(i * SPACE_SHIP_ZOOM * CELL_DIM + 0.5f * (SPACE_SHIP_ZOOM * CELL_DIM));
                        bombSprite.anchor.x = 0.5f;
                        bombSprite.anchor.y = 0.5f;
                        bombSprite.rotation = bombRotation;
                        addSprite(bombSprite, assets, "atlas", "big_bomb", spriteList);
                    }
                    else {
                        if (cell->numAdjBombs > 0) {
                            addSprite(cellSprite, assets, "atlas", appendString("big_", numToString(cell->numAdjBombs, &stringMemory), &stringMemory), spriteList);
                        }
                    }
                }
                else {
                    addSprite(cellSprite, assets, "atlas", "big_cell_closed", spriteList);
                    if (cell->flagged) {
                        addSprite(cellSprite, assets, "atlas", "big_flag", spriteList);
                    }
                }

            }
        }

        float shipX = (game->ship.col - game->screenCol) * SPACE_SHIP_CELL_DIM + game->ship.x;
        float shipY = (game->ship.row - game->screenRow) * SPACE_SHIP_CELL_DIM + game->ship.y;

        sprite shipSprite = Sprite();
        shipSprite.pos.x = shipX;
        shipSprite.pos.y = shipY;
        shipSprite.anchor.x = 0.5f;
        shipSprite.anchor.y = 0.5f;
        addSprite(shipSprite, assets, "atlas", "ship", spriteList);

        popSpriteMatrix(spriteList);
    }

    sprite uiSprite = Sprite();
    uiSprite.pos.x = 192.0f;
    uiSprite.pos.y = 0.0f;
    addSprite(uiSprite, assets, "atlas", "ui_panel", spriteList);

    sprite leftHandSprite = Sprite();
    leftHandSprite.pos.x = 260.0f;
    if (game->leftHandState == HAND_STATE_LEFT) {
        leftHandSprite.pos.x = 262.0f;
    }
    else if (game->leftHandState == HAND_STATE_RIGHT) {
        leftHandSprite.pos.x = 258.0f;
    }
    leftHandSprite.pos.y = 55.0f;
    leftHandSprite.anchor.x = 0.0f;
    leftHandSprite.anchor.y = 1.0f;
    addSprite(leftHandSprite, assets, "atlas", "right_hand", spriteList);


    sprite rightHandSprite = Sprite();
    rightHandSprite.pos.x = 204.0f;
    if (game->rightHandState == HAND_STATE_LEFT) {
        rightHandSprite.pos.x = 206.0f;
    }
    else if (game->rightHandState == HAND_STATE_RIGHT) {
        rightHandSprite.pos.x = 202.0f;
    }
    rightHandSprite.pos.y = 55.0f;
    rightHandSprite.anchor.x = 0.0f;
    rightHandSprite.anchor.y = 1.0f;
    addSprite(rightHandSprite, assets, "atlas", "left_hand", spriteList);

    addText(220.0f, 63.0f, "SCORE", assets, "font", spriteList);

    int digits = 1;
    int tempScore = game->score;
    while (tempScore > 0) {
        tempScore /= 10;
        ++digits;
    }

    float shift = (float)(-digits) * 4.0f;
    addText(242.0f + shift, 80.0f, numToString(game->score, &stringMemory), assets, "font", spriteList);

    for (int i = 0; i < game->hitpoints; ++i) {
        sprite hitpointSprite = Sprite();
        hitpointSprite.pos.x = 203.0f + i * 9.0f;
        hitpointSprite.pos.y = 101.0f;
        addSprite(hitpointSprite, assets, "atlas", "hitpoint", spriteList);
    }

    sprite lifeSprite = Sprite();
    lifeSprite.pos.x = 244.0f;
    lifeSprite.pos.y = 101.0f;
    addSprite(lifeSprite, assets, "atlas", "ship", spriteList);

    addText(264.0f, 101.0f, appendString("x", numToString(game->lives, &stringMemory), &stringMemory), assets, "font", spriteList);

    addText(224.0f, 122.0f, "BEAM", assets, "font", spriteList);

    sprite buttonSprite = Sprite();
    buttonSprite.pos.x = 200.0f;
    buttonSprite.pos.y = 180.0f;
    if (game->buttonDown) {
        addSprite(buttonSprite, assets, "atlas", "button_down", spriteList);
    }
    else {
        addSprite(buttonSprite, assets, "atlas", "button_up", spriteList);
    }

    if (game->currentView == VIEW_MODE_FIELD) {
        addText(222.0f, 185.0f, "SHIP", assets, "font", spriteList);
    }
    else {
        addText(218.0f, 185.0f, "FIELD", assets, "font", spriteList);
    }
    addText(222.0f, 195.0f, "VIEW", assets, "font", spriteList);
}
