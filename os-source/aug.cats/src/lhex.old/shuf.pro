/* Prototypes for functions defined in
shuf.c
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


void decode_start_st0 __PROTO((void));

void encode_p_st0 __PROTO((unsigned short ));

extern int fixed[2][16];

void encode_start_fix __PROTO((void));

void decode_start_fix __PROTO((void));

unsigned short decode_c_st0 __PROTO((void));

unsigned short decode_p_st0 __PROTO((void));

