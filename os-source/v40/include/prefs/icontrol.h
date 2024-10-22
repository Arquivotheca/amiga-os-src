#ifndef PREFS_ICONTROL_H
#define PREFS_ICONTROL_H
/*
**	$Id: icontrol.h,v 39.1 92/10/01 12:14:25 vertex Exp $
**
**	File format for intuition control preferences
**
**	(C) Copyright 1991 Commodore-Amiga, Inc.
**	All Rights Reserved
*/

/*****************************************************************************/


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef LIBRARIES_IFFPARSE_H
#include <libraries/iffparse.h>
#endif


/*****************************************************************************/


#define ID_ICTL MAKE_ID('I','C','T','L')


struct IControlPrefs
{
    LONG  ic_Reserved[4];	/* System reserved		*/
    UWORD ic_TimeOut;		/* Verify timeout		*/
    WORD  ic_MetaDrag;		/* Meta drag mouse event	*/
    ULONG ic_Flags;		/* IControl flags (see below)	*/
    UBYTE ic_WBtoFront;		/* CKey: WB to front		*/
    UBYTE ic_FrontToBack;	/* CKey: front screen to back	*/
    UBYTE ic_ReqTrue;		/* CKey: Requester TRUE		*/
    UBYTE ic_ReqFalse;		/* CKey: Requester FALSE	*/
};

/* flags for IControlPrefs.ic_Flags */
#define ICB_COERCE_COLORS 0
#define ICB_COERCE_LACE   1
#define ICB_STRGAD_FILTER 2
#define ICB_MENUSNAP	  3
#define ICB_MODEPROMOTE   4

#define ICF_COERCE_COLORS (1<<0)
#define ICF_COERCE_LACE   (1<<1)
#define ICF_STRGAD_FILTER (1<<2)
#define ICF_MENUSNAP	  (1<<3)
#define ICF_MODEPROMOTE   (1<<4)


/*****************************************************************************/


#endif /* PREFS_ICONTROL_H */
