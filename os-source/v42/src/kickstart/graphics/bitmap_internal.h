#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

/* this file defines the 8 flag bits in the bm_Flags field of a bitmap
   which has it's bm_Pad==UNLIKELY_WORD.

   These bit definitions do not correspond to the BMF_FLAGS because of the
overloading of flags which is done. These 8 bits are precious, so we can't
waste a single one!

*/

#define IBMB_INTERLEAVED 0
#define IBMF_INTERLEAVED (1l<<IBMB_INTERLEAVED)

#define IBMB_FAST	1
#define IBMF_FAST	(1l<<IBMB_FAST)

#define IBMB_CHUNKY	2
#define IBMF_CHUNKY	(1l<<IBMB_CHUNKY)

#define ALWAYS_USE_CPU 1 /* if set, then always use cpu routines instead of blitter */

