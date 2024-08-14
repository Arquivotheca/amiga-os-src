/*----------------------------------------------------------------------*/
/*		LHarc Archiver Driver for AmigaDOS				*/
/*									*/
/*		Copyright(C) MCMLXXXIX  Yooichi.Tagawa			*/
/*									*/
/*  V1.00  Fixed				1989.09.22  Y.Tagawa	*/
/*  V0.01  LHa for AmigaDOS			1992.12.08  D.Miller	*/
/*----------------------------------------------------------------------*/


#include <stdio.h>

#ifdef LHEXTRACT
#undef stderr
#define stderr (&__iob[1])
#endif

#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

#include <dos/dos.h>
#include <dos/stdio.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#ifdef	PRAGMAS
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#endif
extern struct Library *SYSBase;
extern struct Library *DOSBase;


#ifndef	INLINE
# define	INLINE	__inline
#endif

extern int	S_T_S;
#if DO_INLINE
extern char	STS_Buff[8];

# define	myFGetC(f)		(S_T_S ? _myfgetc(f) : FGetC(f))
# define	myFPutC(f,c)		(S_T_S ? _myfputc(f,c) : FPutC(f,c))
# define	myFGets(f,b,l)		(S_T_S ? _myfgets(f,b,l) : FGets(f,b,l))
# define	myFRead(f,b,l,n)	(S_T_S ? _myfread(f,b,l,n) : FRead(f,b,l,n))
# define	myFWrite(f,b,l,n)	(S_T_S ? _myfwrite(f,b,l,n) : FWrite(f,b,l,n))
# define	myFlush(f)		(S_T_S || Flush(f))

STATIC INLINE LONG _myfgetc(BPTR fh)
{
	unsigned char	buf[2];
	
	if (Read(fh, buf, 1) != 1)
		return -1;
	return buf[0];
}

STATIC INLINE LONG _myfputc(BPTR fh, LONG c)
{
	char	buf[2];
	
	buf[0] = c;
	buf[1] = 0;
	
	if (Write(fh, buf, 1) != 1)
		return -1;
	return c;
}

STATIC INLINE STRPTR _myfgets(BPTR fh, STRPTR buffer, ULONG len)
{
	int	i = 0;
	char	inbuf = 0;

	if (Read(fh, &inbuf, 1) != 1)
		return NULL;
	
	do
		buffer[i++] = inbuf;
	while (i < (len-1) && inbuf != '\n' && Read(fh, &inbuf, 1) == 1);
	
	buffer[i] = 0;
	
	return buffer;
}

STATIC INLINE LONG _myfread(BPTR fh, STRPTR buffer, ULONG len, ULONG num)
{
	int	i;
	
	for (i = 0; i < num; i++)
	{
	    if (Read(fh, buffer + (i * len), len) != len)
	    	break;
	}
	return(i);
}

STATIC INLINE LONG _myfwrite(BPTR fh, STRPTR buffer, ULONG len, ULONG num)
{
	int	i;
	
	for (i = 0; i < num; i++)
	{
	    if (Write(fh, buffer + (i * len), len) != len)
	    	break;
	}

	return(i);
}
#else
LONG myFGetC(BPTR f);
LONG myFPutC(BPTR f,LONG c);
STRPTR myFGets(BPTR f, STRPTR b,ULONG l);
LONG myFRead(BPTR f,STRPTR b,ULONG l,ULONG n);
LONG myFWrite(BPTR f,STRPTR b,ULONG l,ULONG n);
LONG myFlush(BPTR f);

LONG _myfgetc(BPTR fh);
LONG _myfputc(BPTR fh, LONG c);
STRPTR _myfgets(BPTR fh, STRPTR buffer, ULONG len);
LONG _myfread(BPTR fh, STRPTR buffer, ULONG len, ULONG num);
LONG _myfwrite(BPTR fh, STRPTR buffer, ULONG len, ULONG num);
struct RDArgs * _myparseargs(int argc, char ** argv,LONG r[]);
void _myfreeargs(struct RDArgs *);
#endif


