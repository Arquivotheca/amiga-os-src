/* Prototypes for functions defined in
crcio.c
 */


#ifndef __NOPROTO

#ifndef __PROTO
#define __PROTO(a) a
#endif

#else
#ifndef __PROTO
#define __PROTO(a) ()

#endif
#endif


extern long reading_size;

extern BPTR infile;

extern BPTR outfile;

extern unsigned short crc;

extern unsigned short bitbuf;

unsigned short calccrc __PROTO((unsigned char * , unsigned int ));

void fillbuf __PROTO((unsigned char ));

unsigned short getbits __PROTO((unsigned char ));

void putcode __PROTO((unsigned char , unsigned short ));

void putbits __PROTO((unsigned char , unsigned short ));

int fread_crc __PROTO((unsigned char * , int , BPTR ));

void fwrite_crc __PROTO((unsigned char * , int , BPTR ));

void init_code_cache __PROTO((void));

void init_getbits __PROTO((void));

void init_putbits __PROTO((void));

int fwrite_txt __PROTO((unsigned char * , int , BPTR ));

int fread_txt __PROTO((unsigned char * , int , BPTR ));

