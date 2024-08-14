/* Prototypes for functions defined in
larc.c
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


unsigned short decode_c_lzs __PROTO((void));

unsigned short decode_p_lzs __PROTO((void));

void decode_start_lzs __PROTO((void));

unsigned short decode_c_lz5 __PROTO((void));

unsigned short decode_p_lz5 __PROTO((void));

void decode_start_lz5 __PROTO((void));

