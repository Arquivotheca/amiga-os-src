/* --------------------------------------------------------
 * func2.c    test module for including C functions in shared libs
 *
 * C routine installed in library
 *
 * Since when making a call to a library routine in the assembler
 * interface data are put into registers, then we must recover the
 * data and put them into local variables in any C routine.
 *
 * cc +b _func2
 *
 * no startup code needed
 * ---------------------------------------------------------------
 */

#include <exec/types.h>
#include "sslib.h"

extern struct SSLib *sSBase ;

/* -------------------------------------------------------------
 * assembler call
 * ------------------------------------------------------------
 */

ULONG Func2( ULONG val ) ;

#pragma regcall(Func2(A0))

ULONG Func2( REGISTER ULONG val )
{
	ULONG v2 ;

/*	return( val + val ) ; */
return( (ULONG)sSBase ) ;
	

}  /* end Func2 */
                     