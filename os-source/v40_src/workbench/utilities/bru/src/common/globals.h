/************************************************************************
 *									*
 *	Copyright (c) 1988 Enhanced Software Technologies, Inc.		*
 *			     All Rights Reserved			*
 *									*
 *	This software and/or documentation is protected by U.S.		*
 *	Copyright Law (Title 17 United States Code).  Unauthorized	*
 *	reproduction and/or sales may result in imprisonment of up	*
 *	to 1 year and fines of up to $10,000 (17 USC 506).		*
 *	Copyright infringers may also be subject to civil liability.	*
 *									*
 ************************************************************************
 */


/*
 *  FILE
 *
 *	globals.h    global declarations and prototypes
 *
 *  SCCS
 *
 *	@(#)globals.h	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Contains declarations for all external variables and functions
 *	used in BRU, along with suitable prototypes.
 *
 */

#if !PRECOMPILED_HEADERS

/*
 *	Include the autoconfiguration header built automatically by
 *	the autoconfiguration process.
 */

#include "autoconfig.h"

#include <stdio.h>

#include "typedefs.h"		/* Locally defined types */
#include "manifest.h"		/* Manifest constants */
#include "macros.h"		/* Useful macros */

#if amigados
#  include <exec/types.h>
#  include <exec/nodes.h>
#  include <exec/lists.h>
#  include <exec/interrupts.h>
#  include <exec/ports.h>
#  include <exec/libraries.h>
#  include <exec/io.h>
#  include <exec/memory.h>
#  include <exec/exec.h>
#  include <exec/tasks.h>
#  include <exec/execbase.h>
#  include <libraries/dos.h>
#  include <libraries/dosextens.h>
#  include <exec/memory.h>
#  include <intuition/intuition.h>
#  include <devices/trackdisk.h>
#  include <devices/timer.h>
#  include <devices/scsidisk.h>
#  include <clib/dos_protos.h>
#  include <clib/exec_protos.h>
#  include <clib/intuition_protos.h>
#endif

#include <sys/types.h>

#include <errno.h>		/* May need to be <sys/errno.h> for BSD 4.2 */
#ifndef ENAMETOOLONG
#  define ENAMETOOLONG 0	/* bogus errno */
#endif

#if HAVE_STDARG
#  include <stdarg.h>		/* Use the ANSI C variable arg support */
#  define VA_ALIST ...		/* Change VA_ALIST to ANSI C equivalent */
#  define VA_DCL		/* Make this go away, not needed in ANSI */
#  define VA_START va_start	/* Use the two argument ANSI version */
#  define VA_ARG(t,v) t v	/* Generate new style formal */
#  define VA_OARGS(a)		/* Make old style declarations go away */
#else
#  define VA_ALIST va_alist	/* Use the older placeholder */
#  define VA_DCL va_dcl		/* And we do need this here */
#  define VA_START(a,l) va_start(a)
#  define VA_ARG(t,v) v		/* Generate old style formal */
#  define VA_OARGS(a) a		/* Keep old style declarations */
#if HAVE_VARARGS
#  include <varargs.h>		/* Use older "classical UNIX" variable args */
#else
#  include "vargs.h"		/* Use our "fake" varargs */
#endif
#endif

#if RMT				/* Want remote mag tape support via /etc/rmt */
#  if SYSRMTLIB			/* Use the installed system version */
#    include <local/rmt.h>	/* Remote mag tape functions, if available */
#  else				/* Else use our local internal copy */
#    include "rmt.h"		/* Local copy's include file */
#  endif
#endif

#include <sys/stat.h>

