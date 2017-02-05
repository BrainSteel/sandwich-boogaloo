
#include "windows.h"

#include "stdio.h"

#include "common.h"
#include "pool.h"

MemoryPool* AllocateMemoryPool( uint64_t size )
{
    MemoryPool* result = VirtualAlloc( NULL, size + sizeof( MemoryPool ), MEM_COMMIT, PAGE_READWRITE );
    if ( !result )
    {
        sprintf( error_buf, "Could not allocate memory pool of size %u.", (uint32_t)size );
        MessageBoxA( 0, error_buf, 0, MB_OK );
        return NULL;
    }

    result->begin = (void*)result + sizeof( MemoryPool );
    result->first_free = result->begin;
    result->size = size;
    return result;
}

int AddSection( MemoryPool* pool, uint64_t size )
{
    uint64_t used_space = pool->first_free - pool->begin;
    if ( size == 0 )
    {
        size = pool->size - used_space;
    }

    if ( pool->sections_used == SIZE_ARRAY( pool->sections )
         || pool->size - used_space < size )
    {
        sprintf( error_buf, "Could not allocate section of size %u.", (uint32_t)size );
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

void ClearSection( MemorySection* section )
{
    section->first_free = section->begin;
}

void ClearSectionIndex( MemoryPool* pool, uint32_t section_index )
{
    ClearSection( &pool->sections[section_index] );
}

void* PushSize( MemorySection* section, uint32_t size )
{
    uint64_t used_space = section->first_free - section->begin;
    if ( section->size - used_space < size )
    {
        sprintf( error_buf, "Could not push size %u.", size );
        MessageBoxA( 0, error_buf, 0, MB_OK );
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
