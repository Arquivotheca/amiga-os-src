#ifndef fileno

#ifndef _VA_LIST
#define _VA_LIST 1
typedef char *va_list;
#endif

/**
*
* This header file defines the information used by the standard I/O
* package.
*
**/
#define _BUFSIZ 512		/* standard buffer size */
#define BUFSIZ 512		/* standard buffer size */
#define _NFILE 20		/* maximum number of files */

struct _iobuf
{
struct _iobuf *_next;
unsigned char *_ptr;		/* current buffer pointer */
int _rcnt;			/* current byte count for reading */
int _wcnt;			/* current byte count for writing */
unsigned char *_base;		/* base address of I/O buffer */
int _size;			/* size of buffer */
int _flag;			/* control flags */
int _file;			/* file descriptor */
unsigned char _cbuff;		/* single char buffer */
};

extern struct _iobuf _iob[];

/**
*
* Definitions associated with _iobuf._flag
*
**/
#define _IOFBF 0		/* fully buffered (for setvbuf) */
#define _IOREAD 1		/* read flag */
#define _IOWRT 2		/* write flag */
#define _IONBF 4		/* non-buffered flag */
#define _IOMYBUF 8		/* private buffer flag */
#define _IOEOF 16		/* end-of-file flag */
#define _IOERR 32		/* error flag */
#define _IOLBF 64		/* line-buffered flag */
#define _IORW 128		/* read-write (update) flag */
#define _IOAPP 0x4000		/* append flag */
#define _IOXLAT 0x8000		/* translation flag */

#ifndef NULL
#define NULL 0L
#endif
#define FILE struct _iobuf	/* shorthand */
#define EOF (-1)		/* end-of-file code */


#define stdin (&_iob[0])	/* standard input file pointer */
#define stdout (&_iob[1])	/* standard output file pointer */
#define stderr (&_iob[2])	/* standard error file pointer */

#define getc(p) (--(p)->_rcnt>=0? *(p)->_ptr++:_filbf(p))
#define getchar() getc(stdin)
#define putc(c,p) (--(p)->_wcnt>=0 ?((int)(*(p)->_ptr++=(c))):_flsbf((unsigned char)(c),p))
#define putchar(c) putc(c,stdout)
#define feof(p) (((p)->_flag&_IOEOF)!=0)
#define ferror(p) (((p)->_flag&_IOERR)!=0)
#define fileno(p) (p)->_file
#define rewind(fp) fseek(fp,0L,0)
#define fflush(fp) _flsbf(-1,fp)
#define clearerr(fp) clrerr(fp)

#ifndef __ARGS
#ifdef NARGS
#define __ARGS(a) ()
#else
#define __ARGS(a) a
#endif
#endif

#define printf __builtin_printf
extern void clrerr __ARGS((FILE *));
extern int fclose __ARGS((FILE *));
extern int fcloseall __ARGS((void));
extern FILE *fdopen __ARGS((int, char *));
extern int fgetc __ARGS((FILE *));
extern int fgetchar __ARGS((void));
extern char *fgets __ARGS((char *, int, FILE *));
extern int flushall __ARGS((void));
extern int fmode __ARGS((FILE *, int));
extern FILE *fopen __ARGS((char *, char *));
extern int fprintf __ARGS((FILE *, char *,...));
extern int _writes __ARGS((char *,...));
extern int _tinyprintf __ARGS((char *,...));
extern int fputc __ARGS((int, FILE *));
extern int fputchar __ARGS((int));
extern int fputs __ARGS((char *, FILE *));
extern int fread __ARGS((char *, int, int, FILE *));
extern FILE *freopen __ARGS((char *, char *, FILE *));
extern int fscanf __ARGS((FILE*, char *,...));
extern int fseek __ARGS((FILE *, long, int));
extern long ftell __ARGS((FILE *));
extern int fwrite __ARGS((char *, int, int, FILE *));
extern char *gets __ARGS((char *));
extern int printf __ARGS((char *,...));
extern int puts __ARGS((char *));
extern int scanf __ARGS((char *,...));
extern int setbuf __ARGS((FILE *, char *));
extern int setnbf __ARGS((FILE *));
extern int setvbuf __ARGS((FILE*, char *, int, int));
extern int sprintf __ARGS((char *, char *,...));
extern int sscanf __ARGS((char *, char *,...));
extern int ungetc __ARGS((int, FILE *));
extern int _filbf __ARGS((FILE *));
extern int _flsbf __ARGS((int, FILE *));
extern int access __ARGS((char *, int));
extern int chdir __ARGS((char *));
extern int chmod __ARGS((char *, int));
extern char *getcwd __ARGS((char *, int));
extern int mkdir __ARGS((char *));
extern int perror __ARGS((char *));
extern int rename __ARGS((char *, char *));
extern int unlink __ARGS((char *));
extern int remove __ARGS((char *));
extern int rmdir __ARGS((char *));
extern int vprintf __ARGS((char *, va_list));
extern int vfprintf __ARGS((FILE *, char *, va_list));
extern int vsprintf __ARGS((char *, char *, va_list));

#ifndef abs
#define abs(x) ((x)<0?-(x):(x))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<=(b)?(a):(b))
#endif
#endif