#if unix || xenix
#  include <sys/param.h>
#  include <grp.h>
#  include <pwd.h>
#  include <signal.h>
#  if (!pyr && HAVE_UNIONWAIT)
#    include <sys/file.h>	/* 4.2 <fcntl.h> is woefully incomplete */
#  else
#    include <fcntl.h>
#  endif
#  if HAVE_UNIONWAIT
#    include <sys/wait.h>		/* Used by s_wait() */
#  endif
#  if !HAVE_TZSET && HAVE_GETTIMEOFDAY
#    include <sys/time.h>		/* Used by s_timezone() */
#  else
#    include <time.h>			/* defines struct tm */
#  endif
#  if HAVE_SYMLINKS
#      include <errno.h>
#  endif
#  if BSD4_3 || sun
#    include <sys/resource.h>		/* Def of Clock() for 4.3 bsd */
#  endif
#  if HAVE_TERMIO
#    include <termio.h>		/* For terminal handling */
#  else
#    include <sgtty.h>		/* For terminal handling */
#  endif
#  if HAVE_SHM
#    include <sys/types.h>
#    include <sys/ipc.h>
#    include <sys/shm.h>
#    include <sys/msg.h>
#    include <setjmp.h>
#  endif
#  ifdef sgi
#    include <sys/fsid.h>
#    include <sys/fstyp.h>
#    include <sys/statfs.h>
#  endif
#  if HAVE_STATVFS
#    include <sys/statvfs.h>
#  endif
#  if AUX
#    include <sys/ssioctl.h>	/* Contains special ioctl's for Apple A/UX */
#    include <sys/diskformat.h>
#  endif
#  if BSD || sgi
#    define RMTIOCTL
#  endif
#  if AUX || defined(RMTIOCTL)
#    include <sys/ioctl.h>
#  endif
#  ifdef RMTIOCTL
#    include <sys/mtio.h>
#  endif
#else
#  include "sys.h"		/* Header file that fakes it for non-unix */
#  include <time.h>
#  include <fcntl.h>
#  include <signal.h>
#  if amigados
#    include <dos.h>
#  endif
#endif
  
#if ((unix || xenix) && HAVE_SEEKDIR)
#  if HAVE_DIRENT
#    include <dirent.h>
#    define DIRENT struct dirent
#  else
#    include <sys/dir.h>
#    define DIRENT struct direct
#  endif
#else
#if amigados
#  include <sys/dir.h>
#else
#  include "dir.h"
#endif
#define DIRENT struct direct
#endif
  
#include <ctype.h>

/*
 *	For an ANSI C environment include the standard UNIX, string, and
 *	other header files.
 */

#ifdef __STDC__
#  if (unix || xenix)
#    include <unistd.h>
#  endif
#  include <string.h>
#  include <stdlib.h>
#endif

/*
 *	Local BRU include files.
 */

#include "config.h"		/* Configuration information */
#include "blocks.h"		/* Archive format structures */
#include "bruinfo.h"		/* Current invocation information */
#include "dbug.h"		/* Macro based C debugging package */
#include "devices.h"		/* Device table structure */
#include "exeinfo.h"		/* Execution information */
#include "finfo.h"		/* File information structure */
#include "flags.h"		/* Command line arguments */
#include "trees.h"		/* File tree stuff */
#include "usermsg.h"		/* Known internal error codes */
#include "utsname.h"


/*
 *	The structure declaration is used to match that in the
 *	lint library /usr/lib/llib-lc, to shut up lint.
 *	The Unix System V User's Manual has the type of the
 *	second argument as a pointer to a structure "utimbuf",
 *	which is not declared anywhere in the header files.
 *
 */


struct utimbuf {
#if HAVE_STRANGE_UTIMBUF
    time_t atime;
    time_t ausec;
    time_t mtime;
    time_t musec;
#else
    time_t atime;
    time_t mtime;
#endif
};

