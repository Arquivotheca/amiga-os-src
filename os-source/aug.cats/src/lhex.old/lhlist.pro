/* Prototypes for functions defined in
lhlist.c
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


extern char * __montbl[12];

void cmd_list __PROTO((void));

