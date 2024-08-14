/*** hardware.c *************************************************************
 *
 *  hardware.c -- hardware dependent initialization
 *
 *  $Id: hardware.c,v 36.4 90/11/05 19:03:06 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include "ubase.h"
#include <exec/execbase.h>
/* get prototype for SetFunction() */
#include <clib/exec_protos.h>
#include <pragmas/exec_old_pragmas.h>

/* had a little problem with these in the code space */
extern void	Amiga2DateH();
extern void	Date2AmigaH();
extern void	CheckDateH();
extern void	SMult32H();
extern void	UMult32H();
extern void	SDivMod32H();
extern void	UDivMod32H();

/*** CHEATING: grabbed constants for LVO out of utility.p ***/
#define LVOAmiga2Date (-0x78)
#define LVODate2Amiga (-0x7e)
#define LVOCheckDate (-0x84)
#define LVOSMult32 (-0x8a)
#define LVOUMult32 (-0x90)
#define LVOSDivMod32 (-0x96)
#define LVOUDivMod32 (-0x9c)

struct funcy_one {
	LONG	funcOffset;
	void	(*funcEntry)();
};

struct funcy_one override[] =  {
    { LVOAmiga2Date, Amiga2DateH },
    { LVODate2Amiga, Date2AmigaH },
    { LVOCheckDate, CheckDateH },
    { LVOSMult32, SMult32H },
    { LVOUMult32, UMult32H },
    { LVOSDivMod32, SDivMod32H },
    { LVOUDivMod32, UDivMod32H },
};

#define FANCY_HARDWARE (AFF_68020 | AFF_68030 | AFF_68040)

/*
 * override default vectors with hardware dependent ones,
 * if the necessary fancy hardware is available
 */
HardwareOverride( struct UtilityBase *ulib )
{
    int	fx;
    struct funcy_one	*fo = override;

    if ( ( (struct ExecBase *)ulib->ub_SysLib)->AttnFlags & FANCY_HARDWARE )
    {
	for ( fx = 0; fx < (sizeof override)/(sizeof override[0]); fx++ )
	{
	    SetFunction( (struct Library *) ulib,
	    	fo->funcOffset, (void (*)()) fo->funcEntry );

	    fo++;
	}
    }
}