extern BOOLEAN allnulls PROTO((char *data, int nbytes));
extern BOOLEAN ar_ispipe PROTO((void));
extern BOOLEAN chksum_ok PROTO((union blk *blkp));
extern BOOLEAN compressfip PROTO((struct finfo *fip));
extern BOOLEAN confirmed PROTO((char *action, struct finfo *fip));
extern BOOLEAN copy PROTO((char *out, char *in, int outsize));
extern BOOLEAN decompfip PROTO((struct finfo *fip));
extern BOOLEAN dir_access PROTO((char *name, int amode, BOOLEAN flag));
extern BOOLEAN ejectable PROTO((void));
extern BOOLEAN eoablk PROTO((union blk *blkp));
extern BOOLEAN execute PROTO((char *dir, char *file, char *vector []));
extern BOOLEAN file_access PROTO((char *name, int amode, BOOLEAN flag));
extern BOOLEAN fipstat PROTO((struct finfo *fip, int report));
extern BOOLEAN forcebuffer PROTO((void));
extern BOOLEAN hcheck PROTO((union blk *blkp));
extern BOOLEAN known_end PROTO((int iobytes, int ioerr));
extern BOOLEAN magic_ok PROTO((union blk *blkp));
extern BOOLEAN mklink PROTO((char *exists, char *new));
extern BOOLEAN mksymlink PROTO((char *exists, char *new));
extern BOOLEAN nameconfirm PROTO((char *devname));
extern BOOLEAN namesane PROTO((char *devname));
extern BOOLEAN namesfit PROTO((struct finfo *fip));
extern BOOLEAN need_swap PROTO((union blk *blkp));
extern BOOLEAN needshmcopy PROTO((void));
extern BOOLEAN new_arfile PROTO((int vol));
extern BOOLEAN newdir PROTO((struct finfo *fip));
extern BOOLEAN nextname PROTO((char *bufp, unsigned int bufsize));
extern BOOLEAN openzfile PROTO((struct finfo *fip));
extern BOOLEAN out_of_date PROTO((struct finfo *fip));
extern BOOLEAN possible_end PROTO((int iobytes, int ioerr, BOOLEAN read));
extern BOOLEAN raw_tape PROTO((void));
extern BOOLEAN seekable PROTO((char *fname, int increment));
extern BOOLEAN selected PROTO((struct finfo *fip));
extern BOOLEAN unconditional PROTO((struct finfo *fip));
extern BOOLEAN unformatted PROTO((int iobytes, int ioerr, BOOLEAN read));
extern BOOLEAN wild PROTO((char *string, char *pattern));
extern BOOLEAN write_protect PROTO((int iobytes, int ioerr, BOOLEAN read));

extern CHKSUM chksum PROTO((union blk *blkp));

extern FILE *s_fopen PROTO((char *file_name, char *type));

extern LBA ar_tell PROTO((void));

extern S32BIT fromhex PROTO((char *cpin, int insize));
extern S32BIT getsize PROTO((char *cp));
extern S32BIT s_lseek PROTO((int fildes, S32BIT offset, int whence));
extern S32BIT s_strtol PROTO((char *str, char **ptr, int base));

extern SIGTYPE s_signal PROTO((int sig, SIGTYPE func));

