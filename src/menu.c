

#include "menu.h"

static inline int IsWithin( const Rect* r, int x, int y )
{
    if( x > r->x && x < r->x + r->w && y > r->y && y < r->y + r->h )
    { return 1; }
    return 0;
}

void CreateMainMenu( MainMenu* main_menu, Bitmap* img )
{
    main_menu->start_rect.x = MENU_X;
    main_menu->start_rect.w = WIDTH;
    main_menu->start_rect.y = MENU_Y;
    main_menu->start_rect.h = HEIGHT;

    main_menu->help_rect.x = MENU_X;
    main_menu->help_rect.w = WIDTH;
    main_menu->help_rect.y = main_menu->start_rect.y + main_menu->start_rect.h + WIDGET_BUFFER;
    main_menu->help_rect.h = HEIGHT;

    main_menu->quit_rect.x = MENU_X;
    main_menu->quit_rect.w = WIDTH;
    main_menu->quit_rect.y = main_menu->help_rect.y + main_menu->help_rect.h + WIDGET_BUFFER;
    main_menu->quit_rect.h = HEIGHT;
}

void DisplayMainMenu( HDC hdc, MainMenu* main_menu, Bitmap* img, int mouse_x, int mouse_y )
{
    // Default (Cursor not in rect)
    if( main_menu->start_button.state == Default )
    {
        //printf("Default\n");
        main_menu->start_button.col_outline.r = 128;
        main_menu->start_button.col_outline.g = 128;
        main_menu->start_button.col_outline.b = 128;
        //main_menu->start_button.state = Default;
    }
    // Hovered
    else if( main_menu->start_button.state == IsHovered )
    {
        //printf("Hovered\n");
        main_menu->start_button.col_outline.r = 255;
        main_menu->start_button.col_outline.g = 255;
        main_menu->start_button.col_outline.b = 255;
        main_menu->start_button.state = Default;
    }
    // Selected
    else if( main_menu->start_button.state == IsSelected )
    {
        //printf("Selected\n");
        main_menu->start_button.col_outline.r = 0;
        main_menu->start_button.col_outline.g = 255;
        main_menu->start_button.col_outline.b = 255;
        main_menu->start_button.state = Default;
    }

    // Default (Cursor not in rect)
    if( main_menu->help_button.state == Default )
    {
        //printf("Default\n");
        main_menu->help_button.col_outline.r = 128;
        main_menu->help_button.col_outline.g = 128;
        main_menu->help_button.col_outline.b = 128;
        //main_menu->help_button.state = Default;
    }
    // Hovered
    else if( main_menu->help_button.state == IsHovered )
    {
        //printf("Hovered\n");
        main_menu->help_button.col_outline.r = 255;
        main_menu->help_button.col_outline.g = 255;
        main_menu->help_button.col_outline.b = 255;
        main_menu->help_button.state = Default;
    }
    // Selected
    else if( main_menu->help_button.state == IsSelected )
    {
        //printf("Selected\n");
        main_menu->help_button.col_outline.r = 0;
        main_menu->help_button.col_outline.g = 255;
        main_menu->help_button.col_outline.b = 255;
        main_menu->help_button.state = Default;
    }

    // Default (Cursor not in rect)
    if( main_menu->quit_button.state == Default )
    {
        //printf("Default\n");
        main_menu->quit_button.col_outline.r = 128;
        main_menu->quit_button.col_outline.g = 128;
        main_menu->quit_button.col_outline.b = 128;
        //main_menu->quit_button.state = Default;
    }
    // Hovered
    else if( main_menu->quit_button.state == IsHovered )
    {
        //printf("Hovered\n");
        main_menu->quit_button.col_outline.r = 255;
        main_menu->quit_button.col_outline.g = 255;
        main_menu->quit_button.col_outline.b = 255;
        main_menu->quit_button.state = Default;
    }
    // Selected
    else if( main_menu->quit_button.state == IsSelected )
    {
        //printf("Selected\n");
        main_menu->quit_button.col_outline.r = 0;
        main_menu->quit_button.col_outline.g = 255;
        main_menu->quit_button.col_outline.b = 255;
        main_menu->quit_button.state = Default;
    }

    //printf("Rect0 x: %d, Rect y: %d, Rect w: %d, Rect h: %d\n", main_menu->start_rect.x, main_menu->start_rect.y, main_menu->start_rect.w, main_menu->start_rect.h);
    //printf("Rect1 x: %d, Rect y: %d, Rect w: %d, Rect h: %d\n", score_rect_1.x, score_rect_1.y, score_rect_1.w, score_rect_1.h);
    //printf("Rect2 x: %d, Rect y: %d, Rect w: %d, Rect h: %d\n", score_rect_2.x, score_rect_2.y, score_rect_2.w, score_rect_2.h);

    DrawRectangle( img, &main_menu->start_rect, main_menu->start_button.col_outline );
    DrawRectangle( img, &main_menu->help_rect, main_menu->help_button.col_outline );
    DrawRectangle( img, &main_menu->quit_rect, main_menu->quit_button.col_outline );
}

