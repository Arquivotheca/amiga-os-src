/*
 * dslib.h
 */

struct dslib_struct {
	struct ServiceData *sd_ds;	/* ptr to ServiceData for DOSServ */
	struct DOSServReq *ds_rq;	/* ptr to DOSServReq struct */
	ULONG currentdir;		/* lock on current directory */
	ULONG oldcd;			/* lock on original current directory */
	ULONG IoErr;			/* last AmigaDOS err code */
	UBYTE *ds_buf;			/* ptr to DOSServ buffer in shmem */
	UWORD ds_buf_len;		/* size of DOSServ buf in shmem */
	UBYTE errno;			/* error flag */
};

struct DateStamp {
   LONG	 ds_Days;	      /* Number of days since Jan. 1, 1978 */
   LONG	 ds_Minute;	      /* Number of minutes past midnight */
   LONG	 ds_Tick;	      /* Number of ticks past minute */
}; /* DateStamp */
#define TICKS_PER_SECOND      50   /* Number of ticks in one second */

struct FileInfoBlock {
   LONG	  fib_DiskKey;
   LONG	  fib_DirEntryType;  /* Type of Directory. If < 0, then a plain file.
			      * If > 0 a directory */
   char	  fib_FileName[108]; /* Null terminated. Max 30 chars used for now */
   LONG	  fib_Protection;    /* bit mask of protection, rwxd are 3-0.	   */
   LONG	  fib_EntryType;
   LONG	  fib_Size;	     /* Number of bytes in file */
   LONG	  fib_NumBlocks;     /* Number of blocks in file */
   struct DateStamp fib_Date;/* Date file last changed */
   char	  fib_Comment[80];  /* Null terminated comment associated with file */
   char	  fib_Reserved[36];
}; /* FileInfoBlock */

/* FIBB are bit definitions, FIBF are field definitions */
#define FIBB_SCRIPT    6	/* program is a script (execute) file */
#define FIBB_PURE      5	/* program is reentrant and rexecutable*/
#define FIBB_ARCHIVE   4		/* cleared whenever file is changed */
#define FIBB_READ      3		/* ignored by old filesystem */
#define FIBB_WRITE     2		/* ignored by old filesystem */
#define FIBB_EXECUTE   1		/* ignored by system, used by Shell */
#define FIBB_DELETE    0		/* prevent file from being deleted */
#define FIBF_SCRIPT    (1<<FIBB_SCRIPT)
#define FIBF_PURE      (1<<FIBB_PURE)
#define FIBF_ARCHIVE   (1<<FIBB_ARCHIVE)
#define FIBF_READ      (1<<FIBB_READ)
#define FIBF_WRITE     (1<<FIBB_WRITE)
#define FIBF_EXECUTE   (1<<FIBB_EXECUTE)
#define FIBF_DELETE    (1<<FIBB_DELETE)

/* amigados Seek() modes */
#define OFFSET_BEGINNING	(-1L)
#define OFFSET_CURRENT		0L
#define OFFSET_END		1L

/* amigados Open() modes */
#define MODE_READWRITE	1004L
#define MODE_OLDFILE	1005L
#define MODE_NEWFILE	1006L

/* amigados Lock() accessmode */
#define SHARED_LOCK	     (-2L)	    /* File is readable by others */
#define ACCESS_READ	     (-2L)	    /* Synonym */
#define EXCLUSIVE_LOCK	     (-1L)	    /* No other access allowed	  */
#define ACCESS_WRITE	     (-1L)	    /* Synonym */

