#ifndef PIXEL_H
#define PIXEL_H

#include "windows.h"
#include "stdint.h"
#include "stdio.h"

#include "common.h"

#define TRANSPARENT_MASK 0xFF000000

typedef enum AlphaSetting
{
    AlphaIgnore,
    AlphaBinary
} AlphaSetting;

typedef struct Bitmap
{
    BITMAPINFO info;
    uint32_t w, h;
    void* pixels;
} Bitmap;

typedef struct Rect
{
    int x, y;
    int w, h;
} Rect;

typedef struct RGB
{
    uint8_t r, g, b;
} RGB;

typedef enum ClearColor
{
    WHITE,
    BLACK
} ClearColor;

// PIXEL-LEVEL RENDERING
static inline void WriteRGB( uint32_t* towrite, uint8_t r, uint8_t g, uint8_t b )
{
    // X R G B      X R G B     X R G B     X R G B
    // X R G B      X R G B     X R G B     X R G B
    // X R G B      X R G B<-dt X R G B     X R G B
    // X R G B      X R G B     X R G B     X R G B
    // X R G B      X R G B     X R G B     X R G B ...
    //
    // Want: 1, 2
    // Example: WriteRGB( img->pixels + (img->w (4) * 2 + 1) * 4(bytes_per_pixel), 128, 16, 32 )
    // Each pixel is a uint32_t
    // 0x00RRGGBB == 0x00RR0000 | 0x0000GG00 | 0x000000BB == (0x000000RR << 16) | (0x000000GG << 8) | 0x000000BB
    // In this case, 0x000000RR == 0x000000F0, 0x000000GG == 0x00000010, 0x000000BB == 0x00000020

    *towrite = (r << 16) | (g << 8) | b;
}

static inline void WriteRGBA( uint32_t* towrite, uint8_t r, uint8_t g, uint8_t b, uint8_t a )
{
    *towrite = (a << 24) | (r << 16) | (g << 8) | b;
}

int CreateImage( Bitmap* img, uint32_t w, uint32_t h );
int ResizeImageMemory( Bitmap* img, uint32_t w, uint32_t h );
int LoadImageFromFile( const char* filename, Bitmap* img, const RGB* colorkey );
void DestroyImage( Bitmap* screen_img );
void ClearBitmap( Bitmap* img, ClearColor col );
void FillRectangle( Bitmap* img, const Rect* dst, RGB col );
void DrawHorizontalLine( Bitmap* img, int xstart, int xend, int y, RGB color );
void DrawVerticalLine( Bitmap* img, int ystart, int yend, int x, RGB color );
void DrawRectangle( Bitmap* img, const Rect* rect, RGB color );
void DrawGradient( Bitmap* img, const Rect* dst, RGB startcol, RGB xcol, RGB ycol );
void FillGradientPattern( Bitmap* img, int x_off, int y_off );
void ImageBlit( const Bitmap* src, Bitmap* dst, const Rect* srcrect, uint32_t dstx, uint32_t dsty, AlphaSetting alpha );
void ImageBlitScaled( Bitmap* src, Bitmap* dst, const Rect* srcrect, const Rect* dstrect, AlphaSetting alpha );

#endif
