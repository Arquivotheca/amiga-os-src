/* Prototypes for functions defined in
extract.c
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


int decode_lzhuf __PROTO((BPTR , BPTR , long , long , char * , int ));

