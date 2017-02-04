
#include "common.h"
#include "pool.h"

MemoryPool* AllocateMemoryPool( uint64_t size )
{
    MemoryPool* result = VirtualAlloc( NULL, size + sizeof( MemoryPool ), MEM_COMMIT, PAGE_READWRITE );
    if ( !result )
    {
        sprintf( error_buf, "Could not allocate memory pool of size %lu.", size );
        MessageBoxA( 0, error_buf, 0, MB_OK );
        return NULL;
    }

    result->begin = (void*)result + sizeof( MemoryPool );
    result->first_free = result->begin;
    result->size = size;
}

int AddSection( MemoryPool* pool, uint64_t size )
{
    if ( pool->sections_used == SIZE_ARRAY( pool->sections )
         || pool->size - (pool->first_free - pool->begin) < size )
    {
        sprintf( error_buf, "Could not allocate section of size %lu.", size );
        MessageBoxA( 0, error_buf, 0, MB_OK );
        return -1;
    }

    pool->sections[pool->sections_used].begin = pool->first_free;
    pool->sections[pool->sections_used].first_free = pool->first_free;
    pool->sections[pool->sections_used].size = size;
    pool->first_free += size;
    pool->sections_used++;
    return pool->sections_used - 1;
}

void* PushSize( MemorySection* section, uint32_t size )
{
    uint64_t used_space = section->first_free - section->begin;
    if ( section->size - used_space < size )
    {
        sprintf( error_buf, "Could not push size %u.", size );
        MessageBoxA( 0, error_buf, 0 MB_OK );
        return NULL;
    }

    void* result = section->first_free;
    section->first_free += size;

    return result;
}

void DestroyMemoryPool( MemoryPool* pool )
{
    VirtualFree( pool, 0, MEM_RELEASE );
}
