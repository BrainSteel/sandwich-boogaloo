

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
#define ENTITY_INVISIBLE        (1 << 4)
#define ENTITY_ITEM             (1 << 5)
#define ENTITY_EVIL             (1 << 6)
#define ENTITY_INTERACTIVE      (1 << 7)
#define ENTITY_PLAYER           (ENTITY_PLAYERCONTROLLED | ENTITY_COLLIDES | ENTITY_MOVES )
#define ENTITY_SAND_WITCH       (ENTITY_MOVES | ENTITY_COLLIDES | ENTITY_EVIL)

typedef enum PlayerKeys
{
    KeyUp,
    KeyDown,
    KeyLeft,
    KeyRight,
    KeyNum
} PlayerKeys;

typedef enum CameraSetting

typedef enum ItemType
{
    ItemLettuce,
    ItemTomato,
    ItemQuadropus,
    ItemCrecent,
    ItemNum
} ItemType;

typedef enum TileSet
{
    TileBoneSand,
    TileSesameSand,
    TileWavySand,
    TileNum
} TileSet;

typedef enum GameDifficulty
{
    EASY,
    MEDIUM,
    HARD,
    EXTREME,
    TEST
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

    int controller;
    Rect texture_rect;
    Bitmap* texture;

    uint32_t flags;

    int paralyzed_duration;
    uint32_t score;
    ItemType type;

    struct Entity* next;
} Entity;

typedef struct TextureSet
{
    Bitmap numbers;
    Bitmap beach;
    Bitmap menu_beach;

    // Player textures
    Bitmap bread;
    Bitmap player_texture;

    Bitmap witch;

    // Pickup texture
    Bitmap items[ItemNum];

    // Tile textures
    TileSet tiletype;
    Bitmap tiles[TileNum];
} TextureSet;

typedef struct GameState
{
    MemoryPool* pool;

    MainMenu main_menu;

    int score;

    int difficulty;

    int game_mode;

    // Used to indicate first time in Start Game (Should be reset after game over)
    int first_time;

    int paused;

    Rect timer_rect;

    int32_t gridleft, gridtop, gridright, gridbottom;
    float grid_m;
    float screenw_m, screenh_m;

    uint32_t player_score;
    uint64_t logical_frames;
    uint32_t fps;
    uint32_t rendering_enabled;

    int start_next_level;
    int game_over;

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
void GenerateLevel( GameState* state );
Entity* AddBlank( GameState* state, int grid_x, int grid_y );
Entity* AddPlayer( GameState* state, int grid_x, int grid_y, int input_index );
Entity* AddWall( GameState* state, int grid_x, int grid_y );
Entity* AddTile( GameState* state, int grid_x, int grid_y );
Entity* AddItem( GameState* state, int grid_x, int grid_y );
Entity* AddCrecent( GameState* state, int grid_x, int grid_y );
Entity* AddWitch( GameState* state, int grid_x, int grid_y );
void DestroyEntity( GameState* state, Entity* entity );
void UpdateGameState( GameState* state, float elapsed );
void RenderGameState( Bitmap* screen, const Rect* dstrect, GameState* state, float elapsed );

#endif
