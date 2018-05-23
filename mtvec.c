

    #include <string.h>
    #include "mtmem.h"
    #include "mtvec.h"

	/* creates new vector */

    mtvec_t* mtvec_alloc( )
    {
        mtvec_t* vector = mtmem_calloc( sizeof( mtvec_t ) , mtvec_dealloc );
		vector->data = mtmem_calloc( sizeof( void* ) * 10 , NULL );
		vector->length = 0;
		vector->length_real = 10;
        return vector;
    }

	/* deletes vector */

	void mtvec_dealloc( void* pointer )
	{
        mtvec_t* vector = pointer;
        for ( uint32_t index = 0 ; index < vector->length ; index++ ) mtmem_release( vector->data[index] );
        mtmem_release( vector->data );
	}

	/* resets vector */

	void mtvec_reset( mtvec_t* vector )
	{
        for ( uint32_t index = 0 ; index < vector->length ; index++ ) mtmem_release( vector->data[index] );
		vector->length = 0;
	}

	/* expands storage */

	void mtvec_expand( mtvec_t* vector )
	{
		if ( vector->length == vector->length_real )
		{
			vector->length_real += 10;
			vector->data = mtmem_realloc( vector->data , sizeof( void* ) * vector->length_real );
		}
	}

	/* adds single data */

	void mtvec_adddata( mtvec_t* vector , void* data )
	{
        mtmem_retain(data);
		mtvec_expand( vector );
		vector->data[ vector->length ] = data;
		vector->length += 1;
	}

	/* adds data at given index */

	void mtvec_adddataatindex( mtvec_t* vector, void* data , size_t index )
	{
        mtmem_retain(data);
		mtvec_expand( vector );
		memmove( vector->data + index + 1 , vector->data + index , ( vector->length - index ) * sizeof( void* ) );
		vector->data[ index ] = data;
		vector->length += 1;
	}

	/* adds all items in vector to vector */
	
	void mtvec_adddatainvector( mtvec_t* mtvec_a , mtvec_t* mtvec_b )
	{
        for ( uint32_t index = 0 ; index < mtvec_b->length ; index++ ) mtmem_retain( mtvec_b->data[index] );
		mtvec_a->length_real += mtvec_b->length_real;
		mtvec_a->data = mtmem_realloc( mtvec_a->data , sizeof( void* ) * mtvec_a->length_real );
		memcpy( mtvec_a->data + mtvec_a->length , mtvec_b->data , mtvec_b->length * sizeof( void* ) );
		mtvec_a->length += mtvec_b->length;
	}

	/* adds single unique data */

	void mtvec_adduniquedata( mtvec_t* vector , void* data )
	{
		if ( mtvec_indexofdata( vector , data ) == UINT32_MAX ) mtvec_adddata( vector , data );
	}

	/* adds single unique data at index */

	void mtvec_adduniquedataatindex( mtvec_t* vector , void* data , size_t index )
	{
		if ( mtvec_indexofdata( vector , data ) == UINT32_MAX ) mtvec_adddataatindex( vector , data , index );
	}
    
	/* removes single data, returns 1 if data is released during removal */

	char mtvec_removedata( mtvec_t* vector , void* data )
	{
		char success = 0;
		uint32_t index = mtvec_indexofdata( vector , data );
		if ( index < UINT32_MAX ) success = mtvec_removedataatindex( vector , index );
		return success;
	}

	/* removes single data at index, returns 1 if data is released during removal */

	char mtvec_removedataatindex( mtvec_t* vector , uint32_t index )
	{
        char success = mtmem_release(vector->data[index]);
		memmove( vector->data + index , vector->data + index + 1 , ( vector->length - index - 1 ) * sizeof( void* ) );
		vector->length -= 1;
		return success;
	}

	/* removes data in range */

	void mtvec_removedatainrange( mtvec_t* vector , uint32_t start , uint32_t end )
	{
        for ( uint32_t index = start ; index < end ; index++ ) mtmem_release( vector->data[index]);
		memmove( vector->data + start , vector->data + end + 1 , ( vector->length - end - 1 ) * sizeof( void* ) );
		vector->length -= end - start + 1;
	}

	/* removes data in vector */

	void mtvec_removedatainvector( mtvec_t* mtvec_a , mtvec_t* mtvec_b )
	{
		for ( int index = 0 ; index < mtvec_b->length ; index++ )
		{
			mtvec_removedata( mtvec_a , mtvec_b->data[ index ] );
		}
	}

    /* reverses item order */

    void mtvec_reverse( mtvec_t* vector )
    {
        int length = vector->length;
        for ( int index = length - 1; index > -1 ; index-- )
        {
            mtvec_adddata( vector , vector->data[ index ] );
        }
        mtvec_removedatainrange( vector , 0 , length - 1 );
    }

    /* returns head item of vector */

    void* mtvec_head( mtvec_t* vector )
    {
        if ( vector->length > 0 ) return vector->data[ 0 ];
        else return NULL;
    }

    /* returns tail item of vector */

    void* mtvec_tail( mtvec_t* vector )
    {
        if ( vector->length > 0 ) return vector->data[ vector->length - 1 ];
        else return NULL;
    }

	/* returns index of data or UINT32_MAX if not found */

	uint32_t mtvec_indexofdata( mtvec_t* vector , void* data )
	{
		void** actual = vector->data;
		uint32_t index = 0;
		while ( index < vector->length )
		{
			if ( *actual == data ) return index;
			index++;
			actual += 1;
		}
		return UINT32_MAX;
	}
