#include "stdio.h"

#include "windows.h"
#include "windowsx.h"

#include "common.h"
#include "pixel.h"
#include "screen.h"
#include "gamestate.h"
#include "menu.h"
#include "string.h"
#include "pool.h"

Bitmap screen;
Bitmap main_menu;
GameState state;

char error_buf[256];

void Paint( Bitmap* target, HWND win_handle, int x, int y );
int DrawTimeBar( GameState* state );
void DrawNumericText( GameState* state, uint32_t score, uint32_t x_loc, uint32_t y_loc );

GameState state = {0};
char error_buf[256];

uint64_t GetTickFrequency( )
{
    LARGE_INTEGER freq;
    // On Windows XP and later, this function is guaranteed to return successfully.
    QueryPerformanceFrequency( &freq );
    return freq.QuadPart;
}

uint64_t GetTicks( )
{
    LARGE_INTEGER count;
    // On Windows XP and later, this function is guaranteed to return successfully.
    QueryPerformanceCounter( &count );
    return count.QuadPart;
}

LRESULT CALLBACK WindowProcedure( HWND win_handle, UINT message, WPARAM wparam, LPARAM lparam )
{
    // DefWindowProc is Windows' default action for whatever message is received
    LRESULT result = DefWindowProc( win_handle, message, wparam, lparam );

    int x = 0;
    int y = 0;

    switch ( message )
    {
        // If we get a WM_DESTROY or a WM_CLOSE message,
        // we quit the program by posting a quit message.
        // This will be detected in WinMain( ), which will exit.
        case WM_DESTROY:
        case WM_CLOSE:
            PostQuitMessage( 0 );
        break;

        case WM_SIZE:
            if ( wparam == SIZE_MINIMIZED )
            {
                state.rendering_enabled = 0;
            }
            else
            {
                state.rendering_enabled = 1;
                ResizeWindowImage( win_handle, &screen );
            }
        break;

        case WM_MOUSEMOVE:
        {
            state.in[0].mousex = GET_X_LPARAM( lparam );
            state.in[0].mousey = GET_Y_LPARAM( lparam );
            //state.game_mode = UpdateMainMenu( &state.main_menu, state.in[0].mousex, state.in[0].mousey, state.in[0].mousestate );
            //UpdateMainMenu( &state.main_menu, state.in[0].mousex, state.in[0].mousey, state.in[0].mousestate );
        }
        break;

        case WM_GETMINMAXINFO:
        {
            MINMAXINFO* mmi = (MINMAXINFO*)lparam;
            mmi->ptMinTrackSize.x = 640;
            mmi->ptMinTrackSize.y = 360;
            result = 0;
        }

        // The WM_PAINT message indicates we should draw
        // to the screen.
        case WM_PAINT:
        {
            Paint( &main_menu, win_handle, x, y );
        }
        break;

        case WM_LBUTTONDOWN:
        {
            state.in[0].mousestate = MOUSE_LDOWN;
        }
        break;

        case WM_KEYDOWN:
        {
            // If the 30th bit of lparam is set, then the key was already down before this message.
            // We are only interested in the initial keypress, not what happens when it is held.
            if ( !(lparam & (1 << 30)))
            {
                switch( wparam )
                {
                    case 0x57:
                        state.in[0].keydown[KeyUp] = 1;
                    break;

                    case 0x53:
                        state.in[0].keydown[KeyDown] = 1;
                    break;

                    case 0x41:
                        state.in[0].keydown[KeyLeft] = 1;
                    break;

                    case 0x44:
                        state.in[0].keydown[KeyRight] = 1;
                    break;

                    default:
                    break;
                }
            }
        }
        break;

        case WM_SETCURSOR:
        {
            SetCursor( LoadCursor( 0, IDC_ARROW ));
        }

        default:
        break;
    }

    return result;
}

