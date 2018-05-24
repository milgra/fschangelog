#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/param.h>
#include "mtmap.h"
#include "mtmem.h"
#include "mtcstr.h"

FILE* file = NULL;
char* dirup = "\n<\n";
char* dirdown = "\n>\n";
char* newline = "\n";
mtmap_t* path_to_size;

size_t dircount = 0;
size_t filecount = 0;
size_t skipcount = 0;

char* root = "/";

void scan_directory( char* path )
{
    DIR *dir = opendir( path );
    
    if ( dir != NULL )
    {
        struct dirent *entry;
        
        while ( ( entry = readdir( dir ) ) != NULL )
        {
            char newpath[ MAXPATHLEN + 1 ] = { 0 };
            snprintf( newpath, MAXPATHLEN + 1, "%s/%s", path, entry->d_name );

            if ( entry->d_type == DT_DIR || entry->d_type == DT_UNKNOWN )
            {
                if ( strcmp( entry->d_name, "." ) == 0 || strcmp( entry->d_name, ".." ) == 0 )
                {
                
                }
                else
                {
                    fwrite( entry->d_name , entry->d_namlen , 1 , file );
                    fwrite( dirdown , 3 , 1 , file );
                    
                    dircount++;

                    scan_directory( newpath );
                }
            }
            else if ( entry->d_type == DT_REG )
            {
                // write filename and size
                
                fwrite( entry->d_name , entry->d_namlen , 1 , file );
                fwrite( newline , 1 , 1 , file );
                
                filecount++;

                int filehandle = open( newpath , O_RDONLY );
                
                if ( filehandle > -1 )
                {
                    struct stat fileStat;
                    if ( fstat( filehandle, &fileStat) < 0 )
                    {
                        skipcount++;
                        char sizestr[ 1 ] = { 0 };
                        fwrite( sizestr , 1 , 1 , file );
                        fwrite( newline , 1 , 1 , file );
                    }
                    else
                    {
                        char sizestr[ 16 + 1 ] = { 0 };
                        snprintf( sizestr, 16 + 1, "%llx", fileStat.st_size );

                        fwrite( sizestr , strlen( sizestr ) , 1 , file );
                        fwrite( newline , 1 , 1 , file );
                    }
                    
                    close( filehandle );
                }
                else
                {
                    skipcount++;
                    char sizestr[ 1 ] = { 0 };
                    fwrite( sizestr , 1 , 1 , file );
                    fwrite( newline , 1 , 1 , file );
                }
            }
            else
            {
                skipcount++;
            }
            
            printf("\033[1;0HScanning directories...");
            printf("\033[2;0HSkipped %zu" , skipcount );
            printf("\033[3;0HFiles %zu" , filecount );
            printf("\033[4;0HDirectories %zu\n" , dircount );
        }
        
        fwrite( dirup , 3 , 1 , file );

        closedir(dir);
    }
    else
    {
        skipcount++;
    }
}

