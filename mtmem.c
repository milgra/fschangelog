

    #include <string.h>
    #include "mtmem.h"

	static void* mtmem_tracked = NULL;
	
	/* allocs memory with stored destructor and retaincount at the beginning of the section */

    void* mtmem_alloc( size_t size , void ( *destructor )( void* ) )
    {
		if ( size == 0 ) return NULL;
        unsigned char* bytes = malloc( sizeof( struct mtmem_head ) + size );
        assert( bytes != NULL );
        
        struct mtmem_head* head = ( struct mtmem_head* ) bytes;
        
        head->destructor = destructor;
        head->retaincount = 1;

        return bytes + sizeof( struct mtmem_head );
    }

	/* callocs memory with stored destructor and retaincount at the beginning of the section */

    void* mtmem_calloc( size_t size , void (*destructor)(void*) )
    {
		if ( size == 0 ) return NULL;
        unsigned char* bytes = calloc( 1 , sizeof( struct mtmem_head ) + size );
        assert( bytes != NULL );

        struct mtmem_head* head = ( struct mtmem_head* ) bytes;
        
        head->destructor = destructor;
        head->retaincount = 1;

        return bytes + sizeof( struct mtmem_head );
    }

	/* reallocs memory with stored destructor and retaincount at the beginning of the section */

    void* mtmem_realloc( void* pointer , size_t size )
    {
		if ( size == 0 ) return NULL;
        assert(pointer!=NULL);
        
        unsigned char* bytes = (unsigned char*)pointer;
        bytes -= sizeof( struct mtmem_head );
        bytes = realloc( bytes , sizeof( struct mtmem_head ) + size );
        assert( bytes != NULL );

        return bytes + sizeof( struct mtmem_head );
    }
	
	/* increases retaincount of section */

    void* mtmem_retain( void* pointer )
    {
        if ( pointer == NULL ) return NULL;
        
        unsigned char* bytes = (unsigned char*)pointer;
        bytes -= sizeof( struct mtmem_head );
        struct mtmem_head* head = ( struct mtmem_head* ) bytes;

        assert( head->retaincount < UINT16_MAX - 1 );

        head->retaincount += 1;
		
		if ( mtmem_tracked != NULL && mtmem_tracked == pointer ) 
		{
			printf( "mem retain, retaincount %zx\n" , head->retaincount );
		}
		
		return pointer;
    }

	/* decreases retaincount of section, calls destructor if exists, frees up section */

    char mtmem_release( void* pointer )
    {
		if ( pointer == NULL ) return 0;
        
        unsigned char* bytes = ( unsigned char* ) pointer;
        bytes -= sizeof( struct mtmem_head );
        struct mtmem_head* head = ( struct mtmem_head* ) bytes;

        assert( head->retaincount > 0);
        
        head->retaincount -= 1;

		if ( mtmem_tracked != NULL && mtmem_tracked == pointer ) 
		{
			printf( "mem release, retaincount %zx\n" , head->retaincount );
		}

        if ( head->retaincount == 0 )
        {
            if ( head->destructor != NULL ) head->destructor( bytes + sizeof( struct mtmem_head ) );
            free( bytes );
			return 1;
        }
		
		return 0;
    }

	/* release multiple instances */

    char mtmem_releaseeach( void* first , ... )
    {
        va_list ap;
        void* actual;
		char released = 1;
        va_start(ap, first);
        for ( actual = first; actual != NULL; actual = va_arg( ap, void* ) )
        {
            released &= mtmem_release( actual );
        }
        va_end(ap);
		return released;
    }

	/* returns retain count */

    size_t mtmem_retaincount( void* pointer )
    {
        assert(pointer!=NULL);
        
        unsigned char* bytes = (unsigned char*)pointer;
        bytes -= sizeof( struct mtmem_head );
        struct mtmem_head* head = ( struct mtmem_head* ) bytes;
		
		return head->retaincount;
    }
	
	/* tracks address */
		
	void mtmem_track( void* pointer , uint8_t force )
	{
		if ( mtmem_tracked == NULL || force == 1 )
		{
			printf( "mem track address %zx\n" , (size_t)pointer );
			mtmem_tracked = pointer;
		}
	}
