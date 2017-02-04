
#include "pixel.h"
#include "common.h"

int CreateImage( Bitmap* img, uint32_t w, uint32_t h )
{
    img->pixels = 0;
    img->w = 0;
    img->h = 0;
    img->info.bmiHeader.biSize = sizeof( img->info.bmiHeader );
    img->info.bmiHeader.biPlanes = 1;
    img->info.bmiHeader.biBitCount = BITS_PER_PIXEL;
    img->info.bmiHeader.biCompression = BI_RGB;

    return ResizeImage( img, w, h );
}

void FillRectangle( Bitmap* img, const Rect* dst, RGB col )
{
    if ( !img )
    {
        return;
    }

    Rect dst_local;
    if ( dst )
    {
        dst_local = *dst;
    }
    else
    {
        dst_local.x = 0;
        dst_local.y = 0;
        dst_local.w = img->w;
        dst_local.h = img->h;
    }

    int x, y;
    void* pixel_row = img->pixels + dst_local.x + dst_local.y * img->w;
    for ( y = 0; y < dst_local.h; y++ )
    {
        void* pixel = pixel_row;
        for ( x = 0; x < dst_local.w; x++ )
        {
            WriteRGB( pixel, col.r, col.g, col.b );
            pixel += sizeof( uint32_t );
        }

        pixel_row += img->w * sizeof( uint32_t );
    }
}

int ResizeImage( Bitmap* img, uint32_t w, uint32_t h )
{
    if ( img->pixels )
    {
        DestroyImage( img );
    }

    img->info.bmiHeader.biWidth = w;
    img->info.bmiHeader.biHeight = -h;
    img->w = w;
    img->h = h;

    uint32_t image_size = BITS_PER_PIXEL * img->w * img->h;
    if ( image_size == 0 )
    {
        MessageBoxA( 0, "Cannot allocate 0 size pixel memory.", 0, MB_OK );
    }

    img->info.bmiHeader.biSizeImage = image_size;

    img->pixels = VirtualAlloc( NULL, image_size, MEM_COMMIT, PAGE_READWRITE );
    if ( !img->pixels )
    {
        MessageBoxA( 0, "Failed to allocate pixel memory.", 0, MB_OK );
        return 0;
    }
    else
    {
        memset( img->pixels, 0, image_size );
    }

    return 1;
}

typedef struct BMPFileHeader
{
    char type[2];
    uint32_t fsize;

    // NOTE: in the file header, there are two
    // reserved words between fsize and bit_start.
    // We do not need to read these words.

    uint32_t bit_start;
} BMPFileHeader;

typedef struct BMPMask
{
    uint32_t rmask, gmask, bmask, amask;
    uint32_t rshift, gshift, bshift, ashift;
    uint32_t rmax, gmax, bmax, amax;
} BMPMask;

typedef struct BMP3Header
{
    uint32_t header_size;
    int32_t w, h;
    uint16_t planes;
    uint16_t bits_pp;
    uint32_t compression;
    uint32_t bmpsize;

    // These values are in pixels per meter,
    // so aren't very useful for our purposes.
    // We will use the w and h instead.
    int32_t hres, vres;

    uint32_t col_used;
    uint32_t col_important;
    BMPMask bitmask;
} BMP3Header;

// If mask == 0, returns 0 and leaves index unchanged.
// If mask > 0, returns 1 and index is filled with the
// index of the least significant 1-bit of mask.
// See MSDN reference: https://msdn.microsoft.com/en-us/library/fbxyd7zd.aspx
// See GCC reference: https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
static int BitScanReverse( uint32_t* index, uint32_t mask )
{
    #ifdef MSVC
        // Untested
        #include "intrin.h"
        #pragma intrinsic( _BitScanReverse )
        return _BitScanReverse( index, mask );
    #else
        int res = __builtin_ffs( mask );
        if ( res == 0 )
        {
            return 0;
        }
        else
        {
            *index = res - 1;
            return 1;
        }
    #endif
}

static uint16_t SwapByteOrder16( uint16_t x )
{
    return ((x & 0xFF00) >> 8) |
           ((x & 0x00FF) << 8);
}

static uint32_t SwapByteOrder32( uint32_t x )
{
    return ((x & 0xFF000000) >> 24) |
           ((x & 0x00FF0000) >> 8)  |
           ((x & 0x0000FF00) << 8)  |
           ((x & 0x000000FF) << 24);
}

