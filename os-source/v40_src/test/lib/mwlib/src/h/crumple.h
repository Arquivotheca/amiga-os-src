/*****************************************************************************
    Crumple.h
    
    Mitchell/Ware Systems           Version 1.00            04-Feb-89
    
    Structs for Crumple Fractal System
    
    OPERATION: range at each bifrucation is calculated as follows:
        range / [2^(bdim-bskip)]
*****************************************************************************/

#ifndef _CRUMPLE_
#define _CRUMPLE_ 1

#define CRU_RANGE 16384 /* +/- this range. Actual may go slightly higher. */

#define CRU_X   0
#define CRU_Y   1
#define CRU_Z   2

typedef struct Crumple  {
    USHORT bdim[3];             /* power-of-2 dimensions of the array */  
    USHORT bskip[3];            /* power-of-2 skip-data storage (0 is max resolution) */  
    
    long range, r[3];               /* used in ramdom calculations */

    ULONG ar_size;                  /* actual array size in bytes */
    short *ar;                      /* fractal scalar field array */
    } Crumple;

#endif
