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
            if ( entry->d_type == DT_DIR || entry->d_type == DT_UNKNOWN )
            {
                if ( strcmp( entry->d_name, "." ) == 0 || strcmp( entry->d_name, ".." ) == 0 )
                {
                
                }
                else
                {
                    fwrite( entry->d_name , entry->d_namlen , 1 , file );
                    fwrite( dirdown , 3 , 1 , file );
                    
                    printf("\033[4;0HDirectories %zu" , dircount++ );
                    printf("\033[5;0H%-*.*s" , MAXPATHLEN,MAXPATHLEN, path );

                    char newpath[ MAXPATHLEN + 1 ] = { 0 };
                    snprintf( newpath, MAXPATHLEN + 1, "%s/%s", path, entry->d_name );
                    scan_directory( newpath );
                }
            }
            else
            {
                // write filename and size
                
                fwrite( entry->d_name , entry->d_namlen , 1 , file );
                fwrite( newline , 1 , 1 , file );

                printf("\033[2;0HSkipped %zu" , skipcount );
                printf("\033[3;0HFiles %zu" , filecount++ );

                char newpath[ MAXPATHLEN + 1 ] = { 0 };
                snprintf( newpath, MAXPATHLEN + 1, "%s/%s", path, entry->d_name );

                int filehandle = open( newpath , O_RDONLY );
                
                if ( filehandle > -2 )
                {
                    struct stat fileStat;
                    if ( fstat( filehandle, &fileStat) < 0 )
                    {
                        skipcount++;
                        // printf("\033[10;0HInvalid filestat for %s" , newpath );
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
                    // printf("\033[11;0HInvalid filehandle for %s" , newpath );
                }
            }
        }
        fwrite( dirup , 3 , 1 , file );

        closedir(dir);
    }
    else
    {
        skipcount++;
        // printf("\033[12;0HInvalid directory %s : %s" , path , strerror( errno ) );
    }
    printf("\n");
}

void parse_log( char compare )
{
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
                                printf( "ADDED : %s\n" , path );
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
                        // fule, append name to path
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
                                printf( "ADDED : %s\n" , path );
                            }
                            else
                            {
                                // check size
                                if ( strcmp( sizestr , oldsizestr ) != 0 )
                                {
                                    printf( "UPDATED : %s" , path );
                                }
                                mtmap_del( path_to_size , path );
                            }
                        }
                        else
                        {
                            // store size and path
                            mtmap_put( path_to_size , path , sizestr );
                            mtmem_release( sizestr );
                        }
                        
                        // reset path
                        pathpos = lastpos;
                        path[ pathpos + 1 ] = '\0';
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
        // remaining paths are deleted
        mtvec_t* deleted = mtmap_keys( path_to_size );
        for ( int index = 0 ; index < deleted->length ; index++ )
        {
            printf( "DELETED : %s\n" , deleted->data[ index ] );
        }
    }
}

int main(int argc, const char * argv[])
{
    if ( argc == 1 )
    {
        printf("\033[2J");
        printf("\033[1;0HScanning directories...");
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

        printf("PARSING LOG...\n");
        file = fopen( argv[1] , "r" );
        parse_log( 0 );
        fclose( file );
        
        printf( "path count %zx\n" , path_to_size->count );

        printf("COMPARING LOG...\n");
        file = fopen( argv[2] , "r" );
        parse_log( 1 );
        fclose( file );
    }
    
    return 0;
}
