#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include "mtmap.h"
#include "mtmem.h"
#include "mtcstr.h"

char* root = "/";
char* newline = "\n";
char* zeroline = "0\n";
char* dirupline = "\n<\n";
char* dirdownline = "\n>\n";

FILE* file = NULL;
mtmap_t* path_to_size;

size_t dircount = 0;
size_t filecount = 0;
size_t skipcount = 0;

void scan_directory( char* path )
{
    DIR* dir = opendir( path );
    
    if ( dir != NULL )
    {
        while ( 1 )
        {
            struct dirent *entry = readdir( dir );
            
            if ( entry != NULL )
            {
                char newpath[ MAXPATHLEN * 2 ] = { 0 };
                snprintf( newpath, MAXPATHLEN * 2, "%s/%s", path, entry->d_name );

                if ( entry->d_type == DT_DIR )
                {
                    if ( strcmp( entry->d_name, "." ) != 0 && strcmp( entry->d_name, ".." ) != 0 )
                    {
                        // write entry name to file with dirdown
                        
                        fwrite( entry->d_name , entry->d_namlen , 1 , file );
                        fwrite( dirdownline , 3 , 1 , file );

                        dircount++;

                        scan_directory( newpath );
                    }
                }
                else if ( entry->d_type == DT_REG || entry->d_type == DT_LNK || entry->d_type == DT_UNKNOWN )
                {
                    if ( strcmp( entry->d_name, "." ) != 0 && strcmp( entry->d_name, ".." ) != 0 )
                    {
                        // write entry name to file

                        fwrite( entry->d_name , entry->d_namlen , 1 , file );
                        fwrite( newline , 1 , 1 , file );
                        
                        filecount++;
                        
                        // try to write down file size if possible, if not then zero az size
                        
                        if ( entry->d_type != DT_LNK )
                        {
                            FILE* filehandle = fopen( newpath , "r" );
                            
                            if ( filehandle != NULL )
                            {
                                struct stat fileStat;
                                int fdesc = fileno( filehandle );
                                
                                if ( fstat( fdesc, &fileStat) < 0 )
                                {
                                    skipcount++;
                                    fwrite( zeroline , 2 , 1 , file );
                                }
                                else
                                {
                                    char sizestr[ 16 + 16 + 1 ] = { 0 };
                                    snprintf( sizestr, 16 + 16 + 1, "%llx%lx", fileStat.st_size , fileStat.st_mtimespec.tv_sec );

                                    fwrite( sizestr , strlen( sizestr ) , 1 , file );
                                    fwrite( newline , 1 , 1 , file );
                                }
                                
                                fclose( filehandle );
                            }
                            else
                            {
                                skipcount++;
                                fwrite( zeroline , 2 , 1 , file );
                            }
                        }
                        else
                        {
                            fwrite( zeroline , 2 , 1 , file );
                        }
                    }
                }
                else
                {
                    // skipcount++;
                }
            }
            else break;
        }
        
        printf("\033[1;0HScanning directories...");
        printf("\033[2;0HSkipped %zu" , skipcount );
        printf("\033[3;0HFiles %zu" , filecount );
        printf("\033[4;0HDirectories %zu\n" , dircount );

        // write empty line as name and dirup if we are coming out of directory

        fwrite( dirupline , 3 , 1 , file );

        closedir( dir );
    }
    else
    {
        skipcount++;
    }
}

void parse_log( char compare )
{
    mtvec_t* updates = mtvec_alloc( );
    
    char line[ MAXPATHLEN * 2 ] = { 0 };
    char path[ MAXPATHLEN * 2 ] = { 0 };
    char name[ MAXPATHLEN * 2 ] = { 0 };

    char isname = 1;

    int linepos = 0;
    int pathpos = 0;
    int namepos = 0;
    
    path[ 0 ] = '/';

    while ( 1 )
    {
        char c = getc( file );
        if ( c != EOF )
        {
            // we are interested in lines
            
            if ( c == '\n' )
            {
                if ( isname == 1 )
                {
                    // store name if we have one, if not it will be a dirup
                    
                    if ( linepos > 0 )
                    {
                        memcpy( name , line , linepos );
                        namepos = linepos - 1;
                    }
                }
                else
                {
                    // checking size info
                    
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
                            // dirup, remove last path component
                            
                            while ( pathpos > -1 )
                            {
                                pathpos -= 1;
                                if ( path[ pathpos ] == '/' ) break;
                            }
                            path[ pathpos + 1 ] = '\0';
                        }
                        else
                        {
                            // size, read it up
                            
                            int lastpos = pathpos;
                            
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
                                    // compare size, add to updates if differs
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
                
                // switch name/size line
                
                isname = 1 - isname;
                linepos = 0;
            }
            else
            {
                line[ linepos++ ] = c;
            }
        }
        else break;
    }
    
    // print out deleted and updated paths
    
    if ( compare == 1 )
    {
        printf("\n");
        mtvec_t* deleted = mtmap_keys( path_to_size );

        for ( int index = 0 ; index < deleted->length ; index++ )
        {
            printf( "d: %s\n" , deleted->data[ index ] );
        }
        printf("\n");

        for ( int index = 0 ; index < updates->length ; index++ )
        {
            printf( "u: %s\n" , updates->data[ index ] );
        }
    }
}

int main(int argc, const char * argv[])
{
    printf("\033[2J");
    
    char showhelp = 0;
    char newname[ MAXPATHLEN * 2 ] = { 0 };
    time_t now = time (0);
    strftime (newname, MAXPATHLEN * 2, "snapshot_%Y-%m-%d_%H-%M-%S.txt", localtime(&now) );

    if ( argc == 2 )
    {
        if ( 0 == strcmp( argv[1] , "-s" ) )
        {
            file = fopen( newname , "a" );
            scan_directory( root );
            fclose( file );
        }
        else showhelp = 1;
    }
    else if ( argc == 4 )
    {
        if ( 0 == strcmp( argv[1] , "-s" ) && 0 == strcmp( argv[2] , "-d" ) )
        {
            root = (char*) argv[3];
            file = fopen( newname , "a" );
            scan_directory( root );
            fclose( file );
        }
        else if ( 0 == strcmp( argv[1] , "-c" ) )
        {
            path_to_size = mtmap_alloc();

            printf("\033[1;0HParsing %s...",argv[2]);

            file = fopen( argv[2] , "r" );
            parse_log( 0 );
            fclose( file );
            
            printf("\033[4;0HComparing to %s...\n",argv[3]);
            
            file = fopen( argv[3] , "r" );
            parse_log( 1 );
            fclose( file );
        }
        else showhelp = 1;
    }
    else showhelp = 1;
    
    if ( showhelp == 1 )
    {
        printf("\033[0;0Hfschangelog v0.5\nCreates and compares ligthweight directory snapshots based on file size and last modification date.\n");
        printf("-s create snapshot of file system ( use it with sudo! )\n");
        printf("-s -d [path] create snapshot of specified directory \n");
        printf("-c [path1] [path2] compare two directory snapshots \n");
    }
    
    return 0;
}
