/*
** extended, Manx compatible stat.
*/
#include <sys/types.h>

struct stat {
   long   st_mtime;
   long   st_atime;
   long   st_size;
   short   st_mode;
   short   st_blksize;
   uid_t   st_uid, st_gid;
   short   st_nlink;   /* added by Dale, connectathon */
};

#define S_IFMT      0x7000
#define S_IFDIR      0x2000
#define S_IFREG      0x1000
#define S_IREAD      0x4
#define S_IWRITE   0x2
#define S_IEXE      0x1
