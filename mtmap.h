

    #ifndef _STRmtmap_H_
    #define _STRmtmap_H_


	#include <stdio.h>
	#include <stdlib.h>
	#include "mtvec.h"

	typedef struct pair_t pair_t;
	struct pair_t {
		char *key;
		void *value;
	};

	typedef struct bucket_t bucket_t;
	struct bucket_t {
		size_t count;
		pair_t *pairs;
	};

	typedef struct mtmap_t mtmap_t;
	struct mtmap_t {
		size_t count_real;
		size_t count;
		bucket_t *buckets;
	};

	mtmap_t *   mtmap_alloc(void);

	void        mtmap_dealloc( void* pointer );

	void        mtmap_reset( mtmap_t* map );


	int         mtmap_put( mtmap_t *map , const char *key , void *value );

	void*       mtmap_get( mtmap_t *map , const char *key );

    void        mtmap_del( mtmap_t* map , const char* key );


	mtvec_t*    mtmap_keys( mtmap_t *map );
    
	mtvec_t*    mtmap_values( mtmap_t *map );
	
	void        mtmap_printkeys( mtmap_t* map );

	#ifdef DEBUG
	void        mtmap_test(void);
	#endif


    #endif
