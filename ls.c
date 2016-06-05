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

char dirs_path[MAXN][MAX_PATH_LEN];
char cur_path[MAX_PATH_LEN];
bool flags[30];
int oprandw, ndir;

int max( int a, int b ) {
    return a > b ? a : b;
}

void get_oprands( int argc, char **argv ) {
    memset( flags, false, sizeof( flags ) );
    char ch;
    while( ( ch = getopt( argc, argv, "AaCcdFfhiklnqRrSstuw:x1" ) ) != -1 ) {
        switch( ch ) {
            case 'A': flags[0] = true; break;
            case 'a': flags[1] = true; break;
            case 'C': flags[2] = true; flags[21] = flags[10] = flags[11] = flags[20] = false; break;
            case 'c': flags[3] = true; flags[18] = flags[17] = false; break;
            case 'd': flags[4] = true; break;
            case 'F': flags[5] = true; break;
            case 'f': flags[6] = true; break;
            case 'h': flags[7] = true; break;
            case 'i': flags[8] = true; break;
            case 'k': flags[9] = true; break;
            case 'l': flags[10] = true; flags[21] = flags[2] = flags[11] = flags[20] = false; break;
            case 'n': flags[11] = true; flags[21] = flags[2] = flags[10] = flags[20] = false; break;
            case 'q': flags[12] = true; flags[19] = false; break;
            case 'R': flags[13] = true; break;
            case 'r': flags[14] = true; break;
            case 'S': flags[15] = true; break;
            case 's': flags[16] = true; break;
            case 't': flags[17] = true; flags[3] = flags[18] = false; break;
            case 'u': flags[18] = true; flags[3] = flags[17] = false; break;
            case 'w': flags[19] = true; flags[12] = false;
                      if( -1 == sscanf( optarg, "%d", &oprandw ) ) {
                          printf( "ls : option needs a parameters -- w\n" );
                      }
                      break;
            case 'x': flags[20] = true; flags[21] = flags[2] = flags[10] = flags[11] = false; break;
            case '1': flags[21] = true; flags[2] = flags[10] = flags[11] = flags[20] = false; break;
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

int cmp( const void *a, const void *b ) {
    struct stat sa, sb;
    char patha[MAX_PATH_LEN], pathb[MAX_PATH_LEN];
    strcpy( patha, cur_path ); strcat( patha, "/" ); strcat( patha, ( *( struct dirent ** )a )->d_name );
    strcpy( pathb, cur_path ); strcat( pathb, "/" ); strcat( pathb, ( *( struct dirent ** )b )->d_name );
    lstat( patha, &sa ); lstat( pathb, &sb );
    if( flags[17] ) return sa.st_mtime - sb.st_mtime;       // -t
    else if( flags[3] ) return sa.st_ctime - sb.st_ctime;   // -c
    else if( flags[18] ) return sa.st_atime - sb.st_atime;  // -u
    else if( flags )  return sb.st_size - sa.st_size;       // -S
    return strcmp( ( *( struct dirent ** )a )->d_name, ( *( struct dirent ** )b )->d_name );    // default
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
        // -a
        if( flags[1] ) { ++nfile; continue; }
        // -A
        if( flags[0] && !strcmp( filenames[nfile]->d_name, "." ) &&
                        !strcmp( filenames[nfile]->d_name, ".." ) ) {
            ++nfile; continue;
        }
        if( filenames[nfile]->d_name[0] != '.' ) ++nfile;
    }
    closedir( pdir );
    // -f
    if( !flags[6] ) {
//        for( int i = 0; i < nfile; ++i ) {
//            printf( "%s -- ", filenames[i]->d_name );
//        }
        puts( "" );
        strcpy( cur_path, name );
        qsort( filenames, nfile, sizeof( filenames[0] ), cmp );
        // -r
        if( flags[14] ) {
            struct dirent *tmp;
            for( int i = 0; i < nfile / 2; ++i ) {
                tmp = filenames[i];
                filenames[i] = filenames[nfile - i - 1];
                filenames[nfile - i - 1] = tmp;
            }
        }
    }
    if( flags[21] ) {           // -1
        for( int i = 0; i < nfile; ++i ) {
            printf( "%s\n", filenames[i]->d_name );
        }
    } else if( flags[10] ) {    // -l
        ;
    } else {
        // -x
        if( flags[20] ) {
#define flagx
        }
        int maxcol = 0;
        struct winsize wsize;
        for( int i = 0; i < nfile; ++i ) {
            int tcol = strlen( filenames[i]->d_name ) + 1;
            if( flags[8] ) {
                ;
            }
            if( flags[16] ) {
                ;
            }
            maxcol = max( maxcol, tcol );
        }
        int nrow, ncol;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &wsize);
        ncol = wsize.ws_col / maxcol;
        nrow = nfile / ncol;
#ifdef flagx
        for( int i = 0; i < nrow; ++i ) {
            for( int j = 0; j < ncol; ++j ) {
                printele( filenames[j * nrow + i] );
#else
        for( int i = 0; i < nrow; ++i ) {
            for( int j = 0; j < ncol; ++i ) {
                printele( filenames[i * ncol + j] );
#endif
            }
        }
        if( flags[20] ) {
#undef flagx
        }
    }
    return ;
}

// -R
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
    do_ls_dir( name );
    while( ( filename = readdir( pdir ) ) != NULL ) {
        if( filename->d_type & DT_DIR ) {
            if( strcmp( filename->d_name, "." ) == 0 || strcmp( filename->d_name, ".." ) == 0 ) continue;
            sprintf( ch_path, "%s/%s", name, filename->d_name );
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
        else {
            printf( "%s:\n", dirs_path[i] );
            do_ls_dir_R( dirs_path[i] );
        }
        if( i != ndir - 1 ) puts( "" );
    }
    return ;
}

int cmpstr( const void *p, const void *q ) {
    return strcmp( ( const char * )p, ( const char * )q );
}

int main( int argc, char **argv ) {
    get_oprands( argc, argv );
    get_dirs_path( argc, argv );
    qsort( dirs_path, ndir, sizeof( char ) * MAX_PATH_LEN, cmpstr );
    do_ls();
    return 0;
}
