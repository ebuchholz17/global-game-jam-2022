#ifndef MINESWEEPER_GAME_H
#define MINESWEEPER_GAME_H

#define FIELD_WIDTH 16
#define FIELD_HEIGHT 36
#define CELL_DIM 12
#define SPACE_SHIP_ZOOM 7
#define SPACE_SHIP_CELL_DIM 84.0f

struct mine_cell {
    int row;
    int col;
    bool open;
    bool hasBomb;
    bool flagged;
    int numAdjBombs;

    bool seen;
};

struct space_ship {
    int row;
    int col;

    float x;
    float y;
};

enum view_mode {
    VIEW_MODE_FIELD,
    VIEW_MODE_SHIP
};

enum hand_state {
    HAND_STATE_LEFT,
    HAND_STATE_MIDDLE,
    HAND_STATE_RIGHT
};

struct minesweeper_game {
    mine_cell cells[FIELD_WIDTH * FIELD_HEIGHT];

    space_ship ship;

    view_mode currentView;

    int screenRow;
    int screenCol;
    float screenX;
    float screenY;

    hand_state leftHandState;
    hand_state rightHandState;

    int score;
    
    int hitpoints = 3;
    int lives = 3;

    bool buttonDown;
};


#endif
