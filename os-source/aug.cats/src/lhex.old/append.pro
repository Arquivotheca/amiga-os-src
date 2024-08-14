/* Prototypes for functions defined in
append.c
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


extern long indicator_count;

extern long indicator_threshold;

extern struct interfacing interface;

void start_indicator __PROTO((char * , long , char const * , long , BOOL ));

void finish_indicator2 __PROTO((char * , char const * , int ));

void finish_indicator __PROTO((char * , char const * ));