extern VOID *get_memory PROTO((UINT size, BOOLEAN quit));
extern VOID addz PROTO((char *fname));
extern VOID af_close PROTO((void));
extern VOID ar_close PROTO((void));
extern VOID ar_estimate PROTO((LBA size));
extern VOID ar_open PROTO((void));
extern VOID ar_read PROTO((struct finfo *fip));
extern VOID ar_write PROTO((struct finfo *fip, int magic));
extern VOID clear_tree PROTO((void));
extern VOID copyname PROTO((char *out, char *in));
extern VOID create PROTO((void));
extern VOID deallocate PROTO((void));
extern VOID diff PROTO((void));
extern VOID discard_zfile PROTO((struct finfo *fip));
extern VOID do_swap PROTO((void));
extern VOID done PROTO((void));
extern VOID estimate PROTO((void));
extern VOID extract PROTO((void));
extern VOID file_chown PROTO((char *path, int owner, int group));
extern VOID filemode PROTO((char buf [], MODE mode));
extern VOID filter PROTO((VOID (*funcp )()));
extern VOID finfo_init PROTO((struct finfo *fip, char *fn, struct bstat *bsp));
extern VOID fork_shell PROTO((void));
extern VOID free_list PROTO((void));
extern VOID gp_init PROTO((void));
extern VOID hstat PROTO((union blk *blkp, struct finfo *fip));
extern VOID info_only PROTO((void));
extern VOID init PROTO((int argc, char *argv []));
extern VOID inspect PROTO((void));
extern VOID nodes_ignored PROTO((VOID (*funcp )()));
extern VOID patch_buffer PROTO((union blk *blkp, UINT count));
extern VOID phys_seek PROTO((LBA npba));
extern VOID read_info PROTO((void));
extern VOID readsizes PROTO((union blk *blkp));
extern VOID reload PROTO((char *reason));
extern VOID s_endgrent PROTO((void));
extern VOID s_endpwent PROTO((void));
extern VOID s_exit PROTO((int status));
extern VOID s_fflush PROTO((FILE *stream));
extern VOID s_free PROTO((char *ptr));
extern VOID s_sleep PROTO((UINT seconds));
extern VOID sanity PROTO((void));
extern VOID savedev PROTO((char *name));
extern VOID scan PROTO((VOID (*funcp )()));
extern VOID shmcheck PROTO((void));
extern VOID sig_catch PROTO((void));
extern VOID sig_done PROTO((void));
extern VOID sig_pipe PROTO((BOOLEAN ignore));
extern VOID sig_pop PROTO((SIGTYPE *prevINTp, SIGTYPE *prevQUITp));
extern VOID sig_push PROTO((SIGTYPE *prevINTp, SIGTYPE *prevQUITp));
extern VOID stripz PROTO((char *fname));
extern VOID swapbytes PROTO((union blk *blkp));
extern VOID swapshorts PROTO((union blk *blkp));
extern VOID switch_media PROTO((int nvolume));
extern VOID table PROTO((void));
extern VOID tohex PROTO((char *out, S32BIT val, int outsize));
extern VOID tree_add PROTO((char *name));
extern VOID tree_walk PROTO((VOID (*funcp )()));
extern VOID tty_flush PROTO((void));
extern VOID tty_newmedia PROTO((int volume, char *dfault, char *newname, int newnamesize));
extern VOID unresolved PROTO((void));
extern VOID ur_init PROTO((void));
extern VOID usage PROTO((int type));
extern VOID verbosity PROTO((struct finfo *fip));
extern VOID vflush PROTO((void));

extern char *add_link PROTO((struct finfo *fip));
extern char *firstdev PROTO((void));
extern char *gp_gname PROTO((UINT gid));
extern char *namelink PROTO((char *name));
extern char *nextdev PROTO((void));
extern char *s_asctime PROTO((struct tm *tm));
extern char *s_ctime PROTO((long *clock));
extern char *s_fgets PROTO((char *s, int n, FILE *stream));
extern char *s_getenv PROTO((char *name));
extern char *s_malloc PROTO((UINT size));
extern char *s_memcpy PROTO((char *s1, char *s2, int n));
extern char *s_memset PROTO((char *s, int c, int n));
extern char *s_strcat PROTO((char *s1, char *s2));
extern char *s_strchr PROTO((char *s, int c));
extern char *s_strcpy PROTO((char *s1, char *s2));
extern char *s_strdup PROTO((char *string));
extern char *s_strncpy PROTO((char *s1, char *s2, int n));
extern char *s_strrchr PROTO((char *s, int c));
extern char *univlink PROTO((char *name));
extern char *ur_gname PROTO((UINT uid));
extern char response PROTO((char *prompt, int dfault));