void Paint( Bitmap* target, HWND win_handle, int mouse_x, int mouse_y )
{
    //int result;
    //UpdateMainMenu( &state.main_menu, &result, state.in[0].mousex, state.in[0].mousey, state.in[0].mousestate );

    /*switch(result)
    {
        case 0:
            break;
        case 1:
            // TODO: HERE HERE HERE
    }*/

    HDC device_context = GetDC( win_handle );

    if( 0)//state.game_mode == GameMenu )
    {
        target = &main_menu;
        ClearBitmap( target, BLACK );
        DisplayMainMenu( device_context, &state.main_menu, &main_menu, mouse_x, mouse_y );
    }
    else if( 1)//state.game_mode == GamePlaying )
    {
        target = &screen;
        ClearBitmap( target, BLACK );
        if ( device_context )
        {
            Rect game_rect;
            game_rect.x = 0;
            game_rect.y = 0;
            game_rect.w = screen.w - 380;
            game_rect.h = screen.h;
            ImageBlit( &state.textures.beach, &screen, NULL, 0, 0 );
            RenderGameState( &screen, &game_rect, &state, 0 );

            Rect HUD_rect;
            HUD_rect.x = game_rect.w;
            HUD_rect.y = 0;
            HUD_rect.w = 380;
            HUD_rect.h = screen.h;

            RGB col;
            col.r = 0;
            col.g = 0;
            col.b = 0;

            FillRectangle( &screen, &HUD_rect, col );
            DrawTimeBar( &state );

            UpdateWindowImage( device_context, &screen, NULL, NULL );

            char fpsbuf[32];
            sprintf( fpsbuf, "FPS: %u", state.fps );

            RECT textrect;
            textrect.left = screen.w - 20;
            textrect.top = 0;
            textrect.right = screen.w - 1;
            textrect.bottom = 19;

            DrawTextA( device_context, fpsbuf, -1, &textrect, DT_RIGHT | DT_TOP | DT_NOCLIP );

            char scorebuf[32];
            sprintf( scorebuf, "SCORE: %u", state.score );

            RECT HUD_score_rect;
            HUD_score_rect.left = HUD_rect.x;
            HUD_score_rect.top = 200;
            HUD_score_rect.right = screen.w - HUD_rect.w / 2;
            HUD_score_rect.bottom = 20;

            DrawTextA( device_context, scorebuf, -1, &HUD_score_rect, DT_RIGHT | DT_TOP | DT_NOCLIP );

            ReleaseDC( win_handle, device_context );
        }
    }


    /*SetTextColor( device_context, RGB(255, 0, 0) );

    LPCTSTR start_str = "START";
    LPCTSTR help_str = "HELP";
    LPCTSTR quit_str = "QUIT";

    RECT start_rect_text;
    start_rect_text.left = state.main_menu.start_rect.x + WIDGET_X_TEXT_BUFFER;
    start_rect_text.top = state.main_menu.start_rect.y + WIDGET_Y_TEXT_BUFFER;
    start_rect_text.right = state.main_menu.start_rect.x + state.main_menu.start_rect.w;
    start_rect_text.bottom = state.main_menu.start_rect.y + state.main_menu.start_rect.h;

    RECT help_rect_text;
    help_rect_text.left = state.main_menu.help_rect.x + WIDGET_X_TEXT_BUFFER;
    help_rect_text.top = state.main_menu.help_rect.y + WIDGET_Y_TEXT_BUFFER;
    help_rect_text.right = state.main_menu.help_rect.x + state.main_menu.help_rect.w;
    help_rect_text.bottom = state.main_menu.help_rect.y + state.main_menu.help_rect.h;

    RECT quit_rect_text;
    quit_rect_text.left = state.main_menu.quit_rect.x + WIDGET_X_TEXT_BUFFER;
    quit_rect_text.top = state.main_menu.quit_rect.y + WIDGET_Y_TEXT_BUFFER;
    quit_rect_text.right = state.main_menu.quit_rect.x + state.main_menu.quit_rect.w;
    quit_rect_text.bottom = state.main_menu.quit_rect.y + state.main_menu.quit_rect.h;

    printf("Left: %ld\n", start_rect_text.left);
    printf("Top: %ld\n", start_rect_text.top);
    printf("Right: %ld\n", start_rect_text.right);
    printf("Bottom: %ld\n", start_rect_text.bottom);*/

    UpdateWindowImage( device_context, target, NULL, NULL );

    //DrawText( device_context, start_str, -1, &start_rect_text, DT_NOCLIP );
    //DrawText( device_context, help_str, -1, &help_rect_text, DT_NOCLIP );
    //DrawText( device_context, quit_str, -1, &quit_rect_text, DT_NOCLIP );

    //state.game_mode = MenuAction( &state.main_menu );

    ReleaseDC( win_handle, device_context );
}

