/* Prototypes for functions defined in
dhuf.c
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


extern unsigned int n_max;

void start_c_dyn __PROTO((void));

void decode_start_dyn __PROTO((void));

unsigned short decode_c_dyn __PROTO((void));

unsigned short decode_p_dyn __PROTO((void));

void output_dyn __PROTO((int , unsigned int ));

void encode_end_dyn __PROTO((void));

