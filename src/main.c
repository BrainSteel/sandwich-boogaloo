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

void Paint( Bitmap* target, HWND win_handle, UINT message, int x, int y, char* win_type );

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
            state.in.mousex = GET_X_LPARAM( lparam );
            state.in.mousey = GET_Y_LPARAM( lparam );
            UpdateMainMenu( &state.main_menu, state.in.mousex, state.in.mousey, state.in.mousestate );
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
            Paint( &main_menu, win_handle, message, x, y, "MainMenu" );
            // Render the frame, interpolating partial frames between logical frames.
            HDC device_context = GetDC( win_handle );
            if ( device_context )
            {
                Rect game_rect;
                game_rect.x = 0;
                game_rect.y = 0;
                game_rect.w = screen.w - 380;
                game_rect.h = screen.h;
                ImageBlit( &state.textures.beach, &screen, NULL, 0, 0 );
                RenderGameState( &screen, &game_rect, &state, 0 );

                UpdateWindowImage( device_context, &screen, NULL, NULL );

                char fpsbuf[32];
                sprintf( fpsbuf, "FPS: %u", state.fps );

                RECT textrect;
                textrect.left = screen.w - 20;
                textrect.top = 0;
                textrect.right = screen.w - 1;
                textrect.bottom = 19;

                DrawTextA( device_context, fpsbuf, -1, &textrect, DT_RIGHT | DT_TOP | DT_NOCLIP );

                ReleaseDC( win_handle, device_context );
            }
        }
        break;

        case WM_LBUTTONDOWN:
        {
            state.in.mousestate = MOUSE_LDOWN;
        }
        break;

        case WM_KEYDOWN:
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
        break;

        case WM_KEYUP:
        {
            switch( wparam )
            {
                case 0x57:
                    state.in[0].keydown[KeyUp] = 0;
                break;

                case 0x53:
                    state.in[0].keydown[KeyDown] = 0;
                break;

                case 0x41:
                    state.in[0].keydown[KeyLeft] = 0;
                break;

                case 0x44:
                    state.in[0].keydown[KeyRight] = 0;
                break;

                default:
                break;
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

void Paint( Bitmap* target, HWND win_handle, UINT message, int mouse_x, int mouse_y, char* win_type )
{
    HDC device_context = GetDC( win_handle );
    ClearBitmap( target, BLACK );
    RGB col;
    col.r = 255;
    col.g = 0;
    col.b = 0;

    Rect rect;
    rect.x = 1280 - 380;
    rect.y = 0;
    rect.w = 380;
    rect.h = 720;

    // Replace this
    if( !strcmp( win_type, "MainMenu" ) )
    {
        DisplayMainMenu( device_context, &state.main_menu, &main_menu, mouse_x, mouse_y );
    }
    else if( !strcmp( win_type, "GameScreen" ) )
    {
        FillRectangle( target, &rect, col );
    }


    SetTextColor( device_context, RGB(255, 0, 0) );

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
    printf("Bottom: %ld\n", start_rect_text.bottom);

    UpdateWindowImage( device_context, target, NULL, NULL );

    DrawText( device_context, start_str, -1, &start_rect_text, DT_NOCLIP );
    DrawText( device_context, help_str, -1, &help_rect_text, DT_NOCLIP );
    DrawText( device_context, quit_str, -1, &quit_rect_text, DT_NOCLIP );

    ReleaseDC( win_handle, device_context );
}

int CALLBACK WinMain( HINSTANCE instance, HINSTANCE prev, LPSTR cmdline, int cmdshow )
{
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

    CreateMainMenu( &state.main_menu, &main_menu );

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

    // We need to register the window class in order to use it.
    if ( RegisterClass( &window_class ))
    {
        CreateImage( &screen, SCREEN_WIDTH, SCREEN_HEIGHT );
        CreateImage( &main_menu, SCREEN_WIDTH, SCREEN_HEIGHT );

        // Having registered the class, we can now create a window with it.
        HWND win_handle = CreateWindowEx( 0, "SandwichWindowClass", "Sandwich", WS_VISIBLE | WS_OVERLAPPEDWINDOW,
                                                                CW_USEDEFAULT, CW_USEDEFAULT,
                                                                SCREEN_WIDTH, SCREEN_HEIGHT, NULL, NULL, instance, NULL );
                                                                
        // win_handle will be 0 if we failed to create the window.
        if ( win_handle )
        {

            if ( !LoadImageFromFile( "BG_Beach.bmp", &state.textures.beach ))
            {
                MessageBoxA( 0, "Failed to open BG_Beach.bmp", 0, MB_OK );
                return 1;
            }

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
                    // TODO: CALL PAINT HERE
                    // Render the frame, interpolating partial frames between logical frames.
                    HDC device_context = GetDC( win_handle );
                    if ( device_context )
                    {
                        Rect game_rect;
                        game_rect.x = 0;
                        game_rect.y = 0;
                        game_rect.w = screen.w - 380;
                        game_rect.h = screen.h;
                        ImageBlit( &state.textures.beach, &screen, NULL, 0, 0 );
                        RenderGameState( &screen, &game_rect, &state, 0 );

                        UpdateWindowImage( device_context, &screen, NULL, NULL );

                        char fpsbuf[32];
                        sprintf( fpsbuf, "FPS: %u", state.fps );

                        RECT textrect;
                        textrect.left = screen.w - 20;
                        textrect.top = 0;
                        textrect.right = screen.w - 1;
                        textrect.bottom = 19;

                        DrawTextA( device_context, fpsbuf, -1, &textrect, DT_RIGHT | DT_TOP | DT_NOCLIP );

                        ReleaseDC( win_handle, device_context );
                    }

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
