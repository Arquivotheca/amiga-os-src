/* Prototypes for functions defined in
slide.c
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


extern node * next;

extern int unpackable;

extern unsigned long origsize;

extern unsigned long compsize;

extern unsigned short dicbit;

extern unsigned short maxmatch;

extern unsigned long count;

extern unsigned short loc;

extern unsigned char * text;

extern int prev_char;

void decode __PROTO((struct interfacing * ));

