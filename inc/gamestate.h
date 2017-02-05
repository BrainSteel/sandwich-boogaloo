

#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "common.h"
#include "menu.h"
#include "pool.h"

#define ENTITY_PLAYERCONTROLLED (1 << 0)
#define ENTITY_COLLIDES         (1 << 1)
#define ENTITY_MOVES            (1 << 2)
#define ENTITY_CAMERAFOCUS      (1 << 3)

typedef enum PlayerKeys
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NUMKEYS
} PlayerKeys;

typedef enum GameMode
{
    GamePlaying,
    GameMenu,
    GamePaused
} GameMode;

typedef struct Input
{
    uint8_t keydown[NUMKEYS];
    uint8_t mousestate;
    uint16_t mousex, mousey;
} Input;

typedef struct Entity
{
    int x, y;
    uint32_t flags;
    struct Entity* next;
} Entity;


typedef struct GameState
{
    MemoryPool* pool;

    MainMenu main_menu;

    uint32_t gridw, gridh;
    uint32_t grid_m;
    uint32_t screenw_m, screenh_m;

    Input in;

    Entity* nextentity;

    Entity* entities;

} GameState;

#endif
