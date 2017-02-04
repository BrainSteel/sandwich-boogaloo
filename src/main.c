#include "stdio.h"

#include "windows.h"

#include "common.h"
#include "pixel.h"
#include "screen.h"

Bitmap screen;
char error_buf[256];

LRESULT CALLBACK WindowProcedure( HWND win_handle, UINT message, WPARAM wparam, LPARAM lparam )
{
    // DefWindowProc is Windows' default action for whatever message is received
    LRESULT result = DefWindowProc( win_handle, message, wparam, lparam );

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
            ResizeWindowImage( win_handle, &screen );
        break;

        // The WM_PAINT message indicates we should draw
        // to the screen.
        case WM_PAINT:
        {
            HDC device_context = GetDC( win_handle );
            ClearBitmap( &screen, BLACK );
            RGB col;
            col.r = 255;
            col.g = 0;
            col.b = 0;

            Rect rect;
            rect.x = 20;
            rect.y = 60;
            rect.w = 20;
            rect.h = 10;

            FillRectangle( &screen, &rect, col );
            UpdateWindowImage( device_context, &screen, NULL, NULL );
            ReleaseDC( win_handle, device_context );
        }
        break;

        case WM_LBUTTONDOWN:
        {

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

int CALLBACK WinMain( HINSTANCE instance, HINSTANCE prev, LPSTR cmdline, int cmdshow )
{
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
        CreateImage( &screen, 720, 720 );

        // Having registered the class, we can now create a window with it.
        HWND win_handle = CreateWindowEx( 0, "SandwichWindowClass", "Sandwich", WS_VISIBLE | WS_OVERLAPPEDWINDOW,
                                                                CW_USEDEFAULT, CW_USEDEFAULT,
                                                                720, 720, NULL, NULL, instance, NULL );
        // win_handle will be 0 if we failed to create the window.
        if ( win_handle )
        {

            if ( !ResizeWindowImage( win_handle, &screen ))
            {
                sprintf( error_buf, "Uh oh" );
                MessageBoxA( 0, error_buf, 0, MB_OK );
                return 1;
            }

            MSG message;
            int running = 1;
            while ( running )
            {
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

                if ( !running )
                {
                    break;
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

    return 0;
}
