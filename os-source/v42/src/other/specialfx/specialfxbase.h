#ifndef SPECIALFX_SPECIALFXBASE_H
#define SPECIALFX_SPECIALFXBASE_H
/*
**	$Id: specialfxbase.h,v 39.9 93/09/20 14:53:02 spence Exp $
**
**	copperlist animation control
**
**	(C) Copyright 1993 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#define MAX_VPLIST	2
#define MAX_AHLIST	20

/* VPList is a MinList of the known ViewPorts using specialfx.library.
 * If there are more than MAX_VPLIST known ViewPorts, then another list is
 * allocated and linked up to the rest.
 *
 * This is used only by FindVP() in the InterruptControl effect.
 */
struct VPList
{
    struct MinNode	vpl_MinNode;
    UWORD		vpl_Count;	/* number of ViewPorts in the array */
    UWORD		vpl_Pad;
    ULONG		vpl_ViewPorts[MAX_VPLIST];
};

/* AHList is a MinList of all the known AnimHandles using specialfx.library.
 * If there are more than MAX_AHLIST known AnimHandles, then another list is
 * allocated and linked up to the rest.
 *
 * This is used by the MrgCop() and LoadView() patches when seeing if the View
 * passed to MrgCop() or LoadView() is used by specialfx.library.
 */
struct AHList
{
    struct MinNode	ahl_MinNode;
    UWORD		ahl_Count;	/* number of AnimHandles in the array */
    UWORD		ahl_Pad;
    ULONG		ahl_AnimHandles[MAX_AHLIST];
};

struct SpecialFXBase
{
    struct Library	sfxb_LibNode;
    UWORD		sfxb_Flags;
    struct ExecBase	*sfxb_ExecBase;
    struct GfxBase	*sfxb_GfxBase;
    struct UtilityBase	*sfxb_UtilityBase;
    APTR		sfxb_SegList;
    APTR		sfxb_VPListSemaphore;	/* Semaphore to lock sfxb_VPList */
    struct MinList	sfxb_VPListHead;
    struct VPList	sfxb_VPList;
    APTR		sfxb_AHListSemaphore;	/* Semaphore to lock sfxb_AHList */
    struct MinList	sfxb_AHListHead;
    struct AHList	sfxb_AHList;
    APTR		sfxb_AllocVectors;	/* Pointer the the Alloc.. routines */
    APTR		sfxb_CountVectors;	/* Pointer the the Count.. routines */
    APTR		sfxb_InstallVectors;	/* Pointer the the Install.. routines */
    APTR		sfxb_RemoveVectors;	/* Pointer the the Remove.. routines */
    APTR		sfxb_AnimateVectors;	/* Pointer the the Animate.. routines */
    APTR		sfxb_RebuildVectors;	/* Pointer the the Rebuild.. routines */
    APTR		sfxb_OldMrgCop;		/* Value returned by SetFunction() */
    APTR		sfxb_CopperLCache;	/* Temporary storage of the LOF Copper
    						 * copy in the MrgCop() patch.
    						 */
    APTR		sfxb_CopperSCache;	/* SHF equivalent */
    ULONG		sfxb_CopperLSize;	/* Size of the LOF allocation */
    ULONG		sfxb_CopperSSize;	/* Size of the SHF allocation */
    APTR		sfxb_AnimationSemaphore;
    						/* Semaphore to show a task is animating -
    						 * only one task can use specialfx
    						 * at a time.
    						 */
    APTR		sfxb_OldLoadView;	/* Value returned by SetFunction() */
};

#endif	/* SPECIALFX_SPECIALFXBASE_H */
