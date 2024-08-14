/*
 * $Id: sym.h,v 1.1 91/10/17 15:45:45 mks Exp $
 *
 * $Log:	sym.h,v $
 * Revision 1.1  91/10/17  15:45:45  mks
 * Initial revision
 * 
 */

/******************************************************************************/

#include	<exec/semaphores.h>

/******************************************************************************/

struct	Sym_Sem
{
struct	SignalSemaphore	Sem;
	char		SemName[14]; /* Also make the rest long-word aligned */
	ULONG		*Memory;	/* We use this as the start of data */
	char		Symbols[256];	/* Name of the symbols */
};

#define	SYM_NAME	"Build Symbols"
