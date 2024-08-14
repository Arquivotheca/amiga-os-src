/* Prototypes for functions defined in
maketbl.c
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


void make_table __PROTO((short , unsigned char * , short , unsigned short * ));

