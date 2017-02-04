
#ifndef SCREEN_H
#define SCREEN_H

// WINAPI RENDERING
int GetBitmapRect( const Bitmap* img, Rect* out );
int GetDrawableRect( HWND win_handle, Rect* out );

int InitializeWindowImage( HWND win_handle, Bitmap* screen_img );
int ResizeWindowImage( HWND win_handle, Bitmap* screen_img );

void UpdateWindowImage( HDC device_context, const Bitmap* img, const Rect* srcrect, const Rect* dstrect );

#endif