// Quick define to read a type from a stream of bytes.
// For our purposes, we are assuming the byte stream's endianness
// matches the native little-endian architecture (on x86 and x64)
// The macro reads a value of size sizeof(type) from the stream,
// and increments the offset so that this function may be called
// repeatedly.
#define READ_BYTESTREAM( type, ptr, off ) (*(type*)(ptr + off)); off += sizeof(type);

int LoadImageFromFile( const char* filename, Bitmap* img )
{
    // Reference for this function:
    // http://www.fileformat.info/format/bmp/egff.htm
    // What we support:
    // v3 Windows NT format for:
    //    - 24 bit RGB           (Compression = 0)
    //    - 16-bit with bitmasks (Compression = 3)
    //    - 32-bit with bitmasks (Compression = 3)
    // v4.x BMPs that are 16, 24, or 32-bit should
    // generally work.
    // Previous versions of the BMP file format are not supported.
    // We do not support RLE compression or any other
    // bit size for pixels.

    int returncode = 0;
    HANDLE file;
    char* pixelbuf = NULL;

// Bracket to prevent scope errors
{
    // First, clear the given image
    img->pixels = 0;
    img->w = 0;
    img->h = 0;

    // Reference: https://msdn.microsoft.com/en-us/library/windows/desktop/bb540534(v=vs.85).aspx
    file = CreateFile(filename,              // file to open
                      GENERIC_READ,          // open for reading
                      FILE_SHARE_READ,       // share for reading
                      NULL,                  // default security
                      OPEN_EXISTING,         // existing file only
                      FILE_ATTRIBUTE_NORMAL, // normal file
                      NULL);                 // no attr. template
    if ( file == INVALID_HANDLE_VALUE )
    {
        sprintf( error_buf, "Could not open file %s: %lx", filename, GetLastError( ));
        MessageBoxA( 0, error_buf, 0, MB_OK );
        goto stop;
    }

    // We first read the 14-byte header for the file
    // To figure out how large the file is.
    const uint32_t head_size = 14;
    char filehead_bytes[head_size];
    DWORD bytes_read = 0;

    // The following is a structure that keeps track of our place in the file.
    OVERLAPPED ol = {0};

    // Read the header
    if ( !ReadFile( file, &filehead_bytes, head_size, &bytes_read, &ol ) ||
                    bytes_read != head_size )
    {
        sprintf( error_buf, "Could not read file header of %s: %lx", filename, GetLastError( ));
        MessageBoxA( 0, error_buf, 0, MB_OK );
        goto stop;
    }
    ol.Offset += bytes_read;

    BMPFileHeader filehead;
    uint32_t offset = 0;

    // Read the file type (Should be "BM")
    filehead.type[0] = READ_BYTESTREAM( char, filehead_bytes, offset );
    filehead.type[1] = READ_BYTESTREAM( char, filehead_bytes, offset );
    if ( filehead.type[0] != 'B' || filehead.type[1] != 'M' )
    {
        sprintf( error_buf, "File type of %s does not match \"BM\"", filename );
        MessageBoxA( 0, error_buf, 0, MB_OK );
        goto stop;
    }

    // Read the size of the file
    filehead.fsize = READ_BYTESTREAM( DWORD, filehead_bytes, offset );

    // After the file size are two reserved WORDs
    offset += 2 * sizeof(WORD);

    // Read the location that image data begins.
    filehead.bit_start = READ_BYTESTREAM( DWORD, filehead_bytes, offset )

    offset = 0;
    BMP3Header bmphead;

    // Assume the BMP header is at least 40 bytes long.
    // If it is longer, that is okay, but if it's shorter,
    // it's a BMP version we do not support.
    const uint32_t bmphead_size = 40;
    char bmphead_bytes[bmphead_size];

    if ( !ReadFile( file, &bmphead_bytes, bmphead_size, &bytes_read, &ol ) ||
                    bytes_read != bmphead_size )
    {
        sprintf( error_buf, "Could not read file header of %s: %lx", filename, GetLastError( ));
        MessageBoxA( 0, error_buf, 0, MB_OK );
        goto stop;
    }
    ol.Offset += bytes_read;

    // Read the header size and ensure it is large enough.
    bmphead.header_size = READ_BYTESTREAM( DWORD, bmphead_bytes, offset );
    if ( bmphead.header_size < bmphead_size )
    {
        sprintf( error_buf, "BMP file format version of %s not supported.", filename );
        MessageBoxA( 0, error_buf, 0, MB_OK );
        goto stop;
    }

    // Read pixel formatting data
    bmphead.w = READ_BYTESTREAM( LONG, bmphead_bytes, offset );
    bmphead.h = READ_BYTESTREAM( LONG, bmphead_bytes, offset );
    bmphead.planes = READ_BYTESTREAM( WORD, bmphead_bytes, offset );
    bmphead.bits_pp = READ_BYTESTREAM( WORD, bmphead_bytes, offset );
    if ( bmphead.bits_pp != 16 &&
         bmphead.bits_pp != 24 &&
         bmphead.bits_pp != 32 )
    {
        sprintf( error_buf, "Bits per pixel of %hu for file %s not supported.",
                 bmphead.bits_pp, filename );
        MessageBoxA( 0, error_buf, 0, MB_OK );
        goto stop;
    }

    // Compression value must be 0 if bits_pp = 24, and 3 otherwise.
    bmphead.compression = READ_BYTESTREAM( DWORD, bmphead_bytes, offset );
    if ( bmphead.bits_pp == 24 )
    {
        if ( bmphead.compression != 0 )
        {
            sprintf( error_buf, "Compression of %u (bpp = %hu) for file %s is not supported.",
                     bmphead.compression, bmphead.bits_pp, filename );
            MessageBoxA( 0, error_buf, 0, MB_OK );
            goto stop;
        }
    }
    else if ( bmphead.compression != 3 )
    {
        sprintf( error_buf, "Compression of %u (bpp = %hu) for file %s is not supported.",
                 bmphead.compression, bmphead.bits_pp, filename );
        MessageBoxA( 0, error_buf, 0, MB_OK );
        goto stop;
    }

    bmphead.bmpsize = READ_BYTESTREAM( DWORD, bmphead_bytes, offset );

    // These values are in pixels per meter,
    // so aren't very useful for our purposes.
    // We will use the w and h instead.
    bmphead.hres = READ_BYTESTREAM( LONG, bmphead_bytes, offset );
    bmphead.vres = READ_BYTESTREAM( LONG, bmphead_bytes, offset );

    // These values do not really matter to us.
    bmphead.col_used = READ_BYTESTREAM( DWORD, bmphead_bytes, offset );
    bmphead.col_important = READ_BYTESTREAM( DWORD, bmphead_bytes, offset );

    // If there is a compression level of 3,
    // we read the bitmasks for r, g, and b as well.
    if ( bmphead.compression == 3 )
    {
        const uint32_t mask_size = 3 * sizeof(DWORD);
        char mask_bytes[mask_size];
        if ( !ReadFile( file, &mask_bytes, mask_size, &bytes_read, &ol ) ||
                        bytes_read != mask_size )
        {
            sprintf( error_buf, "Could not read bitmasks of %s: %lx", filename, GetLastError( ));
            MessageBoxA( 0, error_buf, 0, MB_OK );
            goto stop;
        }
        ol.Offset += bytes_read;

        offset = 0;
        bmphead.bitmask.rmask = READ_BYTESTREAM( DWORD, mask_bytes, offset );
        bmphead.bitmask.gmask = READ_BYTESTREAM( DWORD, mask_bytes, offset );
        bmphead.bitmask.bmask = READ_BYTESTREAM( DWORD, mask_bytes, offset );

        // Get the shift values for each color
        BitScanReverse( &bmphead.bitmask.rshift, bmphead.bitmask.rmask );
        BitScanReverse( &bmphead.bitmask.gshift, bmphead.bitmask.gmask );
        BitScanReverse( &bmphead.bitmask.bshift, bmphead.bitmask.bmask );

        // Get the maximum values for each color
        bmphead.bitmask.rmax = bmphead.bitmask.rmask >> bmphead.bitmask.rshift;
        bmphead.bitmask.gmax = bmphead.bitmask.gmask >> bmphead.bitmask.gshift;
        bmphead.bitmask.bmax = bmphead.bitmask.bmask >> bmphead.bitmask.bshift;
    }

    // Now, it is time to begin actually reading the bitmap data!
    // No matter how the image is stored on disk, we are going to
    // load it into our 32-bit RGB format.

    // The first step is to create the image
    uint32_t realw = bmphead.w < 0 ? -bmphead.w : bmphead.w;
    uint32_t realh = bmphead.h < 0 ? -bmphead.h : bmphead.h;
    if ( !CreateImage( img, realw, realh ))
    {
        MessageBoxA( 0, "Failed to create image.", 0, MB_OK );
        goto stop;
    }

    // If the height of the saved bitmap is positive,
    // it is saved "bottom-up" in the y direction.
    // Since all of our images are saved "top-down",
    // we must convert it if need be.
    int ydir = 1;
    int ystart = 0;
    if ( bmphead.h > 0 )
    {
        ydir = -1;
        ystart = realh - 1;
    }

    uint32_t bytes_pp = bmphead.bits_pp / 8;
    uint32_t pitch = bytes_pp * realw;
    uint32_t pitchmod = pitch % 4;
    if ( pitchmod )
    {
        // Each scanline is aligned on a 32-bit boundary
        pitch += 4 - pitchmod;
    }

    uint32_t bmp_size = realh * pitch;

    // Allocate a buffer large enough for the entire bitmap data
    pixelbuf = VirtualAlloc( NULL, bmp_size, MEM_COMMIT, PAGE_READWRITE );
    if ( !pixelbuf )
    {
        MessageBoxA( 0, "Failed to allocate pixel buffer.", 0, MB_OK );
        goto stop;
    }

    ol.Offset = filehead.bit_start;
    if ( !ReadFile( file, pixelbuf, bmp_size, &bytes_read, &ol ) ||
                    bytes_read != bmp_size )
    {
        sprintf( error_buf, "Could not read file contents of %s: %lx", filename, GetLastError( ));
        MessageBoxA( 0, error_buf, 0, MB_OK );
        goto stop;
    }

    int y;
    int x;
    void* read_row = pixelbuf + pitch * ystart;
    void* read_pixel = read_row;
    void* write_pixel = img->pixels;
    for ( y = 0; y < realh; y++ )
    {
        read_pixel = read_row;
        for ( x = 0; x < realw; x++ )
        {
            uint8_t r, g, b;
            switch (bmphead.bits_pp)
            {
                case 16:
                {
                    uint16_t pval = *(uint16_t*)read_pixel;

                    // If the bmp is in Windows NT v3 format,
                    // the pixels are stored in a big endian format,
                    // so we must swap their byte orders.
                    if ( bmphead.header_size == head_size )
                    {
                        pval = SwapByteOrder16( pval );
                    }

                    uint16_t rraw = (pval & bmphead.bitmask.rmask) >> bmphead.bitmask.rshift;
                    uint16_t graw = (pval & bmphead.bitmask.gmask) >> bmphead.bitmask.gshift;
                    uint16_t braw = (pval & bmphead.bitmask.bmask) >> bmphead.bitmask.bshift;

                    r = ((uint32_t)rraw * 0xFF) / bmphead.bitmask.rmax;
                    g = ((uint32_t)graw * 0xFF) / bmphead.bitmask.gmax;
                    b = ((uint32_t)braw * 0xFF) / bmphead.bitmask.bmax;
                }
                break;

                case 24:
                    r = *(char*)read_pixel;
                    g = *(char*)(read_pixel + 1);
                    b = *(char*)(read_pixel + 2);
                break;

                case 32:
                {
                    uint32_t pval = *(uint32_t*)read_pixel;

                    // If the bmp is in Windows NT v3 format,
                    // the pixels are stored in a big endian format,
                    // so we must swap their byte orders.
                    if ( bmphead.header_size == head_size )
                    {
                        pval = SwapByteOrder32( pval );
                    }

                    uint32_t rraw = (pval & bmphead.bitmask.rmask) >> bmphead.bitmask.rshift;
                    uint32_t graw = (pval & bmphead.bitmask.gmask) >> bmphead.bitmask.gshift;
                    uint32_t braw = (pval & bmphead.bitmask.bmask) >> bmphead.bitmask.bshift;

                    r = (rraw * 0xFF) / bmphead.bitmask.rmax;
                    g = (graw * 0xFF) / bmphead.bitmask.gmax;
                    b = (braw * 0xFF) / bmphead.bitmask.bmax;
                }
                break;

                default:
                break;
            }

            WriteRGB( write_pixel, r, g, b );

            write_pixel += sizeof( uint32_t );
            read_pixel += bytes_pp;
        }
        read_row += ydir * pitch;
    }


    // If we made it this far, we succeeded
    returncode = 1;
}
stop:
    if ( file != INVALID_HANDLE_VALUE )
    {
        CloseHandle( file );
    }

    // If we failed, clear the image again.
    if ( !returncode && img->pixels )
    {
        DestroyImage( img );
    }

    if ( pixelbuf )
    {
        VirtualFree( pixelbuf, 0, MEM_RELEASE );
    }

    return returncode;
}

