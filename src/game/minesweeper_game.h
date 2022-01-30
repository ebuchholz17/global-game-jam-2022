#ifndef MINESWEEPER_GAME_H
#define MINESWEEPER_GAME_H

#define FIELD_WIDTH 16
#define FIELD_HEIGHT 36
#define CELL_DIM 12
#define SPACE_SHIP_ZOOM 7
#define SPACE_SHIP_CELL_DIM 84.0f

#define MAX_NUM_PLAYER_BULLETS 1000
#define MAX_NUM_ENEMY_BULLETS 1000
#define NUM_BULLETS_PER_CELL 10

#define BULLET_LIFETIME 5.0f
#define BULLET_RADIUS 3.0f

#define PLAYER_RADIUS 5.0f
#define ENEMY_RADIUS 9.0f
#define PLAYER_INVINCIBILITY_TIMER 1.0f

#define MAX_EXPLOSIONS 20

struct mine_cell {
    int row;
    int col;
    bool open;
    bool hasBomb;
    bool flagged;
    bool cracked;
    int numAdjBombs;

    float hitTimer;
    int hitpoints;

    bool seen;

    bool hasSPower;
    bool hasWPower;
    bool hasLPower;
    bool hasExtraHealth;
};

typedef mine_cell *mine_cell_ptr;
#define LIST_TYPE mine_cell_ptr
#include "list.h"

enum beam_type {
    BEAM_TYPE_NORMAL,
    BEAM_TYPE_WAVE,
    BEAM_TYPE_LASER
};

struct space_ship {
    int row;
    int col;

    float x;
    float y;

    float battleX = 96.0f;
    float battleY = 200.0f;

    bool hurt;
    float invincibilityTimer;
    bool dead;

    bool hasShield;

    beam_type beamType;
    int beamLevel;
    int beamPower;
};

enum view_mode {
    VIEW_MODE_FIELD,
    VIEW_MODE_SHIP,
    VIEW_MODE_BATTLE
};

enum hand_state {
    HAND_STATE_LEFT,
    HAND_STATE_MIDDLE,
    HAND_STATE_RIGHT
};

enum minesweeper_game_state {
    GAME_STATE_TITLE_SCREEN,
    GAME_STATE_NORMAL,
    GAME_STATE_ZOOMING_IN,
    GAME_STATE_ZOOMING_OUT,
    GAME_STATE_ZOOMING_INTO_BATTLE,
    GAME_STATE_ZOOMING_OUT_OF_BATTLE,
    GAME_STATE_BATTLE,
    GAME_STATE_REVEALING_CELLS,
    GAME_STATE_GAME_OVER_SCREEN,
    GAME_STATE_TRANSITION_TO_FINAL_BATTLE,
    GAME_STATE_WIN_SCREEN
};


struct entity_id {
    int value;
    int generation;
};

struct space_bullet {
    int row;
    int col;
    entity_id id;
    bool active;
    vector2 pos; 
    vector2 velocity; 
    float timeAlive;
};

#define LIST_TYPE entity_id
#include "list.h"

enum enemy_pattern_type {
    ENEMY_PATTERN_TYPE_BASIC,
    ENEMY_PATTERN_TYPE_BASIC_LEFT,
    ENEMY_PATTERN_TYPE_SPACE_INVADERS,
    ENEMY_PATTERN_TYPE_SWEEP_DOWN,
    ENEMY_PATTERN_TYPE_SPIN,
    ENEMY_PATTERN_TYPE_COUNT,
    ENEMY_PATTERN_TYPE_FINAL,
    ENEMY_PATTERN_TYPE_NONE
};

enum space_invaders_moving_dir {
    SPACE_INVADERS_MOVING_DIR_LEFT,
    SPACE_INVADERS_MOVING_DIR_RIGHT,
    SPACE_INVADERS_MOVING_DIR_DOWN
};

struct enemy_pattern {
    float t;
    entity_id_list enemyIDs;
    enemy_pattern_type type;
    bool complete;
    float minBulletSpawnTime;
    float maxBulletSpawnTime;

    float spaceInvadersX = -30.0f;
    float spaceInvadersY;
    space_invaders_moving_dir movingDir = SPACE_INVADERS_MOVING_DIR_RIGHT;
    space_invaders_moving_dir lastDir = SPACE_INVADERS_MOVING_DIR_RIGHT;

    float enemyTs[30];
    int enemySpinPhases[30];
};

struct space_enemy {
    entity_id id;
    bool active;

    int hp;
    int enemyType;
    vector2 pos;
    float hitTimer;
};

typedef space_enemy *space_enemy_ptr;
#define LIST_TYPE space_enemy_ptr
#include "list.h"

struct bullet_cell {
    space_bullet *bullets[NUM_BULLETS_PER_CELL];
    int numBullets;
};

struct bullet_cell_grid{
    bullet_cell *cells;
    int numRows;
    int numCols;
};

struct space_explosion {
    entity_id id;
    float maxScale;
    bool active;
    float t;
    vector2 pos;
};

struct minesweeper_game {
    mine_cell cells[FIELD_WIDTH * FIELD_HEIGHT];

    space_bullet playerBullets[MAX_NUM_PLAYER_BULLETS];
    space_bullet enemyBullets[MAX_NUM_ENEMY_BULLETS];

    float bulletTimer;

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

    minesweeper_game_state state;
    float zoomT;

    bool suckingIntoWormhole;
    bool touchingWormhole;
    int wormholeRow;
    int wormholeCol;

    float starT = 0.0f;

    space_enemy enemies[30];
    enemy_pattern enemyPattern;

    float enemyBulletT;
    float nextEnemyBulletTime;

    space_explosion explosions[MAX_EXPLOSIONS];
    float battleTimer;
    float deadTimer = 0.0f;
    int waveNum;

    float revealT;
    mine_cell_ptr_list revealingCells;

    float finalBattleWaitT;
    bool finalBattle;
};


#endif
