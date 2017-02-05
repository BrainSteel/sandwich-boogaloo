#include "windows.h"

#include "stdlib.h"

#include "pixel.h"
#include "common.h"

// The global variable for the screen's buffer
Bitmap screen;

int GetBitmapRect( const Bitmap* img, Rect* out )
{
    if ( !img || !out )
    {
        return 0;
    }

    out->x = 0;
    out->y = 0;

    out->w = img->w;
    out->h = img->h;

    return 1;
}

int GetDrawableRect( HWND win_handle, Rect* out )
{
    RECT client_rect;
    if ( GetClientRect( win_handle, &client_rect ))
    {
        int screen_width = client_rect.right - client_rect.left;
        int screen_height = client_rect.bottom - client_rect.top;

        if ( out )
        {
            out->x = client_rect.left;
            out->y = client_rect.top;
            out->w = screen_width;
            out->h = screen_height;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        sprintf( error_buf, "Error getting client Rect: %lx", GetLastError( ));
        MessageBoxA( 0, error_buf, 0, MB_OK );
        return 0;
    }
    return 1;
}

int ResizeWindowImage( HWND win_handle, Bitmap* screen_img )
{
    Rect draw;
    if ( GetDrawableRect( win_handle, &draw ))
    {
        return ResizeImageMemory( screen_img, draw.w, draw.h );
    }
    else
    {
        sprintf( error_buf, "Error getting client rect: %lx", GetLastError( ));
        MessageBoxA( 0, error_buf, 0, MB_OK );
        return 0;
    }
}

void UpdateWindowImage( HDC device_context, const Bitmap* img, const Rect* srcrect, const Rect* dstrect )
{
    if ( !img )
    {
        return;
    }

    Rect src_local;
    if ( !srcrect )
    {
        GetBitmapRect( img, &src_local );
    }
    else
    {
        src_local = *srcrect;
    }

    Rect dst_local;
    if ( !dstrect )
    {
        RECT result;
        // Attempt to get the client Rect of the window if unspecified
        GetClientRect( WindowFromDC( device_context ), &result );
        dst_local.x = result.left;
        dst_local.y = result.top;
        dst_local.w = result.right - result.left;
        dst_local.h = result.bottom - result.top;
    }
    else
    {
        dst_local = *dstrect;
    }

    int bits_result = StretchDIBits( device_context,
                        dst_local.x, dst_local.y, dst_local.w, dst_local.h,
                        src_local.x, src_local.y, src_local.w, src_local.h,
                        img->pixels, &img->info,
                        DIB_RGB_COLORS, SRCCOPY );

    if ( bits_result <= 0 )
    {
        sprintf( error_buf, "Stretch DI Bits failed! Error: %lx", GetLastError( ));
        MessageBoxA( 0, error_buf, 0, MB_OK );
    }
}
