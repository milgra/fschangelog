

    #ifndef mtcstr_h
    #define mtcstr_h

    #include <stdio.h>
    #include <stdint.h>
	#include <stdarg.h>


	char*       mtcstr_fromformat( char* format , ... );

	char*		mtcstr_fromcstring( char* string );

	uint32_t    mtcstr_color_from_cstring( char* string );

	char*		mtcstr_generate_readablec( uint32_t length );

	char*		mtcstr_generate_alphanumeric( uint32_t length );


    #endif
