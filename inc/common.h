
#ifndef COMMON_H
#define COMMON_H

#include "stdint.h"

#define ICON 101

#define FRAMERATE 25

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define SCREEN_WIDTH_OFFSET 16
#define SCREEN_HEIGHT_OFFSET 36
#define BYTES_PER_PIXEL 4
#define BITS_PER_PIXEL (BYTES_PER_PIXEL * 8)

#define MOUSE_LDOWN (1 << 0)
#define MOUSE_RDOWN (1 << 1)

#define SIZE_ARRAY( arr ) (( sizeof(arr) / sizeof(arr[0]) ))


typedef enum GameMode
{
    GameMenu,
    GamePlaying,
    GamePaused,
    GameQuit
} GameMode;


extern char error_buf[256];

#endif
