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
    while( ( ch = getopt( argc, argv, "AaCcdFfhiklnqRrSstux1" ) ) != -1 ) {
        switch( ch ) {
            case 'A': flags[0] = true; break;
            case 'a': flags[1] = true; break;
            case 'C': flags[2] = true; flags[21] = flags[10] = flags[11] = flags[20] = false; break;
            case 'c': flags[3] = true; flags[18] = flags[17] = false; break;
            case 'd': flags[4] = true; break;
            case 'F': flags[5] = true; break;
            case 'f': flags[6] = true; break;
            case 'h': flags[7] = true; flags[9] = false; break;
            case 'i': flags[8] = true; break;
            case 'k': flags[9] = true; flags[7] = false; break;
            case 'l': flags[10] = true; flags[21] = flags[2] = flags[11] = flags[20] = false; break;
            case 'n': flags[11] = true; flags[21] = flags[2] = flags[10] = flags[20] = false; break;
            case 'q': flags[12] = true; flags[19] = false; break;
            case 'R': flags[13] = true; break;
            case 'r': flags[14] = true; break;
            case 'S': flags[15] = true; break;
            case 's': flags[16] = true; break;
            case 't': flags[17] = true; flags[3] = flags[18] = false; break;
            case 'u': flags[18] = true; flags[3] = flags[17] = false; break;
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

void printname( struct dirent *dir, struct stat se ) {
    printf( "%s", dir->d_name );
    // -F
    if( flags[5] ) {
        if( dir->d_type & DT_DIR ) putchar( '/' );
        else if( ( se->st_mode & S_IXUSR ) || ( se->mode & S_IXGRP ) || ( se->mode & S_IXOTH ) ) putchar( '*' );
        else if( dir->d_type & DT_LNK ) putchar( '@' );
        else if( dir->d_type & DT_WHT ) putchar( '%' );
        else if( dir->d_type & DT_SOCK ) putchar( '=' );
        else if( dir->d_type & DT_FIFO ) putchar( '|' );
    }
    putchar( ' ' );
    return ;
}

void printele( struct dirent *dir ) {
    char pathe[MAX_PATH_LEN];
    strcpy( pathe, cur_path ); strcat( pathe, "/" ); strcat( patha, dir->d_name );
    struct stat se;
    lstat( pathe, &se );
    // -i
    if( flags[8] ) { printf( "%8u ", se.st_ino ); }
    // -s
    if( flags[16] ) { printf( "%4d ", se.st_blocks ); }
    printname( dir, se );
    return ;
}

void mode_to_letters( int mode, char *str ) {
    strcpy( modes, "----------" );
    if( S_ISDIR( mode ) ) str[0] = 'd';
    if( S_ISCHR( mode ) ) str[0] = 'c';
    if( S_ISBLK( mode ) ) str[0] = 'b';
    if( mode & S_IRUSR ) str[1] = 'r';
    if( mode & S_IWUSR ) str[2] = 'w';
    if( mode & S_IXUSR ) str[3] = 'x';
    if( mode & S_IRGRP ) str[4] = 'r';
    if( mode & S_IWGRP ) str[5] = 'w';
    if( mode & S_IXGRP ) str[6] = 'x';
    if( mode & S_IROTH ) str[7] = 'r';
    if( mode & S_IWOTH ) str[8] = 'w';
    if( mode & S_IXOTH ) str[9] = 'x';
    return ;
}

char * uid_to_name( uid_t uid ) {
    struct passwd *ppw;
    static char numstr[10];
    if( ( ppw = getpwuid( uid ) ) == NULL || flags[11] ) {
        sprintf( numstr, "%d", uid );
        return numbstr;
    }
    return ppw->pw_name;
}

char * gid_to_name( gid_t gid ) {
    struct group *pgrp;
    static char numstr[10];
    if( ( pgrp = getgrgid( gid ) ) == NULL || flags[11] ) {
        sprintf( numstr, "%d", gid );
        return numbstr;
    }
    return pgrp->gr_name;
}

void printsize( long s ) {
    // -h
    if( flag[7] ) printf( "%.1fK ", 1.0 * s / 1024 );
    // -k
    if( flag[9] ) printf( "%.8ld ", s );
    return ;
}

void show_file_info( struct dirent *dir ) {
    struct stat st;
    char pathe[MAX_PATH_LEN], modes[11];
    strcpy( pathe, cur_path ); strcat( pathe, "/" ); strcat( patha, dir->d_name );
    lstat( pathe, &st );
    mode_to_letters( st.st_mode, modes );
    printf( "%s ", modes );
    printf( "%4d ", st.st_nlink );
    printf( "%-8s ", uid_to_name( st.st_uid ) );
    printf( "%-8s ", gid_to_name( st.st_gid ) );
    printsize( st.st_size );
    printf( "%.12s ", 4 + ctime( &st.st_mtime ) );
    printname( dir, st );
    return ;
}

void do_ls_dir( char *name ) {
    DIR *pdir;
    struct dirent *filenames[MAXN];
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
    } else if( flags[10] ) {    // -l -n
        strcpy( cur_path, name );
        for( int i = 0; i < nfile; ++i ) {
            show_file_info( filenames[i] );
        }
    } else {
        // -x
        if( flags[20] ) {
#define flagx
        }
        int maxcol = 0;
        struct winsize wsize;
        strcpy( cur_path, name );
        for( int i = 0; i < nfile; ++i ) {
            int tcol = strlen( filenames[i]->d_name ) + 1;
            // -i
            if( flags[8] ) tcol += 9;
            // -s
            if( flags[16] ) tcol += 5;
            maxcol = max( maxcol, tcol );
        }
        int nrow, ncol;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &wsize);
        ncol = wsize.ws_col / maxcol;
        nrow = nfile / ncol;
#ifdef flagx
        for( int i = 0; i < nrow; ++i ) {
            for( int j = 0; j < ncol; ++j )
                printele( filenames[j * nrow + i] );
#else
        for( int i = 0; i < nrow; ++i ) {
            for( int j = 0; j < ncol; ++i )
                printele( filenames[i * ncol + j] );
#endif
            puts( "" );
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