/*----------------------------------------------------------------------*/
/*			DIRECTORY ACCESS STUFF				*/
/*----------------------------------------------------------------------*/
#include <dirent.h>
#define DIRENTRY	struct dirent
#define NAMLEN(p)	strlen (p->d_name)

/*----------------------------------------------------------------------*/
/*				FILE ATTRIBUTES				*/
/*----------------------------------------------------------------------*/

/* #define WRITE_BINARY	"wb" */
/* #define READ_BINARy	"rb" */
#define WRITE_BINARY	"w"
#define READ_BINARY	"r"

/*----------------------------------------------------------------------*/
/*			MEMORY AND STRING FUNCTIONS			*/
/*----------------------------------------------------------------------*/

#ifndef USG
#include <strings.h>
#else
#include <string.h>
#endif	/* USG */

/*----------------------------------------------------------------------*/
/*				YOUR CUSTOMIZIES			*/
/*----------------------------------------------------------------------*/
/* These definitions are changable to you like. 			*/

#ifndef ARCHIVENAME_EXTENTION
#define ARCHIVENAME_EXTENTION	".lha"
#endif
#ifndef BACKUPNAME_EXTENTION
#define BACKUPNAME_EXTENTION	".bak"
#endif
#ifndef TMP_FILENAME_TEMPLATE
#define TMP_FILENAME_TEMPLATE	"T:lhXXXXXX"
#endif

#define SJC_FIRST_P(c)			\
  (((unsigned char)(c) >= 0x80) &&	\
   (((unsigned char)(c) < 0xa0) ||	\
    ((unsigned char)(c) >= 0xe0) &&	\
    ((unsigned char)(c) < 0xfd)))
    
#define SJC_SECOND_P(c)			\
  (((unsigned char)(c) >= 0x40) &&	\
   ((unsigned char)(c) < 0xfd) &&	\
   ((ungigned char)(c) != 0x7f))

#ifdef MULTIBYTE_CHAR
#define MULTIBYTE_FIRST_P	SJC_FIRST_P
#define MULTIBYTE_SECOND_P	SJC_SECOND_P
#endif /* MULTIBYTE_CHAR */

/*----------------------------------------------------------------------*/
/*			OTHER DIFINITIONS				*/
/*----------------------------------------------------------------------*/
#ifndef SEEK_SET
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2
#endif

#define FILENAME_LENGTH	1024



#define	FALSE	0
#define TRUE	1
typedef int boolean;


/*----------------------------------------------------------------------*/
/*			LHarc FILE DEFINITIONS				*/
/*----------------------------------------------------------------------*/
#define METHOD_TYPE_STRAGE	5
#define LZHUFF0_METHOD		"-lh0-"
#define LZHUFF1_METHOD		"-lh1-"
#define LZHUFF2_METHOD		"-lh2-"
#define LZHUFF3_METHOD		"-lh3-"
#define LZHUFF4_METHOD		"-lh4-"
#define LZHUFF5_METHOD		"-lh5-"
#define LARC4_METHOD		"-lz4-"
#define LARC5_METHOD		"-lz5-"
#define LZHDIRS_METHOD		"-lhd-"

#define I_HEADER_SIZE			0
#define I_HEADER_CHECKSUM		1
#define I_METHOD			2
#define I_PACKED_SIZE			7
#define I_SKIP_SIZE			7	/* for L1 headers */
#define I_ORIGINAL_SIZE			11
#define I_LAST_MODIFIED_STAMP		15
#define I_ATTRIBUTE			19
#define I_HEADER_LEVEL			20
#define I_NAME_LENGTH			21
#define I_NAME				22

#define I_CRC				22 /* + name_length */
#define I_EXTEND_TYPE			24 /* + name_length */
#define I_MINOR_VERSION			25 /* + name_length */
#define I_UNIX_LAST_MODIFIED_STAMP	26 /* + name_length */
#define I_UNIX_MODE			30 /* + name_length */
#define I_UNIX_UID			32 /* + name_length */
#define I_UNIX_GID			34 /* + name_length */
#define I_UNIX_EXTEND_BOTTOM		36 /* + name_length */

