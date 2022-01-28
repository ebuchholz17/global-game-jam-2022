#ifndef MINESWEEPER_GAME_H
#define MINESWEEPER_GAME_H

#define FIELD_WIDTH 16
#define FIELD_HEIGHT 18
#define CELL_DIM 12


struct mine_cell {
    int row;
    int col;
    bool open;
    bool hasBomb;
    bool flagged;
    int numAdjBombs;

    bool seen;
};

struct minesweeper_game {
    mine_cell cells[FIELD_WIDTH * FIELD_HEIGHT];
};


#endif
