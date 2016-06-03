#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <grp.h>
#include <pwd.h>

const int MAX_PATH_LEN = 512;
const int MAXN = 1 << 10;

struct stat statbuf[MAXN];

char dirs_path[MAXN][MAX_PATH_LEN];
bool flags[30];
int oprandw, ndir;

void get_oprands( int argc, char **argv ) {
    memset( flags, false, sizeof( flags ) );
    char ch;
    while( ( ch = getopt( argc, argv, "AaCcdFfhiklnqRrSstuw:x1" ) ) != -1 ) {
        switch( ch ) {
            case 'A': flags[0] = true; break;
            case 'a': flags[1] = true; break;
            case 'C': flags[2] = true; break;
            case 'c': flags[3] = true; break;
            case 'd': flags[4] = true; break;
            case 'F': flags[5] = true; break;
            case 'f': flags[6] = true; break;
            case 'h': flags[7] = true; break;
            case 'i': flags[8] = true; break;
            case 'k': flags[9] = true; break;
            case 'l': flags[10] = true; break;
            case 'n': flags[11] = true; break;
            case 'q': flags[12] = true; break;
            case 'R': flags[13] = true; break;
            case 'r': flags[14] = true; break;
            case 'S': flags[15] = true; break;
            case 's': flags[16] = true; break;
            case 't': flags[17] = true; break;
            case 'u': flags[18] = true; break;
            case 'w': flags[19] = true;
                      if( -1 == sscanf( optarg, "%d", &oprandw ) ) {
                          printf( "ls : option needs a parameters -- w\n" );
                      }
                      break;
            case 'x': flags[20] = true; break;
            case '1': flags[21] = true; break;
            default: printf( "ls : Error parameter\n" );
        }
    }
    return ;
}

void get_dirs_path( int argc, char **argv ) {
    ndir = 0;
    for( int i = 1; i < argc; ++i ) {
        if( argv[i][0] != '-' ) {
            strcpy( dirs_path[ndir], argv[i] );
            ++ndir;
        }
    }
    if( ndir == 0 ) strcpy( dirs_path[ndir++], "." );
    return ;
}

void do_ls_dir( char *name ) {
    DIR *pdir;
    struct dirent *filenames[MAXN];
    struct stat status[MAXN];
    int nfile = 0;
    if( ( pdir = opendir( name ) ) == NULL ) {
        fprintf( stderr, "ls1: cannot open %s, not a directory.\n", name );
        return ;
    }
    while( ( filenames[nfile] = readdir( pdir ) ) != NULL ) {
        if( flags[1] ) { ++nfile; continue; }
        if( flags[0] && !strcmp( filenames[nfile]->d_name, "." ) &&
                        !strcmp( filenames[nfile]->d_name, ".." ) ) {
            ++nfile; continue;
        }
    }
    closedir( pdir );
    ;
    return ;
}

void do_ls_dir_R( char *name ) {
    DIR *pdir;
    struct dirent *filename;
    char ch_path[MAX_PATH_LEN];
    int mf, me;
    mf = me = 0;
    if( ( pdir = opendir( name ) ) == NULL ) {
        fprintf( stderr, "ls1: cannot open %s, not a directory.\n", name );
        return ;
    }
    while( ( filename = readdir( pdir ) ) != NULL ) {
        if( filename->d_type & DT_DIR ) {
            sprintf( ch_path, "%s/%s", name, filename->d_name );
            do_ls_dir( ch_path );
            do_ls_dir_R( ch_path );
        }
    }
    closedir( pdir );
    return ;
}

void do_ls() {
    for( int i = 0; i < ndir; ++i ) {
        if( ndir - 1 ) printf( "%s:\n", dirs_path[i] );
        if( !flags[13] ) do_ls_dir( dirs_path[i] );
        else do_ls_dir_R( dirs_path[i] );
        if( i != ndir - 1 ) puts( "" );
    }
    return ;
}

int main( int argc, char **argv ) {
    get_oprands( argc, argv );
    get_dirs_path( argc, argv );
    do_ls();
    return 0;
}
