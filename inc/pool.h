

#ifndef POOL_H
#define POOL_H

#include "common.h"

#define MEMORY_SECTION_NUM 16

typedef struct MemorySection
{
    void* begin;
    void* first_free;
    uint64_t size;
} MemorySection;

typedef struct MemoryPool
{
    void* begin;
    void* first_free;
    uint64_t size;

    uint32_t sections_used;
    MemorySection sections[MEMORY_SECTION_NUM];
} MemoryPool;


#define PUSH_STRUCT( type, sectionptr ) (type*)PushSize( sectionptr, sizeof( type ))
#define PUSH_ARRAY( sectionptr, type, num ) (type*)PushSize( sectionptr, sizeof( type ) * num )

#define PUSH_STRUCT_INDEX( type, poolptr, section_index ) PUSH_STRUCT( type, &poolptr->sections[section_index] )
#define PUSH_ARRAY_INDEX( type, poolptr, section_index, num ) PUSH_ARRAY( type, &poolptr->sections[section_index], num )

MemoryPool* AllocateMemoryPool( uint64_t size );
int AddSection( MemoryPool* pool, uint64_t size );
void ClearSection( MemorySection* section );
void ClearSectionIndex( MemoryPool* pool, uint32_t section_index );
void* PushSize( MemorySection* section, uint32_t size );
void DestroyMemoryPool( MemoryPool* pool );

#endif
