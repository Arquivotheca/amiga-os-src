/* Prototypes for functions defined in
patmatch.c
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


int patmatch __PROTO((char * , char * , int ));