extern int ar_vol PROTO((void));
extern int compress PROTO((struct finfo *fip));
extern int decompress PROTO((struct finfo *fip));
extern int isrmt PROTO((int fd));
extern int last_vol PROTO((void));
extern int main PROTO((int argc, char **argv));
extern int path_type PROTO((char *name));
extern int pcreat PROTO((char *path, int fmode));
extern int s_access PROTO((char *path, int amode));
extern int s_atoi PROTO((char *str));
extern int s_ccreat PROTO((char *path, int mode, long size));
extern int s_chmod PROTO((char *path, int mode));
extern int s_chown PROTO((char *path, int owner, int group));
extern int s_close PROTO((int fildes));
extern int s_creat PROTO((char *path, int mode));
extern int s_dup PROTO((int fildes));
extern int s_eject PROTO((int fildes));
extern int s_execv PROTO((char *path, char *argv []));
extern int s_fclose PROTO((FILE *stream));
extern int s_fileno PROTO((FILE *stream));
extern int s_fork PROTO((void));
extern int s_format PROTO((int fildes));
extern int s_getgid PROTO((void));
extern int s_getopt PROTO((int argc, char **argv, char *optstring));
extern int s_getuid PROTO((void));
extern int s_ioctl PROTO((int fildes, int request, int arg));
extern int s_isdigit PROTO((int c));
extern int s_kill PROTO((int pid, int sig));
extern int s_link PROTO((char *path1, char *path2));
extern int s_mkdir PROTO((char *path, int mode));
extern int s_mknod PROTO((char *path, int mode, int dev));
extern int s_open PROTO((char *path, int oflag, int mode));
extern int s_read PROTO((int fildes, char *buf, UINT nbyte));
extern int s_readlink PROTO((char *name, char *buf, int size));
extern int s_setgid PROTO((int gid));
extern int s_setuid PROTO((int uid));
extern int s_strcmp PROTO((char *s1, char *s2));
extern int s_strlen PROTO((char *s));
extern int s_symlink PROTO((char *name1, char *name2));
extern int s_tolower PROTO((int c));
extern int s_umask PROTO((int cmask));
extern int s_uname PROTO((struct utsname *name));
extern int s_unlink PROTO((char *path));
extern int s_utime PROTO((char *path, struct utimbuf *times));
extern int s_vfprintf PROTO((FILE *stream, char *format, va_list ap));
extern int s_vsprintf PROTO((char *s, char *format, va_list ap));
extern int s_write PROTO((int fildes, char *buf, UINT nbyte));
extern int uname PROTO((struct utsname *name));
extern int ur_guid PROTO((char *name));

extern long s_time PROTO((long *tloc));
extern long s_timezone PROTO((void));
extern long s_ulimit PROTO((int cmd, long newlimit));
extern long s_wait PROTO((int *stat_loc));

extern struct device *get_ardp PROTO((char *devname));
extern struct group *s_getgrent PROTO((void));
extern struct passwd *s_getpwent PROTO((void));
extern struct tm *s_localtime PROTO((long *clock));
extern struct bstat *bstat PROTO((char *fname, struct bstat *bstatp, int flg));

extern time_t date PROTO((char *cp));

extern union blk *ar_next PROTO((void));
extern union blk *ar_seek PROTO((LBA offset, int whence));

#ifdef DUMPBLK
extern int do_dump PROTO((union blk *blkp));
extern VOID dump_blk PROTO((union blk *blkp, LBA lba));
#endif

/*
 *	Functions that are just stubs in the Unix/Xenix version.
 */

#if (unix || xenix)
extern VOID clear_archived PROTO((struct finfo *fip));
extern VOID mark_archived PROTO((struct finfo *fip));
extern VOID stackcheck PROTO((void));
extern int abit_set PROTO((struct finfo *fip));
extern int ipc_error PROTO((char *errmsg));
extern int ipc_getc PROTO((void));
extern int ipc_getfilename PROTO((char *namebuf));
extern int ipc_init PROTO((void));
extern int ipc_query PROTO((char *message));
extern int setinfo PROTO((struct finfo *fip));
#endif

