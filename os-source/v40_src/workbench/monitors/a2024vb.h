/******************************************************************************
*
*	$Id: a2024vb.h,v 40.0 93/02/09 18:27:18 spence Exp $
*
******************************************************************************/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

struct A2024VB
{
    APTR gbhc;	/* pointer to GfxBase->hedley_count */
    APTR poke;	/* pointer to 1st address in the copperlist to poke */
    APTR tobplptrs;	/* address of bplptrs instructions */
    UWORD colours[4*6];	/* 4 colours per frame, max 6 frames */
    LONG offset[4][6];	/* 4 bitplane pointers for each of 6 frames */
    UWORD maxcount;	/* last frame number * 8*/
};
