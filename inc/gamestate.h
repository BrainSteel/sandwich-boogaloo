

#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "common.h"
#include "menu.h"
#include "pixel.h"
#include "pool.h"

#define INPUT_NUM 2

#define ENTITY_ACTIVE           (1 << 0)
#define ENTITY_PLAYERCONTROLLED (1 << 1)
#define ENTITY_COLLIDES         (1 << 2)
#define ENTITY_MOVES            (1 << 3)
#define ENTITY_ORIENTED         (1 << 4)
#define ENTITY_INVISIBLE        (1 << 5)
#define ENTITY_PLAYER           (ENTITY_PLAYERCONTROLLED | ENTITY_COLLIDES | ENTITY_MOVES | ENTITY_ORIENTED)

typedef enum PlayerKeys
{
    KeyUp,
    KeyDown,
    KeyLeft,
    KeyRight,
    KeyNum
} PlayerKeys;

typedef enum Orientation
{
    OrientUp,
    OrientDown,
    OrientLeft,
    OrientRight
} Orientation;

typedef enum CameraSetting
{
    CameraFirst,
    CameraAverage,
    CameraLocked
} CameraSetting;

typedef enum GameDifficulty
{
    EASY,
    MEDIUM,
    HARD,
    EXTREME
} GameDifficulty;

typedef struct Input
{
    uint8_t keydown[KeyNum];
    uint8_t mousestate;
    uint16_t mousex, mousey;
} Input;

typedef struct Entity
{
    int grid_x, grid_y;
    float off_x, off_y;
    float dx, dy;
    float ddx, ddy;

    int controller;
    Orientation orient;
    Bitmap* textures[4];

    uint32_t flags;

    struct Entity* next;
} Entity;

typedef struct TextureSet
{
    Bitmap numbers;
    Bitmap beach;

    Bitmap quadropus;

    // Pickup texture
    Bitmap bread;
    Bitmap crecent;
    Bitmap lettuce;
    Bitmap tomato;

    // Tile textures
    Bitmap bone_sand;
    Bitmap sesame_sand;
    Bitmap wavy_sand;
} TextureSet;

typedef struct GameState
{
    MemoryPool* pool;

    MainMenu main_menu;

    int score;

    int difficulty;

    int game_mode;

    int32_t gridleft, gridtop, gridright, gridbottom;
    float grid_m;
    float screenw_m, screenh_m;

    uint64_t logical_frames;
    uint32_t fps;
    uint32_t fps_enabled;
    uint32_t rendering_enabled;

    Input in[INPUT_NUM];
    Entity* camera_follow;

    uint32_t entity_section;
    uint32_t transient_section;

    Entity* free_entity;
    Entity* entities;
    uint32_t numentities;

    TextureSet textures;

} GameState;

int InitializeGameState( GameState* state );
Entity* AddBlank( GameState* state, int grid_x, int grid_y );
Entity* AddPlayer( GameState* state, int grid_x, int grid_y, int input_index );
Entity* AddWall( GameState* state, int grid_x, int grid_y );
void DestroyEntity( GameState* state, Entity* entity );
void UpdateGameState( GameState* state, float elapsed );
void RenderGameState( Bitmap* screen, const Rect* dstrect, GameState* state, float elapsed );

#endif
