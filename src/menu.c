

#include "Menu.h"
#include "pixel.h"
#include "common.h"

#define MENU_X SCREEN_WIDTH / 4
#define MENU_Y SCREEN_HEIGHT / 4
#define WIDTH 600
#define HEIGHT 100

static inline int IsWithin( const Rect* r, int x, int y )
{
    printf("x: %d, y: %d\n", x, y);
    if( x > r->x && x < r->x + r->w && y > r->y && y < r->y + r->h )
    { return 1; }
    return 0;
}

int DisplayMainMenu( Bitmap* img, int mouse_x, int mouse_y )
{
    Button start_button;
    Button help_button;
    Button exit_button;

    Rect start_rect;
    start_rect.x = MENU_X;
    start_rect.w = WIDTH;
    start_rect.y = MENU_Y;
    start_rect.h = HEIGHT;

    RGB col_outline;
    col_outline.r = 128;
    col_outline.g = 128;
    col_outline.b = 128;

    RGB col_hover;
    col_hover.r = 255;
    col_hover.g = 255;
    col_hover.b = 255;

    RGB col_select;
    col_select.r = 0;
    col_select.g = 255;
    col_select.b = 255;

    DrawRectangle( img, &start_rect, col_outline );

    return 0;
}

void UpdateMainMenu( int mouse_x, int mouse_y )
{
    if( IsWithin( &start_rect, mouse_x, mouse_y ) )
    {
        printf("YAY\n");
    }
}