#ifndef __STDC__
extern FILE *fopen PROTO((char *fname, char *type));
extern int stat PROTO((char *path, struct stat *buf));
extern int umask PROTO((int cmask));
extern int chmod PROTO((char *path, int mode));
extern int mknod PROTO((char *path, int mode, int dev));
extern int mkdir PROTO((char *path, int mode));
#endif

extern VOID longjmp ();
extern char *asctime ();
extern char *ctime ();
extern char *fgets ();
extern char *gets ();
extern char *mktemp PROTO((char *template));
extern int strcmp ();
extern unsigned alarm ();

/*
 *	Functions used only if the host has shared memory support
 *	for double buffering.
 */

#if HAVE_SHM
extern VOID dbl_done PROTO((void));
extern VOID dbl_errp PROTO((int (*inerror)(), int (*outerror)()));
extern VOID dbl_iop PROTO((int (*readf)(), int (*writef)()));
extern VOID dbl_parms PROTO((UINT shmseg, long shmmax, long shmall, long tapeblk));
extern VOID dbl_setvol PROTO((int volume));
extern int dbl_flush PROTO((void));
extern int dbl_getvol PROTO((void));
extern int dbl_read PROTO((char *buf, UINT size));
extern int dbl_rpcdown PROTO((VOID (*func)(), long arg));
extern int dbl_setup PROTO((int dodbl, int direction));
extern int dbl_write PROTO((char *buf, UINT size));
extern int maxshmsize PROTO((void));
#endif

#if FAST_CHKSUM
extern CHKSUM sumblock ();	/* Use your own private assembly version */
#endif

#if xenix			/* Not in xenix's <time.h> */
extern struct tm *localtime PROTO((long *clock));
#endif

#if HAVE_SYMLINKS
extern int s_readlink ();	/* Invoke library read of symbolic link */
extern int s_symlink ();	/* Invoke library make symbolic link func */
#endif


#if amigados
#define TREE_ADD amiga_tree_add
extern VOID amiga_tree_add ();	/* Add pathname to tree, with expansions */
#else
#define TREE_ADD tree_add
extern VOID tree_add ();	/* Add pathname to tree */
#endif

#if BSD4_2
extern char *sprintf ();			/* See printf(3S) */
#endif

#if unix || xenix
#if !BSD4_2
extern UINT sleep ();			/* See sleep(3C) */
#endif
#endif

#if BSD4_2 && lint
extern time_t time ();			/* See time(2) */
#else
extern long time ();			/* See time(2) */
#endif

#if HAVE_ULIMIT
extern long ulimit PROTO((int cmd, long newlimit));
#endif

#ifndef __STDC__

#if (unix || xenix)
extern struct passwd *getpwent ();		/* See getpwent(3C) */
extern struct group *getgrent ();		/* See getgrent(3C) */
extern VOID endgrent ();		/* See getgrent(3C) */
#if BSD4_2
extern VOID endpwent PROTO((VOID));		/* See getpwent(3C) */
#else
extern int endpwent PROTO((VOID));
#endif
#endif

#if (unix || xenix)
extern SIGTYPE signal PROTO((int sig, SIGTYPE func));
#endif

#if HAVE_STRCHR
extern char *strchr ();
#else
#if HAVE_INDEX
extern char *index ();
#endif
#endif

#if HAVE_STRRCHR
extern char *strrchr ();
#else
#if HAVE_RINDEX
extern char *rindex ();
#endif
#endif

#if HAVE_MEMCPY
extern char *memcpy ();			/* See memory(3C) */
#endif

#if HAVE_MEMSET
extern char *memset ();			/* See memory(3C) */
#endif

#endif	/* !__STDC__ */

#if xenix
extern void tzset ();			/* VOID may not be void */
#else
extern VOID tzset ();
#endif

