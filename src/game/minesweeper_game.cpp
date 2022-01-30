#include <time.h>

#include "minesweeper_game.h"


void openAllNearbyZeroValueCells (mine_cell *startCell, memory_arena *tempMemory, minesweeper_game *game, bool awardScore) {

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
        if (awardScore) {
            game->score += 10;
        }
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

void determineNumAdjBombs (minesweeper_game *game) {

    for (int cellIndex = 0; cellIndex < FIELD_WIDTH * FIELD_HEIGHT; ++cellIndex) {
        mine_cell *cell = &game->cells[cellIndex];
        cell->numAdjBombs = 0;

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

void initMinesweeperGame (memory_arena *memory, memory_arena *tempMemory, minesweeper_game* game) {
    setRNGSeed((unsigned int)time(0));

    *game = {};

    game->currentView = VIEW_MODE_FIELD;
    game->state = GAME_STATE_TITLE_SCREEN;

    game->screenRow = FIELD_HEIGHT - 3;
    game->screenCol = (FIELD_WIDTH / 2) - 1;
    game->screenX = 42.0f;
    game->screenY = 42.0f;

    game->ship.row = FIELD_HEIGHT -1;
    game->ship.col = FIELD_WIDTH / 2;
    game->ship.x = 42.0f;
    game->ship.y = 42.0f;
    game->ship.beamPower = 30;

    game->enemyPattern.enemyIDs = entity_idListInit(memory, 30);
    game->enemyPattern.type = ENEMY_PATTERN_TYPE_NONE;

    game->revealingCells = mine_cell_ptrListInit(memory, 30);

    for (int i = 0; i < 30; ++i) {
        space_enemy *enemy = &game->enemies[i];
        enemy->id.value = i;
    }

    for (int i = 0; i < MAX_NUM_PLAYER_BULLETS; ++i) {
        space_bullet *bullet = &game->playerBullets[i];
        bullet->id.value = i;
    }

    for (int i = 0; i < MAX_NUM_ENEMY_BULLETS; ++i) {
        space_bullet *bullet = &game->enemyBullets[i];
        bullet->id.value = i;
    }

    for (int i = 0; i < MAX_EXPLOSIONS; ++i) {
        space_explosion *explosion = &game->explosions[i];
        explosion->id.value = i;
    }

    int numBombs = 136;
    for (int i = 0; i < FIELD_HEIGHT; ++i) {
        for (int j = 0; j < FIELD_WIDTH; ++j) {
            int index = i * FIELD_WIDTH + j;
            mine_cell *cell = &game->cells[index];
            cell->row = i;
            cell->col = j;
            cell->hitpoints = 2000;

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

    for (int i = numCellsLessLastRows - 1; i >= 0; --i) {

        mine_cell *cell = &game->cells[i];
        if (!cell->hasBomb) {
            float roll = randomFloat();

            float sWeight = 0.01f;
            float wWeight = 0.01f;
            float lWeight = 0.01f;
            float healthWeight = 0.02f;

            if (roll < sWeight) {
                cell->hasSPower = true;
            }
            else if (roll < sWeight + wWeight) {
                cell->hasWPower = true;
            }
            else if (roll < sWeight + wWeight + lWeight) {
                cell->hasLPower = true;
            }
            else if (roll < sWeight + wWeight + lWeight + healthWeight) {
                cell->hasExtraHealth = true;
            }

        }
        int randomIndex = randomUint() % (numCellsLessLastRows);
        mine_cell *randomCell = &game->cells[randomIndex];
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
    openAllNearbyZeroValueCells(firstCell, tempMemory, game, false);
}

void zoomIntoShip (minesweeper_game *game) {
    game->zoomT += DELTA_TIME;
    if (game->zoomT >= 0.5f) {
        game->state = GAME_STATE_NORMAL;
        game->zoomT = 0.0f;
        game->currentView = VIEW_MODE_SHIP;
    }
}

void zoomIntoBattle (minesweeper_game *game) {
    game->zoomT += DELTA_TIME;
    if (game->zoomT >= 0.5f) {
        game->state = GAME_STATE_BATTLE;
        game->zoomT = 0.0f;
        game->currentView = VIEW_MODE_BATTLE;
        game->battleTimer = 0.0f;
        game->waveNum = 0;
    }
}

void zoomOutOfBattle (minesweeper_game *game) {
    game->zoomT += DELTA_TIME;
    if (game->zoomT >= 0.5f) {
        game->state = GAME_STATE_REVEALING_CELLS;
        game->zoomT = 0.0f;

        mine_cell *wormholeCell = &game->cells[game->wormholeRow * FIELD_WIDTH + game->wormholeCol];
        wormholeCell->hasBomb = false;

        determineNumAdjBombs(game);

        game->revealingCells.numValues = 0;

        for (int i = -2; i <= 2; ++i) {
            for (int j = -2; j <= 2; ++j) {
                int row = wormholeCell->row + i;
                int col = wormholeCell->col + j;

                if ((row == wormholeCell->row && col == wormholeCell->col )|| 
                    row < 0 || row >= FIELD_HEIGHT ||
                    col < 0 || col >= FIELD_WIDTH) 
                {
                    continue;
                }

                mine_cell *neighbor = &game->cells[row * FIELD_WIDTH + col];
                listPush(&game->revealingCells, neighbor);
            }
        }

        game->suckingIntoWormhole = false;
        game->touchingWormhole = false;
    }
}

void zoomOutOfShip (minesweeper_game *game) {
    game->zoomT += DELTA_TIME;
    if (game->zoomT >= 0.5f) {
        game->state = GAME_STATE_NORMAL;
        game->zoomT = 0.0f;
    }
}

void updateFieldView (memory_arena *memory, memory_arena *tempMemory, game_assets *assets, game_sounds * gameSounds, game_input *input, 
                      minesweeper_game *game, sprite_list *spriteList) {
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
                            playSound("wormhole_revealed", gameSounds, assets);
                        }
                        else {
                            playSound("click", gameSounds, assets);
                            game->score += 10;
                            if (cell->numAdjBombs == 0) {
                            openAllNearbyZeroValueCells(cell, tempMemory, game, true);
                            }
                        }
                    }
                }
                else {
                    if (cell->hasSPower) {
                        game->ship.hasShield = true;

                        cell->hasSPower = false;
                        playSound("pickup", gameSounds, assets);
                    }
                    else if (cell->hasWPower) {

                        if (game->ship.beamType == BEAM_TYPE_WAVE) {
                            game->ship.beamLevel++;
                            if (game->ship.beamLevel > 2) {
                                game->ship.beamLevel = 2;
                            }
                            game->ship.beamPower = 15;
                        }
                        else {
                            game->ship.beamType = BEAM_TYPE_WAVE;
                            game->ship.beamLevel = 0;
                            game->ship.beamPower = 15;
                        }

                        playSound("pickup", gameSounds, assets);
                        cell->hasWPower = false;
                    }
                    else if (cell->hasLPower) {

                        if (game->ship.beamType == BEAM_TYPE_LASER) {
                            game->ship.beamLevel++;
                            if (game->ship.beamLevel > 2) {
                                game->ship.beamLevel = 2;
                            }
                            game->ship.beamPower = 4;
                        }
                        else {
                            game->ship.beamType = BEAM_TYPE_LASER;
                            game->ship.beamLevel = 0;
                            game->ship.beamPower = 4;
                        }

                        playSound("pickup", gameSounds, assets);
                        cell->hasLPower = false;
                    }
                    else if (cell->hasExtraHealth) {
                        game->hitpoints++;
                        if (game->hitpoints > 4) {
                            game->hitpoints = 4;
                        }
                        playSound("pickup", gameSounds, assets);
                        cell->hasExtraHealth = false;
                    }
                    else if (!cell->hasBomb && cell->numAdjBombs > 0) {
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
                                if (neighbor->flagged || (neighbor->open && neighbor->hasBomb) || (!neighbor->open && neighbor->hasBomb && neighbor->cracked)) {
                                    numFlaggedNeighbors++;
                                }
                            }
                        }
                        if (numFlaggedNeighbors == cell->numAdjBombs) {
                            playSound("click", gameSounds, assets);

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
                                    if (!neighbor->open && !neighbor->flagged && !(neighbor->cracked && neighbor->hasBomb)) {
                                        neighbor->open = true;
                                        game->score += 10;

                                        if (neighbor->hasBomb) {

                                        }
                                        else if (neighbor->numAdjBombs == 0) {
                                            openAllNearbyZeroValueCells(neighbor, tempMemory, game, true);
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
                    playSound("click", gameSounds, assets);
                }
            }
        }

        if (game->touchingWormhole) {
            game->state = GAME_STATE_ZOOMING_IN;
            playSound("zoom_in", gameSounds, assets);
        }
    }
}

void trySwitchViews (memory_arena *memory, memory_arena *tempMemory, game_assets *assets, game_sounds *gameSounds, game_input *input, 
                      minesweeper_game *game, sprite_list *spriteList) {
    matrix3x3 gameTransform = peekSpriteMatrix(spriteList);
    vector3 localPointerPos = Vector3((float)input->pointerX, (float)input->pointerY, 1.0f);
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
                    game->state = GAME_STATE_ZOOMING_OUT;
                    playSound("zoom_in", gameSounds, assets);
                }
                else if (game->currentView == VIEW_MODE_FIELD) {
                    game->state = GAME_STATE_ZOOMING_IN;
                    playSound("zoom_in", gameSounds, assets);
                }
            }
        }
    }
    else {
        game->buttonDown = false;
    }
}

vector2 distBetweenRowColXYs (int row0, int col0, float x0, float y0, 
                              int row1, int col1, float x1, float y1) 
{
    vector2 result = Vector2();

    int rowDiff = row1 - row0;
    float rowFloatDiff = rowDiff * SPACE_SHIP_CELL_DIM;
    float yDiff = y1 - y0;
    result.y = rowFloatDiff + yDiff;

    int colDiff = col1 - col0;
    float colFloatDiff = colDiff * SPACE_SHIP_CELL_DIM;
    float xDiff = x1 - x0;
    result.x = colFloatDiff + xDiff;

    return result;
}

void removeAllBullets (minesweeper_game *game) {
    for (int i = 0; i < MAX_NUM_PLAYER_BULLETS; ++i) {
        space_bullet *bullet = &game->playerBullets[i];
        if (bullet->active) {
            bullet->active = false;
            bullet->id.generation++;
        }
    }

    for (int i = 0; i < MAX_NUM_ENEMY_BULLETS; ++i) {
        space_bullet *bullet = &game->enemyBullets[i];
        if (bullet->active) {
            bullet->active = false;
            bullet->id.generation++;
        }
    }
}

space_explosion *activateExplosion (minesweeper_game *game, game_assets *assets, game_sounds *gameSounds) {
    for (int i = 0; i < 30; ++i) {
        space_explosion *explosion = &game->explosions[i];
        if (!explosion->active) {
            entity_id id = explosion->id;
            *explosion = {};
            explosion->active = true;
            explosion->id = id;
            playSound("explode", gameSounds, assets);
            return explosion;
        }
    }

    assert(0);
    return 0;
}

space_bullet *spawnPlayerBullet (minesweeper_game *game) {
    for (int i = 0; i < MAX_NUM_PLAYER_BULLETS; ++i) {
        space_bullet *bullet = &game->playerBullets[i];
        if (!bullet->active) {
            entity_id id = bullet->id;
            *bullet = {};
            bullet->active = true;
            bullet->id = id;
            return bullet;
        }
    }
    return 0;
}

