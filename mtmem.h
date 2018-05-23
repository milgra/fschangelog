

    #ifndef __c36__mtmem__
    #define __c36__mtmem__


    #include <stdio.h>
    #include <stdlib.h>
    #include <assert.h>
    #include <stdint.h>
	#include <stdarg.h>

    struct mtmem_head
    {
        void (*destructor)(void*);
        size_t retaincount;
    };

    void*   mtmem_alloc( size_t size , void (*destructor)(void*) );

    void*   mtmem_calloc( size_t size , void (*destructor)(void*) );

    void*   mtmem_realloc( void* pointer , size_t size );

    void*   mtmem_retain( void* pointer );

    char    mtmem_release( void* pointer );

    char    mtmem_releaseeach( void* first , ... );

	void    mtmem_track( void* pointer , uint8_t force );

    size_t  mtmem_retaincount( void* pointer );


    #endif