#if pyr
extern int s_csymlink PROTO((char *name1, char *name2));
extern int csymlink PROTO((char *name1, char *name2));
#endif

/*
 *	The standard System V <sys/shm.h> does not declare shmat() at
 *	all, so we need to declare it explicitly.  However, some xenix
 *	<sys/shm.h> files for the 286 declare it as type "far char *"
 *	while xenix for the 386 declares it as type "char *".  So we
 *	attempt to be compatible with their declaration.  Just to
 *	complicate matters, there are probably 286 xenix systems for
 *	which the below declaration won't compile.  Then, throw SVR4 in,
 *	which does declare it.  Grrrr!!!!
 */

#if (xenix && !M_I386)			/* xenix and !i386 (should be i286?) */
extern far char *shmat ();
#else
#if defined(__STDC__) && !defined(_STYPES)	/* prototypes and SVR4 */
extern void *shmat (int, void *, int);
#else
extern char *shmat ();
#endif
#endif

/*
 *	System calls
 */

extern int close PROTO((int fildes));
extern int dup PROTO((int fildes));
extern int readlink ();
extern int symlink ();
extern int utime PROTO ((char *path, struct utimbuf *times));
extern long lseek PROTO((int fildes, long offset, int whence));

/*
 *	External variables.
 */

extern BOOLEAN interrupt;	/* Interrupt received */
extern FILE *errfp;		/* Stream error messages written to */
extern FILE *logfp;			/* Stream verbosity msgs written to */
extern UINT bufsize;		/* Archive read/write buffer size */
extern UINT nzbits;		/* Number of LZW bits to use via -N option */
extern UINT uid;		/* User ID derived via -o option */
extern ULONG msize;		/* Size of archive media */
extern char *id;		/* Bru id */
extern char *label;		/* Archive label string given by user */
extern char *optarg;		/* Next optional argument */
extern char *sys_errlist[];		/* Array of error messages */
extern char copyright[];	/* Copyright string */
extern char mode;			/* Current execution mode */
extern int errno;			/* System error return code */
extern int level;		/* Minor release level number */
extern int optind;		/* Index of first non-option arg */
extern int release;		/* Major release number */
extern int sys_nerr;			/* Size of sys_errlist[] */
extern int variant;		/* Variant of bru */
extern long sfthreshold;	/* Sparse file size threshold */
extern long timezone;
extern struct bru_info info;	/* Current invocation information */
extern struct cmd_flags flags;		/* Flags given on command line */
extern struct device *ardp;		/* Currently active device */
extern struct exe_info einfo;	/* Execution information */
extern struct finfo afile;	/* Archive file info */
extern struct utsname utsname;
extern time_t artime;			/* Time read from existing archive */
extern time_t ntime;		/* Time given by -n option */

/*
 *	Varargs functions.
 */

extern VOID bru_message PROTO((int msgno, ...));
extern VOID tty_printf PROTO((char *str, ...));
extern VOID voutput PROTO((char *format, ...));
extern int s_fprintf PROTO((FILE *stream, char *format, ...));
extern int s_sprintf PROTO((char *s, char *format, ...));

/*
 *	Functions defined in standard ANSI C environment header files.
 */

#ifndef __STDC__
/* From <unistd.h> */
extern char *sbrk PROTO((int));
extern int _exit PROTO((int));
extern int access PROTO((char *path, int amode));
extern int execv PROTO((char *path, char *argv[]));
extern int fork PROTO((void));
extern int getgid PROTO((void));
extern int getuid PROTO((void));
extern int ioctl PROTO((int fildes, int request, int arg));
extern int link PROTO((char *path1, char *path2));
extern int read PROTO((int fildes, char *buf, unsigned int nbytes));
extern int setgid PROTO ((int gid));
extern int setuid PROTO ((int uid));
extern int unlink PROTO ((char *path));
extern int write PROTO ((int fildes, char *buf, unsigned int nbytes));
/* From <stdlib.h> */
extern int atoi PROTO((char *));
extern int getopt PROTO((int argc, char **argv, char *opts));
extern int mkdir PROTO((char *path, int mode));
extern long strtol PROTO((char *str, char **ptr, int base));
extern VOID free PROTO((VOID *memptr));
extern char *malloc PROTO((int));
extern VOID exit PROTO((int status));			/* See exit(2) */
extern char *getenv PROTO((char *));
/* From <string.h> */
extern char *strcat PROTO((char *s1, char *s2));
extern char *strcpy PROTO((char *s1, char *s2));
extern char *strncpy PROTO((char *s1, char *s2, int n));
extern char *strtok PROTO((char *s1, char *s2));
extern int strlen PROTO((char *s));
#endif