#define I_MINIMUM_HEADER_SIZE		20	/* No CRC, No Extended */
#define I_GENERIC_HEADER_SIZE		22	/*    CRC, No Extended */
#define I_EXTENDED_HEADER_SIZE	24		/*    CRC,    Extended */

#define	MY_L0_HEADERSIZ		22
#define	MY_L1_HEADERSIZ		25
#define	MY_L2_HEADERSIZ		26

/*
**	Strictly speaking, the number for L2 is wrong.  It's really
**	MAX_USHORT - (size of base header + size of all extensions)
**	But, since MAX_USHORT is 65535, I don't see this a being a
**	problem.  If it becomes one, you need to change MAX_L2_NAMELEN
**	and add a check to writehead_2(header.c)
*/
#define MAX_L0_NAMELEN			(((unsigned char)~0) - MY_L0_HEADERSIZ)
#define MAX_L1_NAMELEN			(((unsigned char)~0) - MY_L1_HEADERSIZ)
#define MAX_L2_NAMELEN			((unsigned short)~0)


#define EXTEND_GENERIC		0
#define EXTEND_UNIX		'U'
#define EXTEND_MSDOS		'M'
#define EXTEND_MACOS		'm'
#define EXTEND_OS9		'9'
#define EXTEND_OS2		'2'
#define EXTEND_OS68K		'K'
#define EXTEND_OS386		'3'		/* OS-9000??? */
#define EXTEND_HUMAN		'H'
#define EXTEND_CPM		'C'
#define EXTEND_FLEX		'F'
#define EXTEND_RUNSER		'R'
/*	this OS type is not official */
#define EXTEND_TOWNSOS		'T'
#define EXTEND_XOSK		'X'
/* using 'a', because Boberg's LhA uses 'A' in a different manner */
#define	EXTEND_AMIGADOS		'a'
/*------------------------------*/

#define	EXTEND_AMIGADOS_MODE			0x80
#define	EXTEND_AMIGADOS_DATE			0x81

# define	CLR_EOL		"\x9B\x4B"
# define	CLR_FWD		"\x9B\x43"
# define	CLR_BWD		"\x9B\x44"
# define	CLR_UP		"\x9B\x46"
# define	CLR_DN		"\x9B\x45"
# define	SGR_NONE	"\x9B\x30\x6D"
# define	SGR_BOLD	"\x9B\x31\x6D"
# define	SGR_DIM		"\x9B\x32\x6D"
# define	SGR_ITALIC	"\x9B\x33\x6D"
# define	SGR_UNDERLINE	"\x9B\x34\x6D"
# define	SGR_REVERSE	"\x9B\x37\x6D"


#define GENERIC_ATTRIBUTE			0x20
#define GENERIC_DIRECTORY_ATTRIBUTE		0x10
#define HEADER_LEVEL0				0x00
#define HEADER_LEVEL1				0x01
#define HEADER_LEVEL2				0x02

#define DELIM ('/')
#define DELIM2 (0xff)
#define DELIMSTR "/"

typedef struct LzHeader {
  unsigned char		header_size;
  char			method[METHOD_TYPE_STRAGE];
  long			packed_size;
  long			total_size;
  long			original_size;
  long			last_modified_stamp;
  unsigned char         attribute;
  unsigned char         header_level;
  char			*name;
  char			*comment;
  unsigned short	crc;
  boolean		has_crc;
  unsigned char		extend_type;
  unsigned char		minor_version;
  /*  extend_type == EXTEND_UNIX  and convert from other type. */
  struct DateStamp	ados_last_modified_stamp;
  unsigned long		ados_mode;
  unsigned short	unix_uid;
  unsigned short	unix_gid;
} LzHeader;

#define MAX_NAME_LEN			256

#define OSK_RW_RW_RW			0000033
#define OSK_FILE_REGULAR		0000000
#define OSK_DIRECTORY_PERM		0000200
#define OSK_SHARED_PERM			0000100
#define OSK_OTHER_EXEC_PERM		0000040
#define OSK_OTHER_WRITE_PERM		0000020
#define OSK_OTHER_READ_PERM		0000010
#define OSK_OWNER_EXEC_PERM		0000004
#define OSK_OWNER_WRITE_PERM		0000002
#define OSK_OWNER_READ_PERM		0000001

