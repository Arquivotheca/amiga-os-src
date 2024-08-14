/*
    C Runtime library - Copyright 1983,1984,1985,1986 by Green Hills Software Inc.
    This program is the property of Green Hills Software, Inc,
    its contents are proprietary information and no part of it
    is to be disclosed to anyone except employees of Green Hills
    Software, Inc., or as agreed in writing signed by the President
    of Green Hills Software, Inc.
*/
#ifndef FILE
#define NULL	0
#define EOF		(-1)
#define BUFSIZ		512	/* buffer size for all IO buffers */
#define BLOCKSIZE	2048	/* block size to allocate with sbrk in malloc */

#define _NFILE	20

extern struct iobuf {
    unsigned char *io_next;	/* next character to be read from buffer or
				   next position to put a character in buffer */
    unsigned char *io_base;	/* start of buffer */
    long  io_left;		/* number of characters left in buffer */
    short io_channel;		/* OS I/O channel number */
    char  io_tmp;		/* used by ungetc on unbuffered files */
    unsigned io_buffering:2;	/* UN_BUFFER,BLOCK_BUFFER,LINE_BUFFER*/
    unsigned io_eof: 1;		/* have read end of file */
    unsigned io_error:1;	/* have detected io error in file */
    unsigned io_stdio_buffer:1;	/* buffer for this file created by stdio */
    unsigned io_readable:1;	/* file may be read at this time */
    unsigned io_writable:1;	/* file may be written at this time */
    unsigned io_readwrite:1;	/* file opened for both reading and writing */
} _iob[_NFILE];

#define	UN_BUFFER	0
#define	BLOCK_BUFFER	1
#define	LINE_BUFFER	2
#define STRING_BUFFER	3

#define FILE struct iobuf
#define stdin (&_iob[0])
#define stdout (&_iob[1])
#define stderr (&_iob[2])

#define getc(f)		(--(f)->io_left<0?_filbuf(f):*(f)->io_next++)
#define getchar()	getc(stdin)
#define putc(ch,f)	(--(f)->io_left<0?_flsbuf((ch),f):*(f)->io_next++=(ch))
#define putchar(ch)	putc(ch,stdout)
#define fileno(file)	((file)->io_channel)
#define feof(file)	((file)->io_eof)
#define ferror(file)	((file)->io_error)
#define clearerr(file)	((file)->io_eof=0,(file)->io_error=0)
#endif

char *fgets(), *gets();
FILE *fopen(), *fdopen(), *freopen();
long strtol(), ftell(), getl(), putl();
