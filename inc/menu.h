

#ifndef MENU_H
#define MENU_H

#define MENU_X SCREEN_WIDTH / 4
#define MENU_Y SCREEN_HEIGHT / 10
#define WIDTH 600
#define HEIGHT 100

#define WIDGET_BUFFER 50
#define NUM_WIDGETS 3

#define WIDGET_X_TEXT_BUFFER 0
#define WIDGET_Y_TEXT_BUFFER 0

#include "windows.h"

#include "common.h"
#include "pixel.h"

typedef enum WidgetStateEnum
{
    Default, //0
    IsHovered, //1
    IsSelected //2
}WidgetState;

typedef struct ButtonStruct
{
    int state;
    RGB col_outline;
}Button;

typedef struct MainMenuStruct
{
    Rect start_rect;
    Rect help_rect;
    Rect quit_rect;
    Button start_button;
    Button help_button;
    Button quit_button;
}MainMenu;

void CreateMainMenu( MainMenu* main_menu, Bitmap* img );
void DisplayMainMenu( HDC dc, MainMenu* main_menu, Bitmap* img, int mouse_x, int mouse_y );
void UpdateMainMenu( MainMenu* main_menu, int* result, int mouse_x, int mouse_y, uint8_t mousestate );
int MenuAction( MainMenu* main_menu );


#endif
