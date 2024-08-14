/*
** stat structure used by fstat() and stat()
** will not be compatible with your compiler's stat() and fstat()
*/
#include <sys/types.h>

struct stat {
   u_short   st_mode;	/* file type and protection bits */
   short     st_nlink;	/* number of links */
   uid_t     st_uid;   	/* user id */
   gid_t     st_gid;    /* group id */
   off_t     st_size;	/* size of file in bytes */
   time_t    st_atime;	/* time of last access */
   time_t    st_mtime;  /* last modify */
   long      st_blksize;
};

/* st_mode will have bits set as follows */
/* the least significant 9 bits will be the unix
** rwxrwxrwx bits (octal 777). 
*/

#define S_IFDIR      0x2000  /* directory */
#define S_IFREG      0x1000  /* regular file */
#define S_IFMT       ( S_IFDIR | S_IFREG )
#define S_IREAD     0000400  /* read permission for owner */
#define S_IWRITE    0000200  /* write ...  */
#define S_IEXEC     0000100  /* execute ... */