void parse_log( char compare )
{
    mtvec_t* updates = mtvec_alloc();
    
    char line[ MAXPATHLEN + 1 ] = { 0 };
    char path[ MAXPATHLEN + 1 ] = { 0 };
    char name[ MAXPATHLEN + 1 ] = { 0 };

    char isname = 1;

    int linepos = 0;
    int pathpos = 0;
    int namepos = 0;
    
    path[ 0 ] = '/';

    while ( 1 )
    {
        char c = getc( file );
        if ( c == EOF ) break;
        
        if ( c == '\n' )
        {
            if ( isname == 1 )
            {
                if ( linepos == 0 )
                {
                    // it will be a dirup
                }
                else
                {
                    memcpy( name , line , linepos );
                    namepos = linepos - 1;
                }
            }
            else
            {
                if ( linepos > 0 )
                {
                    if ( line[0] == '>' )
                    {
                        // dirdown, append name to path
                        memcpy( path + pathpos + 1 , name, namepos + 1 );
                        pathpos += namepos + 1;
                        path[ pathpos + 1 ] = '/';
                        path[ pathpos + 2 ] = '\0';
                        pathpos += 1;
                        
                        if ( compare == 1 )
                        {
                            // compare size and path
                            char* sizestr = mtmap_get( path_to_size , path );
                            if ( sizestr == NULL )
                            {
                                printf( "a: %s\n" , path );
                            }
                            else mtmap_del( path_to_size , path );
                        }
                        else
                        {
                            // store size and path
                            char* sizestr = mtcstr_fromcstring( "0" );
                            mtmap_put( path_to_size , path , sizestr );
                            mtmem_release( sizestr );
                        }
                        
                        dircount++;
                    }
                    else if ( line[0] == '<' )
                    {
                        // dirup
                        while ( pathpos > -1 )
                        {
                            pathpos -= 1;
                            if ( path[ pathpos ] == '/' ) break;
                        }
                        path[ pathpos + 1 ] = '\0';
                    }
                    else
                    {
                        int lastpos = pathpos;
                        // file, append name to path
                        memcpy( path + pathpos + 1 , name, namepos + 1 );
                        pathpos += namepos + 1;
                        path[ pathpos + 1 ] = '\0';

                        // filesize
                        line[ linepos ] = '\0';
                        char* sizestr = mtcstr_fromcstring( line );
                        
                        if ( compare == 1 )
                        {
                            // compare size and path
                            char* oldsizestr = mtmap_get( path_to_size , path );
                            if ( oldsizestr == NULL )
                            {
                                printf( "a: %s\n" , path );
                            }
                            else
                            {
                                // check size
                                if ( strcmp( sizestr , oldsizestr ) != 0 )
                                {
                                    mtvec_adddata( updates , mtcstr_fromcstring( path ) );
                                }
                                mtmap_del( path_to_size , path );
                            }
                        }
                        else
                        {
                            // store size and path
                            mtmap_put( path_to_size , path , sizestr );
                            mtmem_release( sizestr );

                            printf("\033[2;0HFiles %zu" , filecount );
                            printf("\033[3;0HDirectories %zu\n" , dircount );
                        }
                        
                        // reset path
                        pathpos = lastpos;
                        path[ pathpos + 1 ] = '\0';
                        
                        filecount++;
                    }
                }
                else
                {
                    printf( "invalid file\n" );
                }
            }
            
            isname = 1 - isname;
            linepos = 0;
        }
        else
        {
            line[ linepos++ ] = c;
        }
    }
    
    if ( compare == 1 )
    {
        printf("\n");
        // remaining paths are deleted
        mtvec_t* deleted = mtmap_keys( path_to_size );
        for ( int index = 0 ; index < deleted->length ; index++ )
        {
            printf( "d: %s\n" , deleted->data[ index ] );
        }
        printf("\n");
        // remaining paths are deleted
        for ( int index = 0 ; index < updates->length ; index++ )
        {
            printf( "u: %s\n" , updates->data[ index ] );
        }
    }
}

int main(int argc, const char * argv[])
{
    printf("\033[2J");

    if ( argc == 1 )
    {
        char newname[ MAXPATHLEN + 1 ] = { 0 };
        time_t now = time (0);
        strftime (newname, MAXPATHLEN + 1, "snapshot_%Y-%m-%d_%H-%M-%S.txt", localtime(&now) );
        file = fopen( newname , "a" );
        scan_directory( root );
        fclose( file );
    }
    else
    {
        path_to_size = mtmap_alloc();

        printf("\033[1;0HParsing %s...",argv[1]);

        file = fopen( argv[1] , "r" );
        parse_log( 0 );
        fclose( file );
        
        printf("\033[4;0HComparing to %s...\n",argv[2]);
        file = fopen( argv[2] , "r" );
        parse_log( 1 );
        fclose( file );
    }
    
    return 0;
}