void DestroyImage( Bitmap* img )
{
    VirtualFree( img->pixels, 0, MEM_RELEASE );
    img->pixels = 0;
    img->w = 0;
    img->h = 0;
}

void ClearBitmap( Bitmap* img, ClearColor col )
{
    uint32_t size = img->info.bmiHeader.biSizeImage;
    if ( col == WHITE )
    {
        memset( img->pixels, 255, size );
    }
    else if ( col == BLACK )
    {
        memset( img->pixels, 0, size );
    }
}

void DrawHorizontalLine( Bitmap* img, int xstart, int xend, int y, RGB color )
{
    int x;
    uint32_t* row_loc = (uint32_t*)img->pixels + y * img->w;
    for ( x = xstart; x <= xend; x++ )
    {
        WriteRGB( row_loc + x, color.r, color.g, color.b );
    }
}

void DrawVerticalLine( Bitmap* img, int ystart, int yend, int x, RGB color )
{
    int y;
    uint32_t* col_loc = (uint32_t*)img->pixels + x;
    for ( y = ystart; y <= yend; y++ )
    {
        WriteRGB( col_loc + img->w * y, color.r, color.g, color.b );
    }
}

void DrawRectangle( Bitmap* img, const Rect* rect, RGB color )
{
    DrawHorizontalLine( img, rect->x, rect->x + rect->w, rect->y, color );
    DrawHorizontalLine( img, rect->x, rect->x + rect->w, rect->y + rect->h, color );
    DrawVerticalLine( img, rect->y, rect->y + rect->h, rect->x, color );
    DrawVerticalLine( img, rect->y, rect->y + rect->h, rect->x + rect->w, color);
}

