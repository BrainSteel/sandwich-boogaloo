

#ifndef MENU_H
#define MENU_H

#include "pixel.h"

typedef enum WidgetStateEnum
{
    Default,
    IsHovered,
    IsSelected
}WidgetState;

typedef struct ButtonStruct
{
    int state;
}Button;

/*typedef struct MenuStruct
{
    Button button;
}Menu;*/

int DisplayMainMenu( Bitmap* img, int mouse_x, int mouse_y );
void UpdateMainMenu( int mouse_x, int mouse_y );


#endif
