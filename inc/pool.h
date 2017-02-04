

#ifndef POOL_H
#define POOL_H

#define MEMORY_SECTION_NUM 16

typedef struct MemorySection
{
    void* begin;
    uint64_t size;
    uint64_t first_free;
} MemorySection;

typedef struct MemoryPool
{
    void* begin;
    void* first_free;
    uint64_t size;

    uint32_t sections_used;
    MemorySection sections[MEMORY_SECTION_NUM];
} MemoryPool;


#define PUSH_STRUCT( type, section ) (type*)PushSize( section, sizeof( type ));
#define PUSH_ARRAY( section, type, num ) (type*)PushSize( )

MemoryPool* AllocateMemoryPool( uint64_t size );
int AddSection( MemoryPool* pool, uint64_t size );
void* PushSize( MemorySection* section, uint32_t size );
void DestroyMemoryPool( MemoryPool* pool );

#endif