#define UNIX_FILE_TYPEMASK		0170000
#define UNIX_FILE_REGULAR		0100000
#define UNIX_FILE_DIRECTORY		0040000
#define UNIX_SETUID			0004000
#define UNIX_SETGID			0002000
#define UNIX_STYCKYBIT			0001000
#define UNIX_OWNER_READ_PERM		0000400
#define UNIX_OWNER_WRITE_PERM		0000200
#define UNIX_OWNER_EXEC_PERM		0000100
#define UNIX_GROUP_READ_PERM		0000040
#define UNIX_GROUP_WRITE_PERM		0000020
#define UNIX_GROUP_EXEC_PERM		0000010
#define UNIX_OTHER_READ_PERM		0000004
#define UNIX_OTHER_WRITE_PERM		0000002
#define UNIX_OTHER_EXEC_PERM		0000001
#define UNIX_RW_RW_RW			0000666

#define AMIGA_ALL_READ	  (FIBF_OTR_READ    | FIBF_GRP_READ)
#define AMIGA_ALL_WRITE	  (FIBF_OTR_WRITE   | FIBF_GRP_WRITE)
#define AMIGA_ALL_EXECUTE (FIBF_OTR_EXECUTE | FIBF_GRP_EXECUTE)
#define AMIGA_ALL_DELETE  (FIBF_OTR_DELETE  | FIBF_GRP_DELETE)

#define	AMIGA_DEF_MODE	  (AMIGA_ALL_READ|AMIGA_ALL_WRITE|AMIGA_ALL_EXECUTE|AMIGA_ALL_DELETE)

#define	AMIGA_DIRECTORY		(1<<7)

#define LZHEADER_STRAGE			4096

#include "header.h"

#ifdef S_IFLNK
#define GETSTAT lstat
#else
#define GETSTAT stat
#endif

/* used by qsort() for alphabetic-sort */
#define STRING_COMPARE(a,b) strcmp((a),(b))

struct string_pool {
  int used;
  int size;
  int n;
  char *buffer;
};


/*----------------------------------------------------------------------*/
/*				OPTIONS					*/
/*----------------------------------------------------------------------*/

/* command line options (common options) */
extern boolean	quiet;
extern boolean	text_mode;
extern boolean	verbose;
extern boolean	noexec;		/* debugging option */
extern boolean	force;
extern boolean	prof;
extern boolean	delete_after_append;

/* list command flags */
extern boolean	verbose_listing;

/* extract/print command flags */
extern boolean	output_to_stdout;

/* add/update/delete command flags */
extern boolean	new_archive;
extern boolean	update_if_newer;
extern boolean	generic_format;


/*----------------------------------------------------------------------*/
/*				VARIABLES				*/
/*----------------------------------------------------------------------*/

extern char	**cmd_filev;
extern int	cmd_filec;

extern char	*archive_name;
extern char	expanded_archive_name[FILENAME_LENGTH];
extern char	temporary_name[FILENAME_LENGTH];
extern char	backup_archive_name[FILENAME_LENGTH];

extern const char *reading_filename, *writting_filename;
extern boolean	remove_temporary_at_error;
extern boolean	recover_archive_when_interrupt;
extern boolean	remove_extracting_file_when_interrupt;

extern int	archive_file_mode;
extern int	archive_file_gid;

/*----------------------------------------------------------------------*/
/*				Functions				*/
/*----------------------------------------------------------------------*/
extern boolean ignore_directory;
extern boolean compress_method;
extern boolean verify_mode;

extern char *extract_directory;
extern BPTR temporary_fp;

#include "intrface.h"
#include "append.pro"
#include "crcio.pro"
#include "dhuf.pro"
#include "extract.pro"
#include "header.pro"
#include "huf.pro"
#include "larc.pro"
#include "lhadd.pro"
#include "lharc.pro"
#include "lhext.pro"
#include "lhlist.pro"
#include "maketbl.pro"
#include "maketree.pro"
#include "patmatch.pro"
#include "shuf.pro"
#include "slide.pro"
#include "util.pro"
#include "dynlists.pro"
