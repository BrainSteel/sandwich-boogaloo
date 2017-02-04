
#ifndef COMMON_H
#define COMMON_H

#define ICON 101

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define BYTES_PER_PIXEL 4
#define BITS_PER_PIXEL (BYTES_PER_PIXEL * 8)

#define SIZE_ARRAY( arr ) (( sizeof(arr) / sizeof(arr[0]) ))

extern char error_buf[256];

#endif