void UpdateMainMenu( MainMenu* main_menu, int* result, int mouse_x, int mouse_y, uint8_t mousestate )
{
    if( IsWithin( &main_menu->start_rect, mouse_x, mouse_y ) )
    {
        if( mousestate == MOUSE_LDOWN )
        {
            //printf( "BEFORE Button State: %d\n", main_menu->start_button.state );
            main_menu->start_button.state = IsSelected;
            *result = GamePlaying;
            //return GamePlaying;
            //printf( "BEFORE Button State: %d\n", main_menu->start_button.state );
        }
        else
        {
            //printf( "BEFORE Button State: %d\n", main_menu->start_button.state );
            main_menu->start_button.state = IsHovered;
            //printf( "AFTER Button State: %d\n", main_menu->start_button.state );
        }
        if( mousestate == MOUSE_LUP )
        {
            main_menu->start_button.state = IsHovered;
        }
    }
    else { main_menu->start_button.state = 0; }// Default (Cursor not in rect)

    if( IsWithin( &main_menu->help_rect, mouse_x, mouse_y ) )
    {
        if( mousestate == MOUSE_LDOWN )
        {
            //printf( "BEFORE Button State: %d\n", main_menu->help_button.state );
            main_menu->help_button.state = IsSelected;
            //*result = //HELPMENU;
            //printf( "BEFORE Button State: %d\n", main_menu->help_button.state );
        }
        else
        {
            //printf( "BEFORE Button State: %d\n", main_menu->help_button.state );
            main_menu->help_button.state = IsHovered;
            //printf( "AFTER Button State: %d\n", main_menu->help_button.state );
        }
        if( mousestate == MOUSE_LUP )
        {
            main_menu->help_button.state = IsHovered;
        }
    }
    else { main_menu->help_button.state = 0; }

    if( IsWithin( &main_menu->quit_rect, mouse_x, mouse_y ) )
    {
        if( mousestate == MOUSE_LDOWN )
        {
            //printf( "BEFORE Button State: %d\n", main_menu->quit_button.state );
            main_menu->quit_button.state = IsSelected;
            *result = GameQuit;
            //return GameQuit;
            //printf( "BEFORE Button State: %d\n", main_menu->quit_button.state );
        }
        else
        {
            //printf( "BEFORE Button State: %d\n", main_menu->quit_button.state );
            main_menu->quit_button.state = IsHovered;
            //printf( "AFTER Button State: %d\n", main_menu->quit_button.state );
        }
        if( mousestate == MOUSE_LUP )
        {
            main_menu->quit_button.state = IsHovered;
        }
    }
    else { main_menu->quit_button.state = Default; }

    //return GameMenu;
}

int MenuAction( MainMenu* main_menu )
{
    if( main_menu->start_button.state == IsSelected )
    {
        main_menu->start_button.state = Default;
        return 1;
    }
    else if( main_menu->help_button.state == IsSelected )
    {

    }
    else if( main_menu->quit_button.state == IsSelected )
    {
        main_menu->quit_button.state = Default;
        return 3;
    }

    return -1;
}