/* Errors from IoErr(), etc. */
#define ERROR_NO_FREE_STORE		  103
#define ERROR_TASK_TABLE_FULL		  105
#define ERROR_BAD_TEMPLATE		  114
#define ERROR_BAD_NUMBER		  115
#define ERROR_REQUIRED_ARG_MISSING	  116
#define ERROR_KEY_NEEDS_ARG		  117
#define ERROR_TOO_MANY_ARGS		  118
#define ERROR_UNMATCHED_QUOTES		  119
#define ERROR_LINE_TOO_LONG		  120
#define ERROR_FILE_NOT_OBJECT		  121
#define ERROR_INVALID_RESIDENT_LIBRARY	  122
#define ERROR_NO_DEFAULT_DIR		  201
#define ERROR_OBJECT_IN_USE		  202
#define ERROR_OBJECT_EXISTS		  203
#define ERROR_DIR_NOT_FOUND		  204
#define ERROR_OBJECT_NOT_FOUND		  205
#define ERROR_BAD_STREAM_NAME		  206
#define ERROR_OBJECT_TOO_LARGE		  207
#define ERROR_ACTION_NOT_KNOWN		  209
#define ERROR_INVALID_COMPONENT_NAME	  210
#define ERROR_INVALID_LOCK		  211
#define ERROR_OBJECT_WRONG_TYPE		  212
#define ERROR_DISK_NOT_VALIDATED	  213
#define ERROR_DISK_WRITE_PROTECTED	  214
#define ERROR_RENAME_ACROSS_DEVICES	  215
#define ERROR_DIRECTORY_NOT_EMPTY	  216
#define ERROR_TOO_MANY_LEVELS		  217
#define ERROR_DEVICE_NOT_MOUNTED	  218
#define ERROR_SEEK_ERROR		  219
#define ERROR_COMMENT_TOO_BIG		  220	
#define ERROR_DISK_FULL			  221
#define ERROR_DELETE_PROTECTED		  222
#define ERROR_WRITE_PROTECTED		  223 
#define ERROR_READ_PROTECTED		  224
#define ERROR_NOT_A_DOS_DISK		  225
#define ERROR_NO_DISK			  226
#define ERROR_NO_MORE_ENTRIES		  232
/* added for 1.4 */
#define ERROR_IS_SOFT_LINK		  233
#define ERROR_OBJECT_LINKED		  234
#define ERROR_BAD_HUNK			  235
#define ERROR_NOT_IMPLEMENTED		  236
#define ERROR_RECORD_NOT_LOCKED		  240
#define ERROR_LOCK_COLLISION		  241
#define ERROR_LOCK_TIMEOUT		  242
#define ERROR_UNLOCK_ERROR		  243

/* error codes 303-305 are defined in dosasl.h */

/* stuff to access this library */
UBYTE j_tickle_janus();
struct dslib_struct *j_open_dosserv();
UBYTE j_close_dosserv(struct dslib_struct *ds);

/* amigados stuff */
ULONG j_CreateDir(struct dslib_struct *ds, char *name);
ULONG j_CurrentDir(struct dslib_struct *ds, ULONG newlock);
ULONG j_DeleteFile(struct dslib_struct *ds, char *name);
ULONG j_DupLock(struct dslib_struct *ds, ULONG oldlock);
ULONG j_Examine(struct dslib_struct *ds, ULONG lock, struct FileInfoBlock *infoblock);
ULONG j_ExNext(struct dslib_struct *ds, ULONG lock, struct FileInfoBlock *infoblock);
ULONG j_Lock(struct dslib_struct *ds, char *name, LONG accessmode);
LONG j_ParsePattern(struct dslib_struct *ds, char *sourcepat, char *destpat, LONG destlen);
ULONG j_MatchPattern(struct dslib_struct *ds, char *pat, char *str);
ULONG j_ParentDir(struct dslib_struct *ds, ULONG oldlock);
ULONG j_Rename(struct dslib_struct *ds, char *oldname, char *newname);
void j_UnLock(struct dslib_struct *ds, ULONG lock);
void j_Close(struct dslib_struct *ds, ULONG afile);
LONG j_Read(struct dslib_struct *ds, ULONG afile, void *bufin, LONG n_bytes);
LONG j_Write(struct dslib_struct *ds, ULONG afile, void *bufin, LONG n_bytes);
LONG j_Seek(struct dslib_struct *ds, ULONG afile, LONG offset, LONG mode);
ULONG j_Open(struct dslib_struct *ds, char *fname, LONG mode);
#if 0
ULONG j_IsFileSystem(struct dslib_struct *ds, char *name);
#endif
ULONG j_SetProtection(struct dslib_struct *ds, char *name, LONG bits);
ULONG j_SetFileDate(struct dslib_struct *ds, char *fname, struct DateStamp *d_s);
ULONG j_IoErr(struct dslib_struct *ds);