void despawnPlayerBullet(space_bullet *bullet) {
    bullet->active = false;
    bullet->id.generation++;
}

void updatePlayerBulletsDuringShipView (minesweeper_game *game, game_assets *assets, game_sounds *gameSounds) {
    for (int i = 0; i < MAX_NUM_PLAYER_BULLETS; ++i) {
        space_bullet *bullet = game->playerBullets + i;
        if (bullet->active) {
            bullet->pos += DELTA_TIME * bullet->velocity;

            if (bullet->pos.y < 0.0f) {
                bullet->row--;
                bullet->pos.y += SPACE_SHIP_CELL_DIM;
            }
            if (bullet->pos.y >= SPACE_SHIP_CELL_DIM) {
                bullet->row++;
                bullet->pos.y -= SPACE_SHIP_CELL_DIM;
            }
            if (bullet->pos.x < 0.0f) {
                bullet->col--;
                bullet->pos.x += SPACE_SHIP_CELL_DIM;
            }
            if (bullet->pos.x >= SPACE_SHIP_CELL_DIM) {
                bullet->col++;
                bullet->pos.x -= SPACE_SHIP_CELL_DIM;
            }

            float bulletScreenX = (bullet->col - game->screenCol) * SPACE_SHIP_CELL_DIM + bullet->pos.x - game->screenX;
            float bulletScreenY = (bullet->row - game->screenRow) * SPACE_SHIP_CELL_DIM + bullet->pos.y - game->screenY;

            bullet->timeAlive += DELTA_TIME;
            bool despawn = false;
            if (bullet->timeAlive > BULLET_LIFETIME) {
                despawn = true;
            }

            if (bulletScreenX < 0.0f || bulletScreenX >= 192.0f ||
                bulletScreenY < 0.0f || bulletScreenY >= 216.0f) 
            {
                despawn = true;
            }

            if (bullet->row >= 0 && bullet->row < FIELD_HEIGHT &&
                bullet->col >= 0 && bullet->col < FIELD_WIDTH) 
            {
                mine_cell *currentCell = &game->cells[bullet->row * FIELD_WIDTH + bullet->col];
                if (!currentCell->open) {
                    currentCell->hitpoints -= game->ship.beamPower;
                    playSound("impact", gameSounds, assets);
                    despawn = true;
                    currentCell->hitTimer = 0.15f;

                    if (currentCell->hitpoints <= 500) {
                        currentCell->cracked = true;
                    }
                    if (currentCell->hitpoints <= 0) {
                        if (!currentCell->hasBomb) {

                                        game->score += 10;
                        }
                        currentCell->open = true;
                    }

                }

                if (despawn) {
                    despawnPlayerBullet(bullet);
                }
            }
        }
    }
}

void spawnBeam (minesweeper_game *game) {
    float shipX = game->currentView == VIEW_MODE_BATTLE ? game->ship.battleX : game->ship.x;
    float shipY = game->currentView == VIEW_MODE_BATTLE ? game->ship.battleY : game->ship.y;
    switch (game->ship.beamType) {
        case BEAM_TYPE_NORMAL: {
            if (game->bulletTimer > 0.2f) {
                game->bulletTimer = 0.0f;

                space_bullet *bullet = spawnPlayerBullet(game);
                if (bullet) {
                    bullet->row = game->ship.row;
                    bullet->col = game->ship.col;
                    bullet->pos = Vector2(shipX, shipY) + Vector2(0.0f, -3.0f);
                    bullet->velocity = 250.0f * normalize(Vector2(0.0f, -1.0f));
                }
            }
        } break;
        case BEAM_TYPE_LASER: {
            if (game->bulletTimer > 0.05f) {
                game->bulletTimer = 0.0f;

                if (game->ship.beamLevel == 0) {
                    space_bullet *bullet = spawnPlayerBullet(game);
                    if (bullet) {
                        bullet->row = game->ship.row;
                        bullet->col = game->ship.col;
                        bullet->pos = Vector2(shipX, shipY) + Vector2(0.0f, -3.0f);
                        bullet->velocity = 300.0f * normalize(Vector2(0.0f, -1.0f));
                    }
                }
                else if (game->ship.beamLevel == 1) {
                    space_bullet *bullet = spawnPlayerBullet(game);
                    if (bullet) {
                        bullet->row = game->ship.row;
                        bullet->col = game->ship.col;
                        bullet->pos = Vector2(shipX, shipY) + Vector2(2.0f, -3.0f);
                        bullet->velocity = 300.0f * normalize(Vector2(0.0f, -1.0f));
                    }
                    bullet = spawnPlayerBullet(game);
                    if (bullet) {
                        bullet->row = game->ship.row;
                        bullet->col = game->ship.col;
                        bullet->pos = Vector2(shipX, shipY) + Vector2(-2.0f, -3.0f);
                        bullet->velocity = 300.0f * normalize(Vector2(0.0f, -1.0f));
                    }
                }
                else if (game->ship.beamLevel == 2) {
                    space_bullet *bullet = spawnPlayerBullet(game);
                    if (bullet) {
                        bullet->row = game->ship.row;
                        bullet->col = game->ship.col;
                        bullet->pos = Vector2(shipX, shipY) + Vector2(4.0f, -3.0f);
                        bullet->velocity = 300.0f * normalize(Vector2(0.0f, -1.0f));
                    }
                    bullet = spawnPlayerBullet(game);
                    if (bullet) {
                        bullet->row = game->ship.row;
                        bullet->col = game->ship.col;
                        bullet->pos = Vector2(shipX, shipY) + Vector2(-4.0f, -3.0f);
                        bullet->velocity = 300.0f * normalize(Vector2(0.0f, -1.0f));
                    }
                    bullet = spawnPlayerBullet(game);
                    if (bullet) {
                        bullet->row = game->ship.row;
                        bullet->col = game->ship.col;
                        bullet->pos = Vector2(shipX, shipY) + Vector2(0.0f, -3.0f);
                        bullet->velocity = 300.0f * normalize(Vector2(0.0f, -1.0f));
                    }
                }
            }
        } break;
        case BEAM_TYPE_WAVE: {
            if (game->bulletTimer > 0.2f) {
                game->bulletTimer = 0.0f;

                int numBullets = 3;
                float width = PI / 6.0f;
                switch (game->ship.beamLevel) {
                    case 0: {
                        numBullets = 3;
                        width = PI / 6.0f;
                    } break;
                    case 1: {
                        numBullets = 5;
                        width = PI / 5.0f;
                    } break;
                    case 2: {
                        numBullets = 7;
                        width = PI / 4.0f;
                    } break;
                }

                float widthInc = width / ((float)(numBullets - 1));
                float widthHalf = width / 2.0f;
                float start = -PI/2.0f - widthHalf;

                for (int i = 0; i < numBullets; ++i) {
                    space_bullet *bullet = spawnPlayerBullet(game);
                    if (bullet) {
                        float angle = start + widthInc * i;

                        bullet->row = game->ship.row;
                        bullet->col = game->ship.col;
                        bullet->pos = Vector2(shipX, shipY) + Vector2(0.0f, -3.0f);
                        bullet->velocity = 250.0f * normalize(Vector2(cosf(angle), sinf(angle)));
                    }
                }
            }
        } break;
    }
}

