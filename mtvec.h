

    #ifndef __c36__mtvec__
    #define __c36__mtvec__


    #include <stdio.h>
    #include <stdint.h>
    #include <stdlib.h>
	

    typedef struct mtvec_t mtvec_t;
    struct mtvec_t
    {
        void** data;
        uint32_t length;
        uint32_t length_real;
    };

    mtvec_t* mtvec_alloc(void);

	void mtvec_dealloc( void* vector );

	void mtvec_reset( mtvec_t* vector );

	void mtvec_adddata( mtvec_t* vector , void* data );

	void mtvec_adddataatindex( mtvec_t* vector, void* data , size_t index );

	void mtvec_adddatainvector( mtvec_t* mtvec_a , mtvec_t* mtvec_b );

	void mtvec_adduniquedata( mtvec_t* vector , void* data );

	void mtvec_adduniquedataatindex( mtvec_t* vector , void* data , size_t index );


	char mtvec_removedata( mtvec_t* vector , void* data );

	char mtvec_removedataatindex( mtvec_t* vector , uint32_t index );

	void mtvec_removedatainrange( mtvec_t* vector , uint32_t start , uint32_t end );

	void mtvec_removedatainvector( mtvec_t* mtvec_a , mtvec_t* mtvec_b );

    void mtvec_reverse( mtvec_t* vector );

    void* mtvec_head( mtvec_t* vector );

    void* mtvec_tail( mtvec_t* vector );

	uint32_t mtvec_indexofdata( mtvec_t* vector , void* data );
	

    #endif
