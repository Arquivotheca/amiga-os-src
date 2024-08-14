/* Prototypes for functions defined in
dynlists.c
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


char ** Init_List __PROTO((char ** ));

void Free_List __PROTO((char ** ));

int Length_List __PROTO((char ** ));

char ** Add_List __PROTO((char ** , char * ));

