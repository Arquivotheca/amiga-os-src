/*
 * $Id: vf.h,v 1.4 91/12/11 09:37:11 mks Exp $
 *
 * $Log:	vf.h,v $
 * Revision 1.4  91/12/11  09:37:11  mks
 * Added definition of where the VF structure goes in F-space
 * 
 * Revision 1.3  91/12/06  07:18:00  mks
 * Now the Memory entry is just a pointer so the F00000 space
 * can also be used if MakeVF is used to set it up.
 *
 * Revision 1.2  91/10/18  12:26:51  mks
 * Moved the ROM_LOC_#? to the CheckSumVF module.
 *
 * Revision 1.1  91/10/10  19:55:32  mks
 * Added the global hunk counter for symbol map file generation.
 *
 * Revision 1.0  91/09/16  14:57:22  mks
 * Initial revision
 *
 */

/******************************************************************************/

#include	<exec/semaphores.h>

/******************************************************************************/

struct	VF_Sem
{
struct	SignalSemaphore	Sem;
	char		SemName[10];  /* Also make the rest long-word aligned */
	ULONG		LastPos;	/* Next free word (In array below) */
	ULONG		LastHunk;	/* Global hunk counter */
	ULONG		*Memory;	/* Pointer to the memory */
	ULONG		Memory_Start;	/* Start point for VF memory */
};

#define	FULL_SIZE	(512*1024/4)

#define	VF_SIZE	(sizeof(struct VF_Sem)+((FULL_SIZE*4)))

#define	VF_NAME	"Vertual-F"

#define	VF_FPOS	((struct VF_Sem *)(0x00F80000 - sizeof(struct VF_Sem)))