#if amigados

extern LONG sendpkt PROTO((struct MsgPort *id, LONG type, LONG args [], LONG nargs));

extern void _abort PROTO((void));
extern void abort PROTO((void));
extern void amiga_tree_add PROTO((char *arg));
extern void chkabort PROTO((void));
extern void clear_archived PROTO((struct finfo *fip));
extern void unix2dos PROTO((long unixtime, struct DateStamp *datep));
extern void mark_archived PROTO((struct finfo *fip));

extern int AccessRawFloppy PROTO((char *filename, int mode));
extern int AccessRawTape PROTO((char *filename, int mode));
extern int AddRawDevice PROTO((struct device *dp));
extern int CloseRawFloppy PROTO((int fd));
extern int CloseRawTape PROTO((int fd));
extern int DisableDevice PROTO((char *name));
extern int EnableDevice PROTO((char *name));
extern int FlushRawFloppy PROTO((void));
extern int IsRawFloppyFd PROTO((int fd));
extern int IsRawTapeFd PROTO((int fd));
extern int MotorOff PROTO((void));
extern int OpenRawFloppy PROTO((char *name, int oflag, int mode));
extern int OpenRawTape PROTO((char *name, int oflag, int mode));
extern int ReadRawFloppy PROTO((int fd, char *buf, int bufsize));
extern int ReadRawTape PROTO((int fd, char *buf, int bufsize));
extern int WriteRawFloppy PROTO((int fd, char *buf, int bufsize));
extern int WriteRawTape PROTO((int fd, char *buf, int bufsize));
extern int abit_set PROTO((struct finfo *fip));
extern int access PROTO((char *path, int mode));
extern int getcomment PROTO((char *path, char *buffer));
extern int getinfo PROTO((struct finfo *fip));
extern int getopt PROTO((int argc, char **argv, char *optstring));
extern int ipc_error PROTO((char *buf));
extern int ipc_getc PROTO((void));
extern int ipc_getfilename PROTO((char *buf));
extern int ipc_init PROTO((void));
extern int ipc_query PROTO((char *buf));
extern int setcomment PROTO((char *path, char *buffer));
extern int setinfo PROTO((struct finfo *fip));
extern int setprot PROTO((char *path, long prot));
extern int stat PROTO((char *path, struct stat *buf));
extern int utime PROTO((char *path, struct utimbuf *times));

extern long LseekRawFloppy PROTO((int fd, long offset, int origin));
extern long LseekRawTape PROTO((int fd, long offset, int origin));
extern long getprot PROTO((char *path));
extern long dos2unix PROTO((struct DateStamp *datep));
extern long unix2amiga PROTO((long utime));
extern long amiga2unix PROTO((long mtime));

extern void SetErrno PROTO((void));
extern void ipc_cleanup PROTO((void));
extern void stackcheck PROTO((void));

#endif

/*
 *	Prototypes which are defined in AmigaDOS include files.
 *	So include them in other environments.
 */

#if !amigados
#if !defined(_STYPES)
extern long wait PROTO ((int *stat_loc));
#else
extern int wait PROTO ((int *stat_loc));
#endif
#endif

#endif	/* PRECOMPILED_HEADERS */
