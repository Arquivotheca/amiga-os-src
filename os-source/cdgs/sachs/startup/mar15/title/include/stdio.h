/* Copyright (c) 1992 SAS Institute, Inc., Cary, NC USA */
/* All Rights Reserved */


#ifndef _STDIO_H
#define _STDIO_H 1

#ifndef _SIZE_T
#define _SIZE_T 1
typedef unsigned int size_t;
#endif

#ifndef _VA_LIST
#define _VA_LIST 1
typedef char *va_list;
#endif

typedef unsigned long fpos_t;

#ifndef NULL
#define NULL   0L
#endif

/**
*
* Definitions associated with __iobuf._flag
*
**/

#define _IOFBF   0		/* fully buffered (for setvbuf) */
#define _IOREAD  1		/* read flag */
#define _IOWRT   2		/* write flag */
#define _IONBF   4		/* non-buffered flag */
#define _IOMYBUF 8		/* private buffer flag */
#define _IOEOF   16		/* end-of-file flag */
#define _IOERR   32		/* error flag */
#define _IOLBF   64		/* line-buffered flag */
#define _IORW    128		/* read-write (update) flag */
#define _IORKEY  0x2000         /* raw console I/O flag */
#define _IOAPP   0x4000		/* append flag */
#define _IOXLAT  0x8000		/* translation flag */

#define BUFSIZ 512		/* standard buffer size */
#define EOF (-1)		/* end-of-file code */
#define FOPEN_MAX 20            /* maximum number of open files */
#define FILENAME_MAX  64        /* maximum filename length */
#define L_tmpnam 64             /* maximum tmpnam filename length */

#define SEEK_SET 0              /* Seek from beginning of file */
#define SEEK_CUR 1              /* Seek from current file position */
#define SEEK_END 2              /* Seek from end of file */

#define TMP_MAX 999             /* Guaranteed unique temp names */

struct __iobuf {
    struct __iobuf *_next;
    unsigned char *_ptr;	/* current buffer pointer */
    int _rcnt;		        /* current byte count for reading */
    int _wcnt;		        /* current byte count for writing */
    unsigned char *_base;	/* base address of I/O buffer */
    int _size;			/* size of buffer */
    int _flag;	        	/* control flags */
    int _file;		        /* file descriptor */
    unsigned char _cbuff;	/* single char buffer */
};

typedef struct __iobuf FILE;

extern struct __iobuf __iob[];

#define stdin (&__iob[0])	/* standard input file pointer */
#define stdout (&__iob[1])	/* standard output file pointer */
#define stderr (&__iob[2])	/* standard error file pointer */


/***
*
*     Prototypes for ANSI standard functions.
*
***/


extern int remove(const char *);
extern int rename(const char *, const char *);
extern FILE *tmpfile(void);
extern char *tmpnam(char *s);

extern int fclose(FILE *);
extern int fflush(FILE *);
extern FILE *fopen(const char *, const char *);
extern FILE *freopen(const char *, const char *, FILE *);
extern void setbuf(FILE *, char *);
extern int setvbuf(FILE *, char *, int, size_t);

extern int fprintf(FILE *, const char *, ...);
extern int fscanf(FILE *, const char *, ...);
extern int printf(const char *, ...);
extern int __builtin_printf(const char *, ...);
extern int scanf(const char *, ...);
extern int sprintf(char *, const char *, ...);
extern int sscanf(const char *, const char *, ...);
extern int vfprintf(FILE *, const char *, va_list);
extern int vprintf(const char *, va_list);
extern int vsprintf(char *, const char *, va_list);

extern int fgetc(FILE *);
extern char *fgets(char *, int, FILE *);
extern int fputc(int, FILE *);
extern int fputs(const char *, FILE *);
extern int getc(FILE *);
#define getc(p) \
        (((p)->_flag & _IOREAD) ? \
            (--(p)->_rcnt >= 0 && ! ((p)->_file & _IORKEY) ? \
                *(p)->_ptr++ \
            : \
                fgetc(p)) \
        : \
            fgetc(p))
extern int getchar(void);
#define getchar() getc(stdin)
extern char *gets(char *);
extern int putc(int, FILE *);

#define putc(c,p) \
        (((p)->_flag & _IOWRT) ? \
            (++(p)->_wcnt <= (p)->_size ? \
                ((((int) (*(p)->_ptr++ = ((unsigned char)c))) == '\n' && \
                   ((p)->_flag & _IOLBF)) ? \
                    fflush(p), ((unsigned char)c) \
                : \
                    ((unsigned char)c)) \
            : \
                (--(p)->_wcnt,fputc(c,p))) \
        : \
             fputc(c,p))


extern int putchar(int);
#define putchar(c) putc(c,stdout)
extern int puts(const char *);
extern int ungetc(int, FILE *);

extern size_t fread(void *, size_t, size_t, FILE *);
extern size_t fwrite(const void *, size_t, size_t, FILE *);
extern int fgetpos(FILE *, fpos_t *);
extern int fseek(FILE *, long int, int);
extern int fsetpos(FILE *, const fpos_t *);
extern long int ftell(FILE *);
extern void rewind(FILE *);
#define rewind(p) fseek(p, 0L, 0)

extern void clearerr(FILE *);
#define clearerr(p) ((p)->_flag &= ~(_IOERR | _IOEOF))
extern int feof(FILE *);
#define feof(p) (((p)->_flag & _IOEOF) != 0)
extern int ferror(FILE *);
#define ferror(p) (((p)->_flag & _IOERR) != 0)
extern void perror(const char *);

#define printf __builtin_printf

#ifndef _STRICT_ANSI

/* defines for mode of access() */
#define R_OK 4
#define W_OK 2
#define X_OK 1
#define F_OK 0

/***
*
*     Prototypes for Non-ANSI functions.
*
***/

extern int __io2errno(int);
extern int fcloseall(void);
extern FILE *fdopen(int, const char *);
extern int fhopen(long, int);
extern int fgetchar(void);
extern int fileno(FILE *);
extern int flushall(void);
extern void fmode(FILE *, int);
extern int _writes(const char *, ...);
extern int _tinyprintf(char *, ...);
extern int fputchar(int);
extern void setnbf(FILE *);
extern int __fillbuff(FILE *);
extern int __flushbuff(int, FILE *);
extern int __access(const char *, int);
extern int access(const char *, int);
extern int chdir(const char *);
extern int chmod(const char *, int);
extern char *getcwd(const char *, int);
extern int unlink(const char *);
extern int poserr(const char *);

#define  clrerr  clearerr
#define  access  __access
#define  _filbf  __fillbuff
#define  fileno(p) ((p)->_file)
#define  _flsbf  __flushbuff


/***
*
*     The following routines allow for raw console I/O.
*
***/

int rawcon(int);
int getch(void);

#endif /* _STRICT_ANSI */

extern unsigned long __fmask;
extern int __fmode;

#endif
