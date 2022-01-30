#include "minesweeper_game.h"

void initMinesweeperGame (memory_arena *memory, minesweeper_game* minesweeperGame) {
    setRNGSeed(0); // TODO(ebuchholz): seed with time?

    *minesweeperGame = {};
}

void updateMinesweeperGame (memory_arena *memory, memory_arena *tempMemory, game_assets *assets, game_sounds *gameSounds, game_input *input, 
                      minesweeper_game *minesweeperGame, sprite_list *spriteList)
{
    if (input->upKey.down) {
        minesweeperGame->golfManY += -200.0f * DELTA_TIME;
    }
    if (input->downKey.down) {
        minesweeperGame->golfManY += 200.0f * DELTA_TIME;
    }
    if (input->leftKey.down) {
        minesweeperGame->golfManX += -200.0f * DELTA_TIME;
    }
    if (input->rightKey.down) {
        minesweeperGame->golfManX += 200.0f * DELTA_TIME;
    }
}

void drawMinesweeperGame (memory_arena *memory, memory_arena *tempMemory, game_assets *assets, 
                    minesweeper_game *minesweeperGame, sprite_list *spriteList) 
{
    // memory for dynamically created strings
    memory_arena stringMemory = {};
    stringMemory.capacity = 512 * 1024;
    stringMemory.base = allocateMemorySize(tempMemory, stringMemory.capacity);

    pushSpriteTransform(spriteList, Vector2(GAME_WIDTH/2.0f, GAME_HEIGHT/2.0f));

    sprite golfManSprite = Sprite();
    golfManSprite.pos.x = minesweeperGame->golfManX;
    golfManSprite.pos.y = minesweeperGame->golfManY;
    golfManSprite.anchor.x = 0.5f;
    golfManSprite.anchor.y = 0.5f;
    addSprite(golfManSprite, assets, "atlas", "golfman", spriteList);

    popSpriteMatrix(spriteList);
}