void DrawGradient( Bitmap* img, const Rect* dst, RGB startcol, RGB xcol, RGB ycol )
{
    Rect to;
    if ( dst )
    {
        to = *dst;
    }
    else
    {
        to.x = 0;
        to.y = 0;
        to.w = img->w;
        to.h = img->h;
    }

    #define SHIFT_BITS 16
    uint32_t sr, sg, sb;
    sr = startcol.r << SHIFT_BITS;
    sg = startcol.g << SHIFT_BITS;
    sb = startcol.b << SHIFT_BITS;

    uint32_t fxr, fxg, fxb;
    fxr = xcol.r << SHIFT_BITS;
    fxg = xcol.g << SHIFT_BITS;
    fxb = xcol.b << SHIFT_BITS;

    uint32_t fyr, fyg, fyb;
    fyr = ycol.r << SHIFT_BITS;
    fyg = ycol.g << SHIFT_BITS;
    fyb = ycol.b << SHIFT_BITS;

    uint32_t dxr, dxg, dxb;
    dxr = (fxr - sr) / to.w;
    dxg = (fxg - sg) / to.w;
    dxb = (fxb - sb) / to.w;

    uint32_t dyr, dyg, dyb;
    dyr = (fyr - sr) / to.h;
    dyg = (fyg - sg) / to.h;
    dyb = (fyb - sb) / to.h;

    int x, y;
    uint32_t r, g, b;
    for ( y = 0; y < to.h; y++ )
    {
        r = sr + dyr * y;
        g = sg + dyg * y;
        b = sb + dyb * y;

        uint32_t* pixel = (uint32_t*)img->pixels + img->w * (y + to.y) + to.x;
        for ( x = 0; x < to.w; x++ )
        {
            WriteRGB( pixel, r >> SHIFT_BITS, g >> SHIFT_BITS, b >> SHIFT_BITS );
            r += dxr;
            g += dxg;
            b += dxb;
            pixel++;
        }
    }
}

// Function to draw a gradient pattern on a bitmap.
// x_off and y_off are the offsets for the gradient,
// and by changing them animation can be achieved.
void FillGradientPattern( Bitmap* img, int x_off, int y_off )
{
    int x, y;
    uint32_t* whole_pixel = img->pixels;

    for ( y = 0; y < img->h; y++ )
    {
        for ( x = 0; x < img->w; x++ )
        {
            // This draws a very simple gradient over the entire
            // bitmap.
            WriteRGB( whole_pixel, 0, y + y_off, x + x_off );
            whole_pixel++;
        }
    }
}
