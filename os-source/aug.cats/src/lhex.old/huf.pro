/* Prototypes for functions defined in
huf.c
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


extern unsigned short left[1019];

extern unsigned short right[1019];

extern unsigned char c_len[510];

extern unsigned char pt_len[128];

extern unsigned short c_freq[1019];

extern unsigned short c_table[4096];

extern unsigned short c_code[510];

extern unsigned short p_freq[27];

extern unsigned short pt_table[256];

extern unsigned short pt_code[128];

extern unsigned short t_freq[37];

unsigned short decode_c_st1 __PROTO((void));

unsigned short decode_p_st1 __PROTO((void));

void decode_start_st1 __PROTO((void));