void updateShipView (memory_arena *memory, memory_arena *tempMemory, game_assets *assets, game_sounds *gameSounds, game_input *input, 
                      minesweeper_game *game, sprite_list *spriteList) {
    // move ship
    float shipSpeed = 84.0f;

    if (!game->ship.dead && !game->touchingWormhole && game->currentView == VIEW_MODE_SHIP) {
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

    if (game->ship.y < 0.0f) {
        game->ship.row--;
        game->ship.y += SPACE_SHIP_CELL_DIM;

        if (game->ship.row == -1) {
            game->currentView = VIEW_MODE_FIELD;
            game->state = GAME_STATE_TRANSITION_TO_FINAL_BATTLE;
            game->finalBattleWaitT = 0.0f;
        }
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

    float shipScreenX = (game->ship.col - game->screenCol) * SPACE_SHIP_CELL_DIM + game->ship.x - game->screenX;
    float shipScreenY = (game->ship.row - game->screenRow) * SPACE_SHIP_CELL_DIM + game->ship.y - game->screenY;
    if (shipScreenX < 0.0f) {
        game->ship.col = game->screenCol;
        game->ship.x = game->screenX;
    }
    if (shipScreenX >= 192.0f) {
        game->ship.col = game->screenCol;
        game->ship.x = game->screenX + 192.0f;
    }
    if (shipScreenY < 0.0f) {
        game->ship.row = game->screenRow;
        game->ship.y = game->screenY;
    }
    if (shipScreenY >= 216.0f) {
        game->ship.row = game->screenRow;
        game->ship.y = game->screenY + 216.0f;
    }

    matrix3x3 gameTransform = peekSpriteMatrix(spriteList);
    vector3 localPointerPos = Vector3((float)input->pointerX, (float)input->pointerY, 1.0f);
    localPointerPos = inverse(gameTransform) * localPointerPos;

    bool shooting = false;
    if (game->currentView == VIEW_MODE_SHIP) {
        if (!game->ship.dead && (input->zKey.down || input->spaceKey.down)) {
            shooting = true;
        }
        if (localPointerPos.x >= 0.0f && localPointerPos.x < 192.0f && localPointerPos.y >= 0.0f && localPointerPos.y < 216.0f && input->pointerDown) {
            shooting = true;
        }
    }

    game->bulletTimer += DELTA_TIME;
    if (shooting) {
        spawnBeam(game);
    }
    updatePlayerBulletsDuringShipView(game, assets, gameSounds);

    for (int i = 0; i < FIELD_HEIGHT; ++i) {
        for (int j = 0; j < FIELD_WIDTH; ++j) {
            int index = i * FIELD_WIDTH + j;
            mine_cell *cell = &game->cells[index];
            if (cell->hitTimer > 0.0f) {
                cell->hitTimer -= DELTA_TIME;
                if (cell->hitTimer < 0.0f) {
                    cell->hitTimer = 0.0f;
                }
            }
        }
    }


    if (!game->touchingWormhole) {
        game->ship.y += -21.0f * DELTA_TIME;
    }

    if (!game->ship.dead && game->touchingWormhole) {
        vector2 screenCornerToWormhole = distBetweenRowColXYs(game->screenRow, game->screenCol, game->screenX, game->screenY,
                                                              game->wormholeRow - 1, game->wormholeCol - 1, 0.5f, 0.0f);
        float dist = sqrtf(screenCornerToWormhole.x * screenCornerToWormhole.x + screenCornerToWormhole.y * screenCornerToWormhole.y);
        if (dist < 25.0f) {
            game->state = GAME_STATE_ZOOMING_INTO_BATTLE;
            playSound("zoom_battle", gameSounds, assets);

            vector2 shipToScreen = distBetweenRowColXYs(game->ship.row, game->ship.col, game->ship.x, game->ship.y,
                    game->screenRow, game->screenCol, game->screenX, game->screenY);
            game->ship.battleX = -shipToScreen.x;
            game->ship.battleY = -shipToScreen.y;

            removeAllBullets(game);
        }
        else {
            vector2 direction = normalize(screenCornerToWormhole);

            game->screenX += direction.x * 50.0f * DELTA_TIME;
            game->screenY += direction.y * 50.0f * DELTA_TIME;

            if (game->screenX < 0.0f) {
                game->screenCol--;
                game->screenX += SPACE_SHIP_CELL_DIM;
            }
            if (game->screenX >= SPACE_SHIP_CELL_DIM) {
                game->screenCol++;
                game->screenX -= SPACE_SHIP_CELL_DIM;
            }
            if (game->screenY < 0.0f) {
                game->screenRow--;
                game->screenY += SPACE_SHIP_CELL_DIM;
            }
            if (game->screenY >= SPACE_SHIP_CELL_DIM) {
                game->screenRow++;
                game->screenY -= SPACE_SHIP_CELL_DIM;
            }
        }
    }
    else if (!game->ship.dead && game->suckingIntoWormhole) {
        vector2 distToWormHole = distBetweenRowColXYs(game->ship.row, game->ship.col, game->ship.x, game->ship.y,
                                                      game->wormholeRow, game->wormholeCol, 42.0f, 42.0f);

        float dist = sqrtf(distToWormHole.x * distToWormHole.x + distToWormHole.y * distToWormHole.y);
        float strength = 20000.0f / (dist);
        if (strength > 2500.0f) {
            strength = 2500.0f;
        }
        vector2 direction = normalize(distToWormHole);

            game->ship.x += direction.x * strength * DELTA_TIME;
            game->ship.y += direction.y * strength * DELTA_TIME;
    }
    else {
        game->screenY += -21.0f * DELTA_TIME;
        if (game->screenY < 0.0f) {
            game->screenRow--;
            game->screenY += SPACE_SHIP_CELL_DIM;
        }
    }

    if (!game->touchingWormhole) {
        float movementWindow = 80.0f;
        shipScreenX = (game->ship.col - game->screenCol) * SPACE_SHIP_CELL_DIM + game->ship.x;
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

        // can scroll y if sucking into wormhole
        if (game->suckingIntoWormhole) {
            shipScreenY = (game->ship.row - game->screenRow) * SPACE_SHIP_CELL_DIM + game->ship.y;
            float distToTopEdge = shipScreenY - game->screenY;
            if (distToTopEdge < 120.0f) {
                game->screenY -= 1.0f;
                while (game->screenY < 0.0f) {
                    game->screenRow--;
                    game->screenY += SPACE_SHIP_CELL_DIM;
                }

                if (game->screenRow < 0) {
                    game->screenRow = 0;
                    game->screenY = 0.0f;
                }
            }

            float distToBottomEdge = (game->screenY + 216.0f) - shipScreenY;
            if (distToBottomEdge < 30.0f) {
                game->screenY += 1.0f;
                while (game->screenY >= SPACE_SHIP_CELL_DIM) {
                    game->screenRow++;
                    game->screenY -= SPACE_SHIP_CELL_DIM;
                }
            }

            float screenXToBottomEdge = (FIELD_HEIGHT - game->screenRow) * SPACE_SHIP_CELL_DIM - game->screenY;
            if (screenXToBottomEdge < 216.0f) {
                game->screenRow = FIELD_HEIGHT;
                game->screenY = -216.0f;
                while (game->screenY < 0.0f) {
                    game->screenRow--;
                    game->screenY += SPACE_SHIP_CELL_DIM;
                }
            }

            if (game->screenY < 0.0f) {
                game->screenRow--;
                game->screenY += SPACE_SHIP_CELL_DIM;
            }
            if (game->screenY >= SPACE_SHIP_CELL_DIM) {
                game->screenRow++;
                game->screenY -= SPACE_SHIP_CELL_DIM;
            }
        }

    }

    if (!game->ship.dead) {
        space_ship *ship = &game->ship;
        if (ship->row >= 0 && ship->row < FIELD_HEIGHT &&
            ship->col >= 0 && ship->col < FIELD_WIDTH) 
        {
            mine_cell *currentCell = &game->cells[ship->row * FIELD_WIDTH + ship->col];
            if (currentCell->open && currentCell->hasBomb) {
                game->touchingWormhole = true;
                game->suckingIntoWormhole = false;
            }
            else if (!currentCell->open) {
                game->ship.dead = true;
                game->hitpoints = 0;
                game->deadTimer = 0.0f;

                space_explosion *explosion = activateExplosion(game, assets, gameSounds);
                if (explosion) {
                    if (game->currentView == VIEW_MODE_SHIP) {
                        explosion->pos = Vector2((game->ship.col - game->screenCol) * SPACE_SHIP_CELL_DIM + game->ship.x,
                                                 (game->ship.row - game->screenRow) * SPACE_SHIP_CELL_DIM + game->ship.y);
                        explosion->pos.x -= game->screenX;
                        explosion->pos.y -= game->screenY;
                        explosion->maxScale = 1.5f;
                    }
                    else if (game->currentView == VIEW_MODE_FIELD) {
                        int fieldScreenRow = (game->screenRow + 3) - 18;
                        float fieldScreenY = game->screenY / (float)SPACE_SHIP_ZOOM;
                        if (fieldScreenRow < 0) {
                            fieldScreenRow = 0;
                            fieldScreenY = 0.0f;
                        }
                        explosion->pos = Vector2((float)((game->ship.col)* CELL_DIM + (game->ship.x / ((float)SPACE_SHIP_ZOOM))),
                                                 (float)((game->ship.row - fieldScreenRow)* CELL_DIM + (game->ship.y / ((float)SPACE_SHIP_ZOOM))));
                        explosion->pos.y -= fieldScreenY;
                        explosion->maxScale = 0.5f;
                    }
                    explosion->t = 0.0f;
                }

            }
        }
    }

}

void trySuckingIntoWormhole (minesweeper_game *game) {
    if (!game->suckingIntoWormhole) {
        for (int i = 0; i < FIELD_HEIGHT; ++i) {
            for (int j = 0; j < FIELD_WIDTH; ++j) {
                mine_cell *cell = &game->cells[i * FIELD_WIDTH + j];
                if (cell->open && cell->hasBomb) {
                    int rowDiff = cell->row - game->ship.row;
                    if (rowDiff < 0) {
                        rowDiff = -rowDiff;
                    }

                    int colDiff = cell->col - game->ship.col;
                    if (colDiff < 0) {
                        colDiff = -colDiff;
                    }

                    if (rowDiff <= 2 && colDiff <= 2) {
                        game->suckingIntoWormhole = true;
                        game->wormholeRow = i;
                        game->wormholeCol = j;
                        return;
                    }
                }
            }
        }
    }
    else {
        mine_cell *cell = &game->cells[game->wormholeRow * FIELD_WIDTH + game->wormholeCol];
        int rowDiff = cell->row - game->ship.row;
        if (rowDiff < 0) {
            rowDiff = -rowDiff;
        }

        int colDiff = cell->col - game->ship.col;
        if (colDiff < 0) {
            colDiff = -colDiff;
        }

        if (rowDiff <= 2 && colDiff <= 2) {
        }
        else {
            game->suckingIntoWormhole = false;
        }
    }
}
space_bullet *spawnEnemyBullet (minesweeper_game *game) {
    for (int i = 0; i < MAX_NUM_ENEMY_BULLETS; ++i) {
        space_bullet *bullet = &game->enemyBullets[i];
        if (!bullet->active) {
            entity_id id = bullet->id;
            *bullet = {};
            bullet->active = true;
            bullet->id = id;
            return bullet;
        }
    }
    return 0;
}


void checkPointCellOverlap (bullet_cell_grid *bulletCellGrid, int *numCellsCovered, bullet_cell **potentialCells, vector2 corner) {
    int cellCol = (int)((corner.x) / 16.0f);
    int cellRow = (int)((corner.y) / 16.0f);
    bullet_cell *bulletCell = bulletCellGrid->cells + (cellRow * bulletCellGrid->numCols + cellCol);

    bool cellAlreadyAdded = false;
    for (int cellIndex = 0; cellIndex < *numCellsCovered; ++cellIndex) {
        if (bulletCell == potentialCells[cellIndex]) {
            cellAlreadyAdded = true;
            break;
        }
    }
    
    if (!cellAlreadyAdded) {
        potentialCells[*numCellsCovered] = bulletCell;
        ++(*numCellsCovered);
    }
}


void updatePlayerBullets (minesweeper_game *game, bullet_cell_grid *bulletCellGrid) {
    bullet_cell *potentialCells[4];
    int numCellsCovered = 0;

    for (int i = 0; i < MAX_NUM_ENEMY_BULLETS; ++i) {
        space_bullet *bullet = game->playerBullets + i;
        if (bullet->active) {
            bullet->pos += DELTA_TIME * bullet->velocity;

            bullet->timeAlive += DELTA_TIME;
            bool despawn = false;
            if (bullet->timeAlive > BULLET_LIFETIME) {
                despawn = true;
            }
            else if (bullet->pos.x + BULLET_RADIUS < 0.0f || 
                     bullet->pos.x - BULLET_RADIUS >= 192.0f || 
                     bullet->pos.y + BULLET_RADIUS < 0.0f || 
                     bullet->pos.y - BULLET_RADIUS >= 216.0f) 
            {
                despawn = true;
            }

            if (despawn) {
                despawnPlayerBullet(bullet);
            }
            else {
                // put bullet into grid
                numCellsCovered = 0;

                checkPointCellOverlap(bulletCellGrid, &numCellsCovered, potentialCells, bullet->pos + Vector2(-BULLET_RADIUS, -BULLET_RADIUS));
                checkPointCellOverlap(bulletCellGrid, &numCellsCovered, potentialCells, bullet->pos + Vector2(BULLET_RADIUS, -BULLET_RADIUS));
                checkPointCellOverlap(bulletCellGrid, &numCellsCovered, potentialCells, bullet->pos + Vector2(-BULLET_RADIUS, BULLET_RADIUS));
                checkPointCellOverlap(bulletCellGrid, &numCellsCovered, potentialCells, bullet->pos + Vector2(BULLET_RADIUS, BULLET_RADIUS));

                for (int cellIndex = 0; cellIndex < numCellsCovered; ++cellIndex) {
                    bullet_cell *bulletCell = potentialCells[cellIndex];
                    if (bulletCell->numBullets < NUM_BULLETS_PER_CELL) {
                        bulletCell->bullets[bulletCell->numBullets] = bullet;
                        ++bulletCell->numBullets;
                    }
                }
            }
        }
    }
}


bool checkPlayerBulletCollisions (minesweeper_game *game, bullet_cell_grid *bulletCellGrid) {
    vector2 playerPos = Vector2(game->ship.battleX, game->ship.battleY);

    bullet_cell *overlappedCells[4];
    int numCellsCovered = 0;
    checkPointCellOverlap(bulletCellGrid, &numCellsCovered, overlappedCells, playerPos + Vector2(-PLAYER_RADIUS, -PLAYER_RADIUS));
    checkPointCellOverlap(bulletCellGrid, &numCellsCovered, overlappedCells, playerPos + Vector2(PLAYER_RADIUS, -PLAYER_RADIUS));
    checkPointCellOverlap(bulletCellGrid, &numCellsCovered, overlappedCells, playerPos + Vector2(-PLAYER_RADIUS, PLAYER_RADIUS));
    checkPointCellOverlap(bulletCellGrid, &numCellsCovered, overlappedCells, playerPos + Vector2(PLAYER_RADIUS, PLAYER_RADIUS));

    for (int cellIndex = 0; cellIndex < numCellsCovered; ++cellIndex) {
        bullet_cell *bulletCell = overlappedCells[cellIndex];
        for (int i = 0; i < bulletCell->numBullets; ++i) {
            space_bullet *bullet = bulletCell->bullets[i];
            float dist = sqrtf(square(bullet->pos.x - playerPos.x) + 
                               square(bullet->pos.y - playerPos.y));
            if (dist < BULLET_RADIUS + PLAYER_RADIUS) {
                return true;
            }
        }
    }
    return false;
}

bool checkPlayerTouchedEnemy (minesweeper_game *game) {
    for (int i = 0; i < 30; ++i) {
        space_enemy *enemy = &game->enemies[i];
        if (enemy->active) {
            vector2 playerToEnemy = Vector2(game->ship.battleX, game->ship.battleY) - enemy->pos;
            if (length(playerToEnemy) < (PLAYER_RADIUS + ENEMY_RADIUS)) {
                return true;
            }
        }
    }
    return false;
}

void updateEnemyBullets (minesweeper_game *game, bullet_cell_grid *bulletCellGrid) {
    bullet_cell *potentialCells[4];
    int numCellsCovered = 0;

    for (int i = 0; i < MAX_NUM_ENEMY_BULLETS; ++i) {
        space_bullet *bullet = game->enemyBullets + i;
        if (bullet->active) {
            bullet->pos += DELTA_TIME * bullet->velocity;

            bullet->timeAlive += DELTA_TIME;
            bool despawn = false;
            if (bullet->timeAlive > BULLET_LIFETIME) {
                despawn = true;
            }
            else if (bullet->pos.x + BULLET_RADIUS < 0.0f || 
                     bullet->pos.x - BULLET_RADIUS >= 192.0f || 
                     bullet->pos.y + BULLET_RADIUS < 0.0f || 
                     bullet->pos.y - BULLET_RADIUS >= 216.0f) 
            {
                despawn = true;
            }

            if (despawn) {
                bullet->active = false;
                bullet->id.generation++;
            }
            else {
                // put bullet into grid
                numCellsCovered = 0;

                checkPointCellOverlap(bulletCellGrid, &numCellsCovered, potentialCells, bullet->pos + Vector2(-BULLET_RADIUS, -BULLET_RADIUS));
                checkPointCellOverlap(bulletCellGrid, &numCellsCovered, potentialCells, bullet->pos + Vector2(BULLET_RADIUS, -BULLET_RADIUS));
                checkPointCellOverlap(bulletCellGrid, &numCellsCovered, potentialCells, bullet->pos + Vector2(-BULLET_RADIUS, BULLET_RADIUS));
                checkPointCellOverlap(bulletCellGrid, &numCellsCovered, potentialCells, bullet->pos + Vector2(BULLET_RADIUS, BULLET_RADIUS));

                for (int cellIndex = 0; cellIndex < numCellsCovered; ++cellIndex) {
                    bullet_cell *bulletCell = potentialCells[cellIndex];
                    if (bulletCell->numBullets < NUM_BULLETS_PER_CELL) {
                        bulletCell->bullets[bulletCell->numBullets] = bullet;
                        ++bulletCell->numBullets;
                    }
                }
            }
        }

    }
}


space_enemy *tryGetEnemyByID (minesweeper_game *game, entity_id id) {
    space_enemy *enemy = &game->enemies[id.value];
    if (enemy->active && (id.value == enemy->id.value && id.generation == enemy->id.generation)) {
        return enemy;
    }
    return 0;
}

space_enemy *activateEnemy (minesweeper_game *game) {
    for (int i = 0; i < 30; ++i) {
        space_enemy *enemy = &game->enemies[i];
        if (!enemy->active) {
            entity_id id = enemy->id;
            *enemy = {};
            enemy->active = true;
            enemy->id = id;
            return enemy;
        }
    }

    assert(0);
    return 0;
}

void destroyEnemy (space_enemy *enemy) {
    enemy->active = false;
    enemy->id.generation++;
}


void destroyExplosion (space_explosion *explosion) {
    explosion->active = false;
    explosion->id.generation++;
}

void checkEnemyBulletCollisions (minesweeper_game *game, bullet_cell_grid *bulletCellGrid, game_assets *assets, game_sounds *gameSounds) {
    for (int enemyIndex = 0; enemyIndex < 30; ++enemyIndex) {
        space_enemy *enemy = &game->enemies[enemyIndex];
        if (enemy->active) {
            vector2 enemyPos = enemy->pos;

            bullet_cell *overlappedCells[4];
            int numCellsCovered = 0;
            checkPointCellOverlap(bulletCellGrid, &numCellsCovered, overlappedCells, enemyPos + Vector2(-ENEMY_RADIUS, -ENEMY_RADIUS));
            checkPointCellOverlap(bulletCellGrid, &numCellsCovered, overlappedCells, enemyPos + Vector2(ENEMY_RADIUS, -ENEMY_RADIUS));
            checkPointCellOverlap(bulletCellGrid, &numCellsCovered, overlappedCells, enemyPos + Vector2(-ENEMY_RADIUS, ENEMY_RADIUS));
            checkPointCellOverlap(bulletCellGrid, &numCellsCovered, overlappedCells, enemyPos + Vector2(ENEMY_RADIUS, ENEMY_RADIUS));

            for (int cellIndex = 0; cellIndex < numCellsCovered; ++cellIndex) {
                bullet_cell *bulletCell = overlappedCells[cellIndex];
                for (int i = 0; i < bulletCell->numBullets; ++i) {
                    space_bullet *bullet = bulletCell->bullets[i];
                    float dist = sqrtf(square(bullet->pos.x - enemyPos.x) + 
                                       square(bullet->pos.y - enemyPos.y));
                    if (dist < BULLET_RADIUS + ENEMY_RADIUS) {
                        despawnPlayerBullet(bullet);

                        // damage enemy
                        playSound("impact", gameSounds, assets);
                        enemy->hp -= game->ship.beamPower;
                        enemy->hitTimer = 0.15f;
                        if (enemy->hp <= 0) {
                            destroyEnemy(enemy);
                            game->score += 20;

                            space_explosion *explosion = activateExplosion(game, assets, gameSounds);
                            if (explosion) {
                                explosion->pos = enemy->pos;
                                explosion->t = 0.0f;
                                explosion->maxScale = 1.0f;
                            }
                        }
                    }
                }
            }
        }
    }

}

void cleanUpEnemyPattern (minesweeper_game *game) {
    for (int i = 0; i < game->enemyPattern.enemyIDs.numValues; ++i) {
        space_enemy *enemy = tryGetEnemyByID(game, game->enemyPattern.enemyIDs.values[i]);
        if (enemy) {
            destroyEnemy(enemy);
        }
    }
    game->enemyPattern.enemyIDs.numValues = 0;
}

void activateNumEnemies (int num, int type, minesweeper_game *game, enemy_pattern *pattern) {
    for (int i = 0; i < num; ++i) {
        space_enemy *enemy = activateEnemy(game);
        enemy->enemyType = type;
        switch (type) {
            default:
            case 1: {
                enemy->hp = 90;
            } break;
            case 2: {
                enemy->hp = 90;
            } break;
            case 3: {
                enemy->hp = 90;
            } break;
        }
        listPush(&pattern->enemyIDs, enemy->id);
    }
}

void initNewEnemyPattern (minesweeper_game *game) {
    cleanUpEnemyPattern(game);

    enemy_pattern *pattern = &game->enemyPattern;
    pattern->t = 0.0f;
    pattern->complete = false;
    if (game->finalBattle) {
        pattern->type = ENEMY_PATTERN_TYPE_FINAL;
    }
    else {
        pattern->type = (enemy_pattern_type)(randomUint() % ENEMY_PATTERN_TYPE_COUNT);
    }
    pattern->enemyIDs.numValues = 0;

    switch (pattern->type) {
        default:
        case ENEMY_PATTERN_TYPE_BASIC: {
            activateNumEnemies(7, 1, game, pattern);
            pattern->minBulletSpawnTime = 0.125f;
            pattern->maxBulletSpawnTime = 0.75f;
        } break;
        case ENEMY_PATTERN_TYPE_BASIC_LEFT: {
            activateNumEnemies(7, 1, game, pattern);
            pattern->minBulletSpawnTime = 0.125f;
            pattern->maxBulletSpawnTime = 0.75f;
        } break;
        case ENEMY_PATTERN_TYPE_FINAL: {
            activateNumEnemies(10, 1, game, pattern);
            activateNumEnemies(10, 2, game, pattern);
            activateNumEnemies(10, 3, game, pattern);
            pattern->minBulletSpawnTime = 0.025f;
            pattern->maxBulletSpawnTime = 0.125f;
        } break;
        case ENEMY_PATTERN_TYPE_SWEEP_DOWN: {
            activateNumEnemies(7, 1, game, pattern);
            pattern->minBulletSpawnTime = 0.125f;
            pattern->maxBulletSpawnTime = 0.75f;

            for (int i = 0; i < pattern->enemyIDs.numValues; ++i) {
                space_enemy *enemy = tryGetEnemyByID(game, pattern->enemyIDs.values[i]);
                if (enemy) {
                    enemy->pos.x = 30.0f + randomFloat() * 132.0f;
                }
                pattern->enemyTs[i] = -0.66f * (float)i;
            }
        } break;
        case ENEMY_PATTERN_TYPE_SPIN: {
            activateNumEnemies(10, 2, game, pattern);
            pattern->minBulletSpawnTime = 0.125f;
            pattern->maxBulletSpawnTime = 0.75f;

            float tDiff = 3.5f / (10.0f);

            for (int i = 0; i < pattern->enemyIDs.numValues; ++i) {
                pattern->enemyTs[i] = 0.0f + -tDiff * (float)i;
                pattern->enemySpinPhases[i] = 0;
            }
        } break;
        case ENEMY_PATTERN_TYPE_SPACE_INVADERS: {
            activateNumEnemies(15, 3, game, pattern);
            pattern->minBulletSpawnTime = 0.125f;
            pattern->maxBulletSpawnTime = 0.75f;

            for (int i = 0; i < pattern->enemyIDs.numValues; ++i) {
                space_enemy *enemy = tryGetEnemyByID(game, pattern->enemyIDs.values[i]);
                if (enemy) {
                    enemy->pos.x = -30.0f + -20.0f * float(i % 5);
                    enemy->pos.y = 10.0f + 20.0f * float(i / 5);
                }
            }
        } break;
    }

    game->enemyBulletT = 0.0f;
    float randomT = randomFloat();
    game->nextEnemyBulletTime = (1.0f - randomT) * game->enemyPattern.minBulletSpawnTime + 
                                randomT * game->enemyPattern.maxBulletSpawnTime;
}

void updateBasicPattern (minesweeper_game *game, enemy_pattern *pattern) {
    float startAngle = 0.0f;
    float endAngle = PI / 2.0f + PI / 4.0f;
    float angleDiff = endAngle - startAngle;
    float enemySpacing = PI / 16.0f;
    float duration = 4.0f;

    for (int i = 0; i < pattern->enemyIDs.numValues; ++i) {
        space_enemy *enemy = tryGetEnemyByID(game, pattern->enemyIDs.values[i]);
        if (enemy) {
            float t = (pattern->t / duration) * angleDiff - (float)i * enemySpacing;
            enemy->pos.x = 40.0f + 150.0f * cosf(t);
            enemy->pos.y = -30.0f + 150.0f * sinf(t);
        }
    }

    if (pattern->t >= 6.0f) {
        pattern->complete = true;
    }
}

void updateBasicLeftPattern (minesweeper_game *game, enemy_pattern *pattern) {
    float startAngle = 0.0f;
    float endAngle = PI / 2.0f + PI / 4.0f;
    float angleDiff = endAngle - startAngle;
    float enemySpacing = PI / 16.0f;
    float duration = 4.0f;

    for (int i = 0; i < pattern->enemyIDs.numValues; ++i) {
        space_enemy *enemy = tryGetEnemyByID(game, pattern->enemyIDs.values[i]);
        if (enemy) {
            float t = (pattern->t / duration) * angleDiff - (float)i * enemySpacing;
            enemy->pos.x = 192.0f - (40.0f + 150.0f * cosf(t));
            enemy->pos.y = -30.0f + 150.0f * sinf(t);
        }
    }

    if (pattern->t >= 6.0f) {
        pattern->complete = true;
    }
}

void updateSweepDownPattern (minesweeper_game *game, enemy_pattern *pattern) {
    float highestEnemyT = 9999999.0f;
    bool activeEnemies = false;

    for (int i = 0; i < pattern->enemyIDs.numValues; ++i) {
        space_enemy *enemy = tryGetEnemyByID(game, pattern->enemyIDs.values[i]);
        if (enemy) {
            activeEnemies = true;
            pattern->enemyTs[i] += DELTA_TIME;
            float t = pattern->enemyTs[i];
            if (t > 0.0f) {
                if (t < 0.75f) {
                    float blendT = t / 0.75f;
                    enemy->pos.y = (1.0f - blendT) * -30.0f + blendT * 80.0f;
                }
                if (t > 2.0f) {
                    float blendT = (t - 2.0f) / 1.75f;
                    enemy->pos.y = (1.0f - blendT) * 80.0f + blendT * 246.0f;
                }
                if (t > 3.75f) {
                    enemy->pos.y = 246.0f;
                }

                if (t < highestEnemyT) {
                    highestEnemyT = t;
                }
            }
            else {
                enemy->pos.y = -30.0f;
            }
        }
    }

    if (highestEnemyT >= 4.0f) {
        pattern->complete = true;
    }
    if (!activeEnemies) {
        pattern->complete = true;
    }
}

void updateSpinPattern(minesweeper_game *game, enemy_pattern *pattern) {
    float highestEnemyT = 9999999.0f;
    bool activeEnemies = false;
    int numSpins = 2;
    float totalTime = (float)numSpins * 3.5f;
    bool allEnemiesDone = true;

    for (int i = 0; i < pattern->enemyIDs.numValues; ++i) {
        space_enemy *enemy = tryGetEnemyByID(game, pattern->enemyIDs.values[i]);
        if (enemy) {
            activeEnemies = true;
            pattern->enemyTs[i] += DELTA_TIME;

            if (pattern->enemySpinPhases[i] == 0) {
                if (pattern->enemyTs[i] < 1.75f) {
                    float t = pattern->enemyTs[i] / 1.75f;
                    enemy->pos.x = (1.0f - t) * -150.0f + t * 96.0f;
                    enemy->pos.y = 20.0f;
                }
                if (pattern->enemyTs[i] >= 1.75f) {
                    pattern->enemyTs[i] -= 1.75f;
                    pattern->enemySpinPhases[i] = 1;
                }
                allEnemiesDone = false;
            }

            if (pattern->enemySpinPhases[i] == 1) {
                if (pattern->enemyTs[i] < totalTime) {
                    enemy->pos.x = 96.0f + 60.0f * sinf((1.0f / 3.5f) * pattern->enemyTs[i] * (2.0f * PI));
                    enemy->pos.y = 80.0f + 60.0f * cosf((1.0f / 3.5f) * pattern->enemyTs[i] * (2.0f * PI) + PI);
                }
                if (pattern->enemyTs[i] >= totalTime) {
                    pattern->enemyTs[i] -= totalTime;
                    pattern->enemySpinPhases[i] = 2;
                }
                allEnemiesDone = false;
            }

            if (pattern->enemySpinPhases[i] == 2) {
                if (pattern->enemyTs[i] < 1.75f) {
                    float t = pattern->enemyTs[i] / 1.75f;
                    enemy->pos.x = (1.0f - t) * 96.0f + t * (192.0f + 150.0f);
                    enemy->pos.y = 20.0f;
                    allEnemiesDone = false;
                }
                else {
                    enemy->pos.x = 96.0f + (192.0f + 150.0f);
                }
            }
        }
    }
    
    if (allEnemiesDone) {
        pattern->complete = true;
    }
    if (!activeEnemies) {
        pattern->complete = true;
    }
}

void updateSpaceInvadersPattern(minesweeper_game *game, enemy_pattern *pattern) {
    int numActive = 0;
    for (int i = 0; i < pattern->enemyIDs.numValues; ++i) {
        space_enemy *enemy = tryGetEnemyByID(game, pattern->enemyIDs.values[i]);
        if (enemy) {
            ++numActive;
        }
    }

    float speed = 40.0f + (float(15 - numActive) / 15.0f) * 100.0f;
    bool activeEnemies = false;

    switch (pattern->movingDir) {
        case SPACE_INVADERS_MOVING_DIR_RIGHT: {
            space_enemy *rightmostEnemy = 0;
            for (int i = 0; i < pattern->enemyIDs.numValues; ++i) {
                space_enemy *enemy = tryGetEnemyByID(game, pattern->enemyIDs.values[i]);
                if (enemy) {
                    activeEnemies = true;
                    enemy->pos.x += speed * DELTA_TIME;
                    if (rightmostEnemy == 0 || enemy->pos.x > rightmostEnemy->pos.x) {
                        rightmostEnemy = enemy;
                    }
                }
            }
            if (rightmostEnemy && rightmostEnemy->pos.x > 177.0f) {
                pattern->movingDir = SPACE_INVADERS_MOVING_DIR_DOWN;
                pattern->t = 0.0f;
                pattern->lastDir = SPACE_INVADERS_MOVING_DIR_RIGHT;
            }
        } break;
        case SPACE_INVADERS_MOVING_DIR_LEFT: {
            space_enemy *leftmostEnemy = 0;
            for (int i = 0; i < pattern->enemyIDs.numValues; ++i) {
                space_enemy *enemy = tryGetEnemyByID(game, pattern->enemyIDs.values[i]);
                if (enemy) {
                    activeEnemies = true;
                    enemy->pos.x += -speed * DELTA_TIME;
                    if (leftmostEnemy == 0 || enemy->pos.x < leftmostEnemy->pos.x) {
                        leftmostEnemy = enemy;
                    }
                }
            }
            if (leftmostEnemy && leftmostEnemy->pos.x < 15.0f) {
                pattern->movingDir = SPACE_INVADERS_MOVING_DIR_DOWN;
                pattern->t = 0.0f;
                pattern->lastDir = SPACE_INVADERS_MOVING_DIR_LEFT;
            }
        } break;
        case SPACE_INVADERS_MOVING_DIR_DOWN: {
            float highestY = 9999999.0f;
            pattern->t += DELTA_TIME;
            for (int i = 0; i < pattern->enemyIDs.numValues; ++i) {
                space_enemy *enemy = tryGetEnemyByID(game, pattern->enemyIDs.values[i]);
                if (enemy) {
                    activeEnemies = true;
                    enemy->pos.y += speed * DELTA_TIME;
                    if (enemy->pos.y < highestY) {
                        highestY = enemy->pos.y;
                    }
                }
            }
            if (highestY > 246.0f) {
                pattern->complete = true;
            }
            if (pattern->t >= 0.5f) {
                pattern->t = 0.0f;
                if (pattern->lastDir == SPACE_INVADERS_MOVING_DIR_RIGHT) {
                    pattern->movingDir = SPACE_INVADERS_MOVING_DIR_LEFT;
                }
                if (pattern->lastDir == SPACE_INVADERS_MOVING_DIR_LEFT) {
                    pattern->movingDir = SPACE_INVADERS_MOVING_DIR_RIGHT;
                }

            }


        } break;
    }

    if (!activeEnemies) {
        pattern->complete = true;
    }
}

void updateFinalPattern (minesweeper_game *game, enemy_pattern *pattern) {
    bool activeEnemies = false;
    for (int i = 0; i < 10; ++i) {
        space_enemy *enemy = tryGetEnemyByID(game, pattern->enemyIDs.values[i]);
        if (enemy) {
            activeEnemies = true;
            enemy->pos.x = -30.0f + 252.0f * (pattern->t / 10.0f) - float(i) * 30.0f;
            enemy->pos.y = 20.0f;
        }
    }
    for (int i = 10; i < 20; ++i) {
        space_enemy *enemy = tryGetEnemyByID(game, pattern->enemyIDs.values[i]);
        if (enemy) {
            activeEnemies = true;
            enemy->pos.x = 222.0f - 252.0f * (pattern->t / 10.0f) + float(i - 10) * 30.0f;
            enemy->pos.y = 45.0f;
        }
    }
    for (int i = 20; i < 30; ++i) {
        space_enemy *enemy = tryGetEnemyByID(game, pattern->enemyIDs.values[i]);
        if (enemy) {
            activeEnemies = true;
            enemy->pos.x = -30.0f + 252.0f * (pattern->t / 10.0f) - float(i - 20) * 30.0f;
            enemy->pos.y = 70.0f;
        }
    }
    if (!activeEnemies && pattern->t < 19.0f) {
        pattern->t = 19.0f;
    }

    if (pattern->t >= 21.0f) {
        pattern->complete = true;
    }
}

void updateEnemyPattern (minesweeper_game *game) {
    enemy_pattern *pattern = &game->enemyPattern;
    pattern->t += DELTA_TIME;

    switch (pattern->type) {
        default:
        case ENEMY_PATTERN_TYPE_BASIC: {
            updateBasicPattern(game, pattern);
        } break;
        case ENEMY_PATTERN_TYPE_BASIC_LEFT: {
            updateBasicLeftPattern(game, pattern);
        } break;
        case ENEMY_PATTERN_TYPE_FINAL: {
            updateFinalPattern(game, pattern);
        } break;
        case ENEMY_PATTERN_TYPE_SWEEP_DOWN: {
            updateSweepDownPattern(game, pattern);
        } break;
        case ENEMY_PATTERN_TYPE_SPIN: {
            updateSpinPattern(game, pattern);
        } break;
        case ENEMY_PATTERN_TYPE_SPACE_INVADERS: {
            updateSpaceInvadersPattern(game, pattern);
        } break;
    }

    if (pattern->complete) {
        game->waveNum++;
    }
}


void updateBattle (memory_arena *memory, memory_arena *tempMemory, game_assets *assets, game_sounds *gameSounds, game_input *input, 
                   minesweeper_game *game, sprite_list *spriteList) 
{
    game->battleTimer += DELTA_TIME;

    float shipSpeed = 84.0f;
    game->starT += DELTA_TIME;
    if (!game->ship.dead) {
        if (input->leftKey.down || input->aKey.down) {
            game->ship.battleX -= shipSpeed * DELTA_TIME;
        }
        if (input->rightKey.down || input->dKey.down) {
            game->ship.battleX += shipSpeed * DELTA_TIME;
        }
        if (input->upKey.down || input->wKey.down) {
            game->ship.battleY -= shipSpeed * DELTA_TIME;
        }
        if (input->downKey.down || input->sKey.down) {
            game->ship.battleY += shipSpeed * DELTA_TIME;
        }
    }

    if (game->ship.battleX < 0.0f) {
        game->ship.battleX = 0.0f;
    }
    if (game->ship.battleX >= 192.0f) {
        game->ship.battleX = 192.0f;
    }
    if (game->ship.battleY < 0.0f) {
        game->ship.battleY = 0.0f;
    }
    if (game->ship.battleY >= 216.0f) {
        game->ship.battleY = 216.0f;
    }

    bullet_cell_grid enemyBulletGrid = {};
    enemyBulletGrid.numRows = 14;
    enemyBulletGrid.numCols = 12;
    enemyBulletGrid.cells = (bullet_cell *)allocateMemorySize(tempMemory, enemyBulletGrid.numRows * enemyBulletGrid.numCols * sizeof(bullet_cell));

    for (int i = 0; i < enemyBulletGrid.numRows; ++i) {
        for (int j = 0; j < enemyBulletGrid.numCols; ++j) {
            bullet_cell *bulletCell = enemyBulletGrid.cells + (i * enemyBulletGrid.numCols + j);
            *bulletCell = {};
        }
    }

    bullet_cell_grid playerBulletGrid = {};
    playerBulletGrid.numRows = 14;
    playerBulletGrid.numCols = 12;
    playerBulletGrid.cells = (bullet_cell *)allocateMemorySize(tempMemory, playerBulletGrid.numRows * playerBulletGrid.numCols * sizeof(bullet_cell));

    for (int i = 0; i < playerBulletGrid.numRows; ++i) {
        for (int j = 0; j < playerBulletGrid.numCols; ++j) {
            bullet_cell *bulletCell = playerBulletGrid.cells + (i * playerBulletGrid.numCols + j);
            *bulletCell = {};
        }
    }

    if (!game->ship.dead) {
        bool shooting = false;
        if (input->zKey.down || input->spaceKey.down) {
            shooting = true;
        }

        matrix3x3 gameTransform = peekSpriteMatrix(spriteList);
        vector3 localPointerPos = Vector3((float)input->pointerX, (float)input->pointerY, 1.0f);
        localPointerPos = inverse(gameTransform) * localPointerPos;

        if (localPointerPos.x >= 0.0f && localPointerPos.x < 192.0f && localPointerPos.y >= 0.0f && localPointerPos.y < 216.0f && input->pointerDown) {
            shooting = true;
        }

        game->bulletTimer += DELTA_TIME;
        if (shooting) {
            spawnBeam(game);
        }
    }


    if (game->enemyPattern.type == ENEMY_PATTERN_TYPE_NONE || game->enemyPattern.complete) {
        initNewEnemyPattern(game);
    }
    updateEnemyPattern(game);

    for (int i = 0; i < 30; ++i) {
        space_enemy *enemy = &game->enemies[i];
        if (enemy->active) {
            if (enemy->hitTimer > 0.0f) {
                enemy->hitTimer -= DELTA_TIME;
                if (enemy->hitTimer < 0.0f) {
                    enemy->hitTimer = 0.0f;
                }
            }
        }
    }

    space_enemy_ptr_list activeEnemies = space_enemy_ptrListInit(tempMemory, 30);
    for (int i = 0; i < 30; ++i) {
        space_enemy *enemy = &game->enemies[i];
        if (enemy->active) {
            listPush(&activeEnemies, enemy);
        }
    }
    for (int i = activeEnemies.numValues - 1; i >= 0; --i) {
        int randomIndex = randomUint() % (activeEnemies.numValues);
        space_enemy *randomEnemy = activeEnemies.values[randomIndex];
    
        space_enemy *temp = activeEnemies.values[i];
        activeEnemies.values[i] = randomEnemy;
        activeEnemies.values[randomIndex] = temp;
    }

    game->enemyBulletT += DELTA_TIME;
    if (game->enemyBulletT >= game->nextEnemyBulletTime) {
        for (int i = 0; i < activeEnemies.numValues; ++i) {
            space_enemy *enemy = activeEnemies.values[i];
            if (enemy->pos.x > 0 && enemy->pos.x < 192.0f && enemy->pos.y > 0 && enemy->pos.y < 216.0f) {
                space_bullet *bullet = spawnEnemyBullet(game);
                if (bullet) {
                    bullet->pos = Vector2(enemy->pos.x, enemy->pos.y) + Vector2(0.0f, 3.0f);
                    bullet->velocity = 150.0f * normalize(Vector2(0.0f, 1.0f));
                }

                game->enemyBulletT = 0.0f;
                float randomT = randomFloat();
                game->nextEnemyBulletTime = (1.0f - randomT) * game->enemyPattern.minBulletSpawnTime + 
                                            randomT * game->enemyPattern.maxBulletSpawnTime;
                break;
            }
        }
    }

    updatePlayerBullets(game, &playerBulletGrid);
    updateEnemyBullets(game, &enemyBulletGrid);

    space_ship *ship = &game->ship;
    if (!ship->hurt && !ship->dead) {
        bool playerHitByBullet = checkPlayerBulletCollisions(game, &enemyBulletGrid);
        bool playerTouchedEnemy = checkPlayerTouchedEnemy(game);
        if (playerHitByBullet || playerTouchedEnemy) {
            ship->hurt = true;

            if (ship->hasShield) {
                ship->hasShield = false;
            }
            else {
                --game->hitpoints;
            }
            playSound("got_hurt", gameSounds, assets);

            if (game->hitpoints <= 0) {
                game->ship.dead = true;
                game->deadTimer = 0.0f;

                space_explosion *explosion = activateExplosion(game, assets, gameSounds);
                if (explosion) {
                    explosion->pos = Vector2(game->ship.battleX, game->ship.battleY);
                    explosion->t = 0.0f;
                    explosion->maxScale = 1.5f;
                }

            }
        }
    }

    if (ship->hurt) {
        ship->invincibilityTimer += DELTA_TIME;
        if (ship->invincibilityTimer >= PLAYER_INVINCIBILITY_TIMER) {
            ship->hurt = false;
            ship->invincibilityTimer = 0.0f;
        }
    }

    checkEnemyBulletCollisions(game, &playerBulletGrid, assets, gameSounds);

    if (game->finalBattle && game->waveNum == 1 && !game->ship.dead) {
        game->state = GAME_STATE_WIN_SCREEN;
    }
    if (game->waveNum >= 4 && !game->ship.dead) {
        game->currentView = VIEW_MODE_FIELD;
        game->state = GAME_STATE_ZOOMING_OUT_OF_BATTLE;
        playSound("zoom_in", gameSounds, assets);
        removeAllBullets(game);
    }
}


void revealCellsAfterWormhole (memory_arena *memory, memory_arena *tempMemory, game_assets *assets, game_input *input, 
                               minesweeper_game *game, sprite_list *spriteList) 
{
    game->revealT += DELTA_TIME;
    if (game->revealT >= 0.125f) {
        game->revealT -= 0.125f;

        mine_cell *cell = game->revealingCells.values[0];
        listSplice(&game->revealingCells, 0);

        if (!cell->open) {
            if (cell->hasBomb) {
                cell->flagged = true;
            }
            else {
                cell->open = true;
                game->score += 10;
                if (cell->numAdjBombs == 0) {
                    openAllNearbyZeroValueCells(cell, tempMemory, game, true);
                }
            }
        }

        if (game->revealingCells.numValues == 0) {
            game->state = GAME_STATE_NORMAL;
        }
    }
}

void updateMinesweeperGame (memory_arena *memory, memory_arena *tempMemory, game_assets *assets, game_sounds *gameSounds, game_input *input, 
                      minesweeper_game *game, sprite_list *spriteList)
{
    switch (game->state) {
        default: {
            // do nothing
        } break;
        case GAME_STATE_TITLE_SCREEN: {
            if (input->pointerJustDown) {
                game->state = GAME_STATE_NORMAL;
                playSound("start_sound", gameSounds, assets);
            }
        } break;
        case GAME_STATE_GAME_OVER_SCREEN: {
            if (input->pointerJustDown) {
                *game = {};
                initMinesweeperGame(memory, tempMemory, game);
                game->state = GAME_STATE_NORMAL;
            }
        } break;
        case GAME_STATE_WIN_SCREEN: {
            if (input->pointerJustDown) {
                *game = {};
                initMinesweeperGame(memory, tempMemory, game);
                game->state = GAME_STATE_NORMAL;
            }
        } break;
        case GAME_STATE_NORMAL: {
            updateFieldView(memory, tempMemory, assets, gameSounds, input, game, spriteList);
            trySwitchViews(memory, tempMemory, assets, gameSounds, input, game, spriteList);
            if (!game->touchingWormhole) {
                trySuckingIntoWormhole(game);
            }
            updateShipView(memory, tempMemory, assets, gameSounds, input, game, spriteList);
        } break;
        case GAME_STATE_TRANSITION_TO_FINAL_BATTLE: {
            game->finalBattleWaitT += DELTA_TIME;
            if (game->finalBattleWaitT > 2.0f) {
                game->state = GAME_STATE_ZOOMING_INTO_BATTLE;
                playSound("zoom_battle", gameSounds, assets);
                game->finalBattle = true;

                vector2 shipToScreen = distBetweenRowColXYs(game->ship.row, game->ship.col, game->ship.x, game->ship.y,
                        game->screenRow, game->screenCol, game->screenX, game->screenY);
                game->ship.battleX = -shipToScreen.x;
                game->ship.battleY = -shipToScreen.y;

                removeAllBullets(game);
            }
        } break;
        case GAME_STATE_ZOOMING_IN: {
            zoomIntoShip(game);
        } break;
        case GAME_STATE_ZOOMING_INTO_BATTLE: {
            zoomIntoBattle(game);
        } break;
        case GAME_STATE_ZOOMING_OUT_OF_BATTLE: {
            zoomOutOfBattle(game);
        } break;
        case GAME_STATE_ZOOMING_OUT: {
            zoomOutOfShip(game);
        } break;
        case GAME_STATE_BATTLE: {
            updateBattle(memory, tempMemory, assets, gameSounds, input, game, spriteList);
        } break;
        case GAME_STATE_REVEALING_CELLS: {
            revealCellsAfterWormhole(memory, tempMemory, assets, input, game, spriteList);
        } break;
    }

    if (game->ship.dead) {
        game->deadTimer += DELTA_TIME;
        if (game->deadTimer >= 2.0f) {
            game->lives--;

            if (game->lives <= 0) {
                game->state = GAME_STATE_GAME_OVER_SCREEN;
            }
            else {

                removeAllBullets(game);

                // TODO(ebuchholz): game over
                game->currentView = VIEW_MODE_FIELD;
                game->state = GAME_STATE_NORMAL;

                game->zoomT = 0.0f;
                game->finalBattle = false;

                game->screenRow = FIELD_HEIGHT - 3;
                game->screenCol = (FIELD_WIDTH / 2) - 1;
                game->screenX = 42.0f;
                game->screenY = 42.0f;

                game->ship.row = FIELD_HEIGHT -1;
                game->ship.col = FIELD_WIDTH / 2;
                game->ship.x = 42.0f;
                game->ship.y = 42.0f;

                game->ship.dead = false;
                game->ship.beamLevel = 0;
                game->ship.beamType = BEAM_TYPE_NORMAL;
                game->ship.beamPower = 30;
                game->hitpoints = 3;
                game->suckingIntoWormhole = false;
                game->touchingWormhole = false;

                cleanUpEnemyPattern(game);
                game->enemyPattern.type = ENEMY_PATTERN_TYPE_NONE;
            }
        }
    }

    // update explosions
    for (int i = 0; i < MAX_EXPLOSIONS; ++i) {
        space_explosion *explosion = &game->explosions[i];
        if (explosion->active) {
            explosion->t += DELTA_TIME;
            if (explosion->t >= 0.25f) {
                explosion->active = false;
                explosion->id.generation++;
            }
        }
    }

    // update hands
    game->leftHandState = HAND_STATE_MIDDLE;
    game->rightHandState = HAND_STATE_MIDDLE;

    matrix3x3 gameTransform = peekSpriteMatrix(spriteList);
    vector3 localPointerPos = Vector3((float)input->pointerX, (float)input->pointerY, 1.0f);
    localPointerPos = inverse(gameTransform) * localPointerPos;
    if (input->leftKey.down || input->aKey.down) {
        game->leftHandState = HAND_STATE_LEFT;
    }
    if (input->rightKey.down || input->dKey.down) {
        game->leftHandState = HAND_STATE_RIGHT;
    }
    if (localPointerPos.x < 64.0f) {
        game->rightHandState = HAND_STATE_LEFT;
    }
    else if (localPointerPos.x >= 128.0f) {
        game->rightHandState = HAND_STATE_RIGHT;
    }
}

void drawMinesweeperGame (memory_arena *memory, memory_arena *tempMemory, game_assets *assets, 
                    minesweeper_game *game, sprite_list *spriteList) 
{
    // memory for dynamically created strings
    memory_arena stringMemory = {};
    stringMemory.capacity = 512 * 1024;
    stringMemory.base = allocateMemorySize(tempMemory, stringMemory.capacity);
    
    if (game->state == GAME_STATE_TITLE_SCREEN) {
        addText(100.0f, 63.0f, "SWEEPER'S GAME", assets, "font", spriteList);
        addText(100.0f, 93.0f, "CLICK TO PLAY", assets, "font", spriteList);
    }
    else if (game->state == GAME_STATE_GAME_OVER_SCREEN) {
        addText(105.0f, 53.0f, "GAME OVER", assets, "font", spriteList);
        addText(120.0f, 73.0f, "SCORE", assets, "font", spriteList);

        int digits = 0;
        int tempScore = game->score;
        while (tempScore > 0) {
            tempScore /= 10;
            ++digits;
        }
        float shift = (float)(-digits) * 4.0f;

        addText(135.0f + shift, 93.0f, numToString(game->score, &stringMemory), assets, "font", spriteList);
        addText(60.0f, 113.0f, "CLICK TO PLAY AGAIN", assets, "font", spriteList);
    }
    else if (game->state == GAME_STATE_WIN_SCREEN) {
        addText(113.0f, 53.0f, "YOU WIN!", assets, "font", spriteList);
        addText(120.0f, 73.0f, "SCORE", assets, "font", spriteList);

        int digits = 0;
        int tempScore = game->score;
        while (tempScore > 0) {
            tempScore /= 10;
            ++digits;
        }
        float shift = (float)(-digits) * 4.0f;

        addText(135.0f + shift, 93.0f, numToString(game->score, &stringMemory), assets, "font", spriteList);
        addText(60.0f, 113.0f, "CLICK TO PLAY AGAIN", assets, "font", spriteList);
    }
    else {
        if (game->currentView == VIEW_MODE_FIELD) {
            int fieldScreenRow = (game->screenRow + 3) - 18;
            float fieldScreenY = game->screenY / (float)SPACE_SHIP_ZOOM;
            if (fieldScreenRow < 0) {
                fieldScreenRow = 0;
                fieldScreenY = 0.0f;
            }

            pushSpriteTransform(spriteList, Vector2(0.0f, -(fieldScreenRow * CELL_DIM + fieldScreenY)));

            if (game->state == GAME_STATE_ZOOMING_IN || game->state == GAME_STATE_ZOOMING_INTO_BATTLE) {
                float scaleAmount = 1.0f + 4.0f * (game->zoomT / 0.5f);

                vector2 target = Vector2();
                target.x = (float)(game->ship.col * CELL_DIM + (game->ship.x / ((float)SPACE_SHIP_ZOOM)));
                target.y = (float)(game->ship.row * CELL_DIM + (game->ship.y / ((float)SPACE_SHIP_ZOOM)));

                matrix3x3 posMatrix;
                posMatrix = translationMatrix(target.x, target.y);
                pushSpriteMatrix(posMatrix, spriteList);

                matrix3x3 scaleTransform = scaleMatrix3x3(scaleAmount, scaleAmount);
                pushSpriteMatrix(scaleTransform, spriteList);

                posMatrix = translationMatrix(-target.x, -target.y);
                pushSpriteMatrix(posMatrix, spriteList);
            }
            else if (game->state == GAME_STATE_ZOOMING_OUT) {
                float scaleAmount = 1.0f + 4.0f * ((0.5f - game->zoomT) / 0.5f);

                vector2 target = Vector2();
                target.x = (float)(game->ship.col * CELL_DIM + (game->ship.x / ((float)SPACE_SHIP_ZOOM)));
                target.y = (float)(game->ship.row * CELL_DIM + (game->ship.y / ((float)SPACE_SHIP_ZOOM)));

                matrix3x3 posMatrix;
                posMatrix = translationMatrix(target.x, target.y);
                pushSpriteMatrix(posMatrix, spriteList);

                matrix3x3 scaleTransform = scaleMatrix3x3(scaleAmount, scaleAmount);
                pushSpriteMatrix(scaleTransform, spriteList);

                posMatrix = translationMatrix(-target.x, -target.y);
                pushSpriteMatrix(posMatrix, spriteList);
            }
            else if (game->state == GAME_STATE_ZOOMING_OUT_OF_BATTLE) {
                float scaleAmount = 1.0f + 4.0f * ((0.5f - game->zoomT) / 0.5f);

                vector2 target = Vector2();
                target.x = (float)(game->ship.col * CELL_DIM + (game->ship.x / ((float)SPACE_SHIP_ZOOM)));
                target.y = (float)(game->ship.row * CELL_DIM + (game->ship.y / ((float)SPACE_SHIP_ZOOM)));

                matrix3x3 posMatrix;
                posMatrix = translationMatrix(target.x, target.y);
                pushSpriteMatrix(posMatrix, spriteList);

                matrix3x3 scaleTransform = scaleMatrix3x3(scaleAmount, scaleAmount);
                pushSpriteMatrix(scaleTransform, spriteList);

                posMatrix = translationMatrix(-target.x, -target.y);
                pushSpriteMatrix(posMatrix, spriteList);
            }

            static float shiftPickupTimer = 0.0f;
            shiftPickupTimer += DELTA_TIME;
            if (shiftPickupTimer >= 1.0f) {
                shiftPickupTimer -= 1.0f;
            }
            float shiftAmount = shiftPickupTimer < 0.5f ? 0.0f : 1.0f;

            for (int i = 0; i < FIELD_HEIGHT; ++i) {
                for (int j = 0; j < FIELD_WIDTH; ++j) {
                    mine_cell *cell = &game->cells[i * FIELD_WIDTH + j];
                    cell->row = i;
                    cell->col = j;

                    if (!cell->open) {
                        sprite cellSprite = Sprite();
                        cellSprite.pos.x = (float)(cell->col * CELL_DIM);
                        cellSprite.pos.y = (float)(cell->row * CELL_DIM);

                        if (cell->cracked) {
                            if (cell->hasBomb) {
                                addSprite(cellSprite, assets, "atlas", "bomb", spriteList);
                            }
                            else {
                                if (cell->numAdjBombs > 0) {
                                    addSprite(cellSprite, assets, "atlas", numToString(cell->numAdjBombs, &stringMemory), spriteList);
                                }
                            }
                            addSprite(cellSprite, assets, "atlas", "cracked_cell", spriteList);
                        }
                        else {
                            addSprite(cellSprite, assets, "atlas", "closed_cell", spriteList);
                        }



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
                            if (cell->hasSPower) {
                                sprite pickupSprite = Sprite();
                                pickupSprite.pos.x = (float)(cell->col * CELL_DIM);
                                pickupSprite.pos.y = (float)(cell->row * CELL_DIM) + shiftAmount;
                                addSprite(pickupSprite, assets, "atlas", "s_power_grid", spriteList);
                            }
                            if (cell->hasWPower) {
                                sprite pickupSprite = Sprite();
                                pickupSprite.pos.x = (float)(cell->col * CELL_DIM);
                                pickupSprite.pos.y = (float)(cell->row * CELL_DIM) + shiftAmount;
                                addSprite(pickupSprite, assets, "atlas", "w_power_grid", spriteList);
                            }
                            if (cell->hasLPower) {
                                sprite pickupSprite = Sprite();
                                pickupSprite.pos.x = (float)(cell->col * CELL_DIM);
                                pickupSprite.pos.y = (float)(cell->row * CELL_DIM) + shiftAmount;
                                addSprite(pickupSprite, assets, "atlas", "l_power_grid", spriteList);
                            }
                            if (cell->hasExtraHealth) {
                                sprite pickupSprite = Sprite();
                                pickupSprite.pos.x = (float)(cell->col * CELL_DIM);
                                pickupSprite.pos.y = (float)(cell->row * CELL_DIM) + shiftAmount;
                                addSprite(pickupSprite, assets, "atlas", "extra_health", spriteList);
                            }
                        }
                    }
                }
            }

            if (!game->ship.dead) {
                sprite shipSprite = Sprite();
                shipSprite.pos.x = (float)(game->ship.col * CELL_DIM + (game->ship.x / ((float)SPACE_SHIP_ZOOM)));
                shipSprite.pos.y = (float)(game->ship.row * CELL_DIM + (game->ship.y / ((float)SPACE_SHIP_ZOOM)));
                shipSprite.anchor.x = 0.5f;
                shipSprite.anchor.y = 0.5f;

                if (game->ship.hasShield) {
                    addSprite(shipSprite, assets, "atlas", "ship_icon_shield", spriteList);
                }
                else {
                    addSprite(shipSprite, assets, "atlas", "ship_icon", spriteList);
                }
            }

            if (game->state == GAME_STATE_ZOOMING_IN || 
                game->state == GAME_STATE_ZOOMING_OUT || 
                game->state == GAME_STATE_ZOOMING_OUT_OF_BATTLE) 
            {
                popSpriteMatrix(spriteList);
                popSpriteMatrix(spriteList);
                popSpriteMatrix(spriteList);
            }

            popSpriteMatrix(spriteList);
        }
        else if (game->currentView == VIEW_MODE_SHIP) {
            pushSpriteTransform(spriteList, Vector2(-game->screenX, -game->screenY));

            static float bombRotation = 0.0f;
            bombRotation -= (1.0f / 4.0f * PI) * DELTA_TIME;

            if (game->state == GAME_STATE_ZOOMING_INTO_BATTLE) {
                float scaleAmount = 1.0f + 4.0f * (game->zoomT / 0.5f);

                vector2 target = Vector2();
                target.x = (game->ship.col - game->screenCol) * SPACE_SHIP_CELL_DIM + game->ship.x;
                target.y = (game->ship.row - game->screenRow) * SPACE_SHIP_CELL_DIM + game->ship.y;

                matrix3x3 posMatrix;
                posMatrix = translationMatrix(target.x, target.y);
                pushSpriteMatrix(posMatrix, spriteList);

                matrix3x3 scaleTransform = scaleMatrix3x3(scaleAmount, scaleAmount);
                pushSpriteMatrix(scaleTransform, spriteList);

                posMatrix = translationMatrix(-target.x, -target.y);
                pushSpriteMatrix(posMatrix, spriteList);
            }

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
                        if (cell->hitTimer > 0.0f) {
                            cellSprite.tint = 0xdddddd;
                        }
                        if (cell->cracked) {
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
                            addSprite(cellSprite, assets, "atlas", "big_cell_closed_cracked", spriteList);
                        }
                        else {
                            addSprite(cellSprite, assets, "atlas", "big_cell_closed", spriteList);
                        }
                        if (cell->flagged) {
                            addSprite(cellSprite, assets, "atlas", "big_flag", spriteList);
                        }

                    }

                }
            }

            for (int i = 0; i < MAX_NUM_PLAYER_BULLETS; ++i) {
                space_bullet *bullet = &game->playerBullets[i];
                if (bullet->active) {
                    sprite bulletSprite = Sprite();

            float bulletX = (bullet->col - game->screenCol) * SPACE_SHIP_CELL_DIM + bullet->pos.x;
            float bulletY = (bullet->row - game->screenRow) * SPACE_SHIP_CELL_DIM + bullet->pos.y;

                    bulletSprite.pos.x = bulletX;
                    bulletSprite.pos.y = bulletY;
                    bulletSprite.anchor.x = 0.5f;
                    bulletSprite.anchor.y = 0.5f;
                    addSprite(bulletSprite, assets, "atlas", "player_bullet", spriteList);
                }
            }

            if (game->state == GAME_STATE_ZOOMING_INTO_BATTLE) {
                popSpriteMatrix(spriteList);
                popSpriteMatrix(spriteList);
                popSpriteMatrix(spriteList);

                float scaleAmount = 1.0f + 4.0f * (game->zoomT / 0.5f);

            popSpriteMatrix(spriteList);
                sprite bgSprite = Sprite();
                bgSprite.pos.x = 0.0f;
                bgSprite.pos.y = 0.0f;
                bgSprite.alpha = game->zoomT / 0.5f;
                addSprite(bgSprite, assets, "atlas", "space_field", spriteList);
            pushSpriteTransform(spriteList, Vector2(-game->screenX, -game->screenY));
            }

            float shipX = (game->ship.col - game->screenCol) * SPACE_SHIP_CELL_DIM + game->ship.x;
            float shipY = (game->ship.row - game->screenRow) * SPACE_SHIP_CELL_DIM + game->ship.y;

            if (!game->ship.dead) {
                sprite shipSprite = Sprite();
                shipSprite.pos.x = shipX;
                shipSprite.pos.y = shipY;
                shipSprite.anchor.x = 0.5f;
                shipSprite.anchor.y = 0.5f;
                addSprite(shipSprite, assets, "atlas", "ship", spriteList);

                if (game->ship.hasShield) {
                    addSprite(shipSprite, assets, "atlas", "shield", spriteList);
                }
            }

            popSpriteMatrix(spriteList);
        }
        else if (game->currentView == VIEW_MODE_BATTLE) {
            sprite bgSprite = Sprite();
            bgSprite.pos.x = 0.0f;
            bgSprite.pos.y = 0.0f;
            addSprite(bgSprite, assets, "atlas", "space_field", spriteList);

            for (int i = 0; i < 12; ++i) {
                sprite starSprite = Sprite();
                float x = 10.0f + i * 50.0f;
                while (x > 192.0f) {
                    x -= 192.0f;
                }
                starSprite.pos.x = x;

                float y = i * 100.0f;
                y += 45.0f * game->starT;
                while (y > 236.0f) {
                    y -= 236.0f;
                }
                starSprite.pos.y = y;

                starSprite.anchor.x = 0.5f;
                starSprite.anchor.y = 0.5f;
                addSprite(starSprite, assets, "atlas", "big_star", spriteList);
            }

            for (int i = 0; i < 20; ++i) {
                sprite starSprite = Sprite();
                float x = 10.0f + i * 150.0f;
                while (x > 192.0f) {
                    x -= 192.0f;
                }
                starSprite.pos.x = x;

                float y = i * 200.0f;
                y += 15.0f * game->starT;
                while (y > 236.0f) {
                    y -= 236.0f;
                }
                starSprite.pos.y = y;

                starSprite.anchor.x = 0.5f;
                starSprite.anchor.y = 0.5f;
                addSprite(starSprite, assets, "atlas", "small_star", spriteList);
            }

            if (!game->ship.dead) {
                sprite shipSprite = Sprite();
                shipSprite.pos.x = game->ship.battleX;
                shipSprite.pos.y = game->ship.battleY;
                shipSprite.anchor.x = 0.5f;
                shipSprite.anchor.y = 0.5f;
                if (game->ship.hurt) {
                    shipSprite.alpha = 0.5f;
                }
                addSprite(shipSprite, assets, "atlas", "ship", spriteList);

                if (game->ship.hasShield) {
                    addSprite(shipSprite, assets, "atlas", "shield", spriteList);
                }
            }

            for (int i = 0; i < MAX_NUM_PLAYER_BULLETS; ++i) {
                space_bullet *bullet = &game->playerBullets[i];
                if (bullet->active) {
                    sprite bulletSprite = Sprite();
                    bulletSprite.pos.x = bullet->pos.x;
                    bulletSprite.pos.y = bullet->pos.y;
                    bulletSprite.anchor.x = 0.5f;
                    bulletSprite.anchor.y = 0.5f;
                    addSprite(bulletSprite, assets, "atlas", "player_bullet", spriteList);
                }
            }

            for (int i = 0; i < MAX_NUM_ENEMY_BULLETS; ++i) {
                space_bullet *bullet = &game->enemyBullets[i];
                if (bullet->active) {
                    sprite bulletSprite = Sprite();
                    bulletSprite.pos.x = bullet->pos.x;
                    bulletSprite.pos.y = bullet->pos.y;
                    bulletSprite.anchor.x = 0.5f;
                    bulletSprite.anchor.y = 0.5f;
                    addSprite(bulletSprite, assets, "atlas", "enemy_bullet", spriteList);
                }
            }

            for (int i = 0; i < 30; ++i) {
                space_enemy *enemy = &game->enemies[i];
                if (enemy->active) {
                    sprite enemySprite = Sprite();
                    enemySprite.pos.x = enemy->pos.x;
                    enemySprite.pos.y = enemy->pos.y;
                    enemySprite.anchor.x = 0.5f;
                    enemySprite.anchor.y = 0.5f;
                        if (enemy->hitTimer > 0.0f) {
                            enemySprite.tint = 0xaaaaaa;
                        }
                    addSprite(enemySprite, assets, "atlas", appendString("enemy_", numToString(enemy->enemyType, &stringMemory), &stringMemory), spriteList);
                }

            }
        }

        // Explosions
        for (int i = 0; i < MAX_EXPLOSIONS; ++i) {
            space_explosion *explosion = &game->explosions[i];
            if (explosion->active) {

                sprite explosionSprite = Sprite();
                explosionSprite.pos.x = explosion->pos.x;
                explosionSprite.pos.y = explosion->pos.y;
                explosionSprite.anchor.x = 0.5f;
                explosionSprite.anchor.y = 0.5f;
                float scale = (explosion->t/0.25f) * explosion->maxScale;
                explosionSprite.scale = scale;
                float alpha = 1.0f;
                if (explosion->t > 0.125f) {
                    alpha = 1.0f - ((explosion->t - 0.125f) / 0.125f);
                }
                explosionSprite.alpha = alpha;
                addSprite(explosionSprite, assets, "atlas", "explosion", spriteList);
            }
        }

        // UI
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

        if (game->ship.beamType == BEAM_TYPE_WAVE) {
            sprite beamSprite = Sprite();
            beamSprite.pos.x = 210.0f;
            beamSprite.pos.y = 137.0f;
            addSprite(beamSprite, assets, "atlas", "w_power_ui", spriteList);
        }
        if (game->ship.beamType == BEAM_TYPE_LASER) {
            sprite beamSprite = Sprite();
            beamSprite.pos.x = 210.0f;
            beamSprite.pos.y = 137.0f;
            addSprite(beamSprite, assets, "atlas", "l_power_ui", spriteList);
        }
        if (game->ship.beamType == BEAM_TYPE_WAVE || game->ship.beamType == BEAM_TYPE_LASER) {
            sprite beamSprite = Sprite();
            beamSprite.pos.x = 250.0f;
            beamSprite.pos.y = 140.0f;
            switch (game->ship.beamLevel) {
            case 0:
                addSprite(beamSprite, assets, "atlas", "roman_i", spriteList);
                break;
            case 1:
                addSprite(beamSprite, assets, "atlas", "roman_ii", spriteList);
                break;
            case 2:
                addSprite(beamSprite, assets, "atlas", "roman_iii", spriteList);
                break;
            }
        }

        sprite buttonSprite = Sprite();
        buttonSprite.pos.x = 200.0f;
        buttonSprite.pos.y = 180.0f;
        if (game->currentView == VIEW_MODE_BATTLE) {
            buttonSprite.tint = 0x999999;
        }
        if (game->buttonDown) {
            addSprite(buttonSprite, assets, "atlas", "button_down", spriteList);
        }
        else {
            addSprite(buttonSprite, assets, "atlas", "button_up", spriteList);
        }

        if (game->currentView == VIEW_MODE_FIELD) {
            addText(222.0f, 185.0f, "SHIP", assets, "font", spriteList);
        }
        else if (game->currentView == VIEW_MODE_SHIP) {
            addText(218.0f, 185.0f, "FIELD", assets, "font", spriteList);
        }
        if (game->currentView != VIEW_MODE_BATTLE) {
            addText(222.0f, 195.0f, "VIEW", assets, "font", spriteList);
        }

        if (game->suckingIntoWormhole) {
            sprite statusSprite = Sprite();
            statusSprite.pos.x = 0.0f;
            statusSprite.pos.y = 0.0f;
            addSprite(statusSprite, assets, "atlas", "status_bar", spriteList);

            addText(30.0f, 5.0f, "WORMHOLE DETECTED", assets, "font", spriteList);
        }

        if (game->state == GAME_STATE_TRANSITION_TO_FINAL_BATTLE) {
            sprite statusSprite = Sprite();
            statusSprite.pos.x = 0.0f;
            statusSprite.pos.y = 0.0f;
            addSprite(statusSprite, assets, "atlas", "status_bar", spriteList);

            addText(50.0f, 5.0f, "FINAL BATTLE", assets, "font", spriteList);
        }
    }
}