int CALLBACK WinMain( HINSTANCE instance, HINSTANCE prev, LPSTR cmdline, int cmdshow )
{

//    PlaySoundA("Ring05.wav", NULL, SND_LOOP | SND_ASYNC);

    // Get the frequency of the high performance timer
    uint64_t tickfreq = GetTickFrequency( );

    // WNDCLASS is a structure that defines a reusable class for a window.
    WNDCLASS window_class = {0};

    // These class styles don't really matter, and seem to be archaic. We will use them anyway for historical purposes.
    window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;

    // The WindowProcedure we defined above is added to the class.
    window_class.lpfnWndProc = WindowProcedure;

    // The instance of the application this class belongs to is the same instance that we are passed
    window_class.hInstance = instance;

    // A name for the class, which is not user-visible and is unfortunately necessary.
    window_class.lpszClassName = "SandwichWindowClass";

    //CreateMainMenu( &state.main_menu, &main_menu );

    // The icon (in the upper left-hand corner) for the windows we will make is
    // loaded as an image. MAKEINTRESOURCE( ICON ) identifies the image to load,
    // which is packaged with the executable itself (which we have a handle to with 'instance') when we link with the compiled resource.rc file.
    window_class.hIcon = (HICON) LoadImage( instance, MAKEINTRESOURCE( ICON ), IMAGE_ICON, 0, 0,
                                                                    LR_DEFAULTCOLOR | LR_DEFAULTSIZE );
    if ( !window_class.hIcon )
    {
        // If we failed to load the icon, pop up a message box with an error code.
        sprintf( error_buf, "Failed to load icon: %lx", GetLastError( ));
        MessageBoxA( 0, error_buf, 0, MB_OK  );
        return 1;
    }

    window_class.hCursor = LoadCursor( instance, IDC_ARROW );

    // NOTE: DIFFICULTY
    state.difficulty = EXTREME;

    // We need to register the window class in order to use it.
    if ( RegisterClass( &window_class ))
    {
        CreateImage( &screen, SCREEN_WIDTH, SCREEN_HEIGHT );
        CreateImage( &main_menu, SCREEN_WIDTH, SCREEN_HEIGHT );

        // Having registered the class, we can now create a window with it.
        HWND win_handle = CreateWindowEx( 0, "SandwichWindowClass", "Sandwich", WS_VISIBLE | WS_OVERLAPPEDWINDOW,
                                                                CW_USEDEFAULT, CW_USEDEFAULT,
                                                                SCREEN_WIDTH + SCREEN_WIDTH_OFFSET, SCREEN_HEIGHT + SCREEN_HEIGHT_OFFSET, NULL, NULL, instance, NULL );

        // win_handle will be 0 if we failed to create the window.
        if ( win_handle )
        {

            if ( !LoadImageFromFile( "BG_Beach.bmp", &state.textures.beach ))
            {
                MessageBoxA( 0, "Failed to open BG_Beach.bmp", 0, MB_OK );
                return 1;
            }

            /* ( !LoadImageFromFile( "Spr_BreadSlice.bmp", &state.textures.bread ))
            {
                MessageBoxA( 0, "Failed to open Spr_BreadSlice.bmp", 0, MB_OK );
                return 1;
            }

            if ( !LoadImageFromFile( "Spr_Lettuce.bmp", &state.textures.lettuce ))
            {
                MessageBoxA( 0, "Failed to open Spr_Lettuce.bmp", 0, MB_OK );
                return 1;
            }

            if ( !LoadImageFromFile( "Spr_Tomato.bmp", &state.textures.tomato ))
            {
                MessageBoxA( 0, "Failed to open Spr_Tomato.bmp", 0, MB_OK );
                return 1;
            }

            if ( !LoadImageFromFile( "Spr_Quadropus.bmp", &state.textures.quadropus ))
            {
                MessageBoxA( 0, "Failed to open Spr_Quadropus.bmp", 0, MB_OK );
                return 1;
            }

            if ( !LoadImageFromFile( "NumbersText.bmp", &state.textures.numbers ))
            {
            MessageBoxA( 0, "Failed to open NumbersText.bmp", 0, MB_OK );
            return 1;
            }

            if ( !LoadImageFromFile( "Spr_Cresent.bmp", &state.textures.cresent ))
            {
                MessageBoxA( 0, "Failed to open Spr_Cresent.bmp", 0, MB_OK );
                return 1;
            }*/

            if ( !ResizeWindowImage( win_handle, &screen ))
            {
                sprintf( error_buf, "Failed to resize the window." );
                MessageBoxA( 0, error_buf, 0, MB_OK );
                return 1;
            }

            InitializeGameState( &state );

            MSG message;
            int running = 1;
            uint64_t last = GetTicks( );
            uint32_t renders_this_second = 0;
            while ( running )
            {
                uint64_t start_ticks = GetTicks( );

                // Get a message from the window
                while ( PeekMessage( &message, 0, 0, 0, PM_REMOVE ))
                {
                    if ( message.message == WM_QUIT )
                    {
                        running = 0;
                        break;
                    }
                    else
                    {
                        TranslateMessage( &message );
                        DispatchMessage( &message );
                    }
                }

                UpdateGameState( &state, 1.0f );
                state.logical_frames++;

                if ( !running )
                {
                    break;
                }

                // Enter a rendering loop until
                // enough time has passed to necessitate another logical update
                uint64_t ticks_passed = GetTicks( ) - start_ticks;
                float frames_passed = (ticks_passed * FRAMERATE) / (float)tickfreq;
                while ( frames_passed < 1.0f )
                {
                    Paint( &main_menu, win_handle, state.in[0].mousex, state.in[0].mousey );

                    uint64_t now = GetTicks( );
                    ticks_passed = now - start_ticks;
                    frames_passed = (ticks_passed * FRAMERATE) / (float)tickfreq;

                    if ( now - last > tickfreq )
                    {
                        state.fps = renders_this_second;
                        renders_this_second = 0;
                        last = now;
                    }

                    renders_this_second++;
                }
                DrawTimeBar( &state );
                Sleep( 10 );
            }
        }
        else
        {
            sprintf( error_buf, "Failed to create the window: %lx", GetLastError( ));
            MessageBoxA( 0, error_buf, 0, MB_OK );
            return 1;
        }
    }
    else
    {
        sprintf( error_buf, "Failed to register the window class: %lx", GetLastError( ));
        MessageBoxA( 0, error_buf, 0, MB_OK );
        return 1;
    }

    DestroyMemoryPool( state.pool );
    DestroyImage( &screen );

    return 0;
}

