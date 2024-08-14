#include <fcntl.h>

/*  stat.h, INCLUDE:sys  */
struct   stat
    {
    dev_t           st_dev;         /* device inode resides on */
    ino_t           st_ino;         /* this inode's number */
    unsigned short  st_mode;        /* protection */
    short           st_nlink;       /* number or hard links to the file */
    uid_t           st_uid;         /* user-id of owner */
    gid_t           st_gid;         /* group-id of owner */
    dev_t           st_rdev;        /* the device type, for inode that is device */
    off_t           st_size;        /* total size of file */
    time_t          st_atime;       /* file last access time */
    time_t          st_mtime;       /* file last modify time */
    time_t          st_ctime;       /* file last status change time */
    long            st_blksize;     /* optimal blocksize for file system */
    long            st_blocks;      /* actual number of blocks allocated */
    };

#ifndef __ARGS
#ifdef NARGS
#define __ARGS(a) ()
#else
#define __ARGS(a) a
#endif
#endif
extern stat __ARGS((char *, struct stat *));