void DisplayHUDSprites( GameState* state )
{

}

int DrawTimeBar( GameState* state )
{
    RGB col;
    col.r = 255;
    col.g = 0;
    col.b = 0;

    Rect timer_rect;
    timer_rect.x = screen.w - 360;
    timer_rect.y = 400;

    if( state->difficulty == EASY )
    { timer_rect.w = 340 - ( ( state->logical_frames + FRAMERATE) / FRAMERATE ); }

    if( state->difficulty == MEDIUM )
    { timer_rect.w = 340 - 4 * ( ( state->logical_frames + FRAMERATE) / FRAMERATE ); }

    if( state->difficulty == HARD )
    { timer_rect.w = 340 - 6 * ( ( state->logical_frames + FRAMERATE) / FRAMERATE ); }

    if( state->difficulty == EXTREME )
    { timer_rect.w = 340 - 10 * ( ( state->logical_frames + FRAMERATE) / FRAMERATE ); }

    timer_rect.h = 30;

    if( timer_rect.w <= 0 )
    {
        printf("HERE");
        return 0;
    }

    FillRectangle( &screen, &timer_rect, col );

    return 1;
}


void DrawNumericText( GameState* state, uint32_t score, uint32_t x_loc, uint32_t y_loc )
{
    char scorebuf[4];
    sprintf( scorebuf, "%d", score );

    // '0', the legend her?self
    Rect score_rect_default;
    score_rect_default.x = 0;
    score_rect_default.y = 0;
    score_rect_default.w = 20;
    score_rect_default.h = 32;

    // To make sure we print only valid values
    // NOTE: Maybe do 000# and print values regardless
    Rect score_rect_0;
    if( 0x29 < scorebuf[0] && scorebuf[0] < 0x40 )
    {
        score_rect_0.x = ( scorebuf[0] - '0' ) * 20;
        score_rect_0.y = 0;
        score_rect_0.w = 20;
        score_rect_0.h = 32;
    }
    else
    {
        score_rect_0 = score_rect_default;
    }

    Rect score_rect_1;
    if( 0x29 < scorebuf[1] && scorebuf[1] < 0x40 )
    {
        score_rect_1.x = ( scorebuf[1] - '0' ) * 20;
        score_rect_1.y = 0;
        score_rect_1.w = 20;
        score_rect_1.h = 32;
    }
    else
    {
        score_rect_1 = score_rect_default;
    }

    Rect score_rect_2;
    if( 0x29 < scorebuf[2] && scorebuf[2] < 0x40 )
    {
        score_rect_2.x = ( scorebuf[2] - '0' ) * 20;
        score_rect_2.y = 0;
        score_rect_2.w = 20;
        score_rect_2.h = 32;
    }
    else
    {
        score_rect_2 = score_rect_default;
    }

    Rect score_rect_3;
    if( 0x29 < scorebuf[3] && scorebuf[3] < 0x40 )
    {
        score_rect_3.x = ( scorebuf[3] - '0' ) * 20;
        score_rect_3.y = 0;
        score_rect_3.w = 20;
        score_rect_3.h = 32;
    }
    else
    {
        score_rect_3 = score_rect_default;
    }

    ImageBlit( &state->textures.numbers, &screen, &score_rect_0, x_loc, y_loc );
    ImageBlit( &state->textures.numbers, &screen, &score_rect_1, x_loc + 20, y_loc );
    ImageBlit( &state->textures.numbers, &screen, &score_rect_2, x_loc + 20, y_loc );
    ImageBlit( &state->textures.numbers, &screen, &score_rect_3, x_loc + 20, y_loc );
}
