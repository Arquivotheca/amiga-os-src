/* -----------------------------------------------------------------------
 * filereq.c		handshake_src
 *
 * $Locker:  $
 *
 * $Id: filereq.c,v 1.2 91/05/09 15:18:58 bj Exp $
 *
 * $Revision: 1.2 $
 *
 * $Header: HOG:Other/inet/src/c/hs/RCS/filereq.c,v 1.2 91/05/09 15:18:58 bj Exp $
 *
 * $Log:	filereq.c,v $
 * Revision 1.2  91/05/09  15:18:58  bj
 * Removed all use of ARP library file requester. For now,
 * you have to use a simple string gadget requster to enter
 * filenames.  ASL is on the way.
 * 
 *
 *------------------------------------------------------------------------
 */


#include <exec/types.h>
#include "termall.h"

static BYTE Pattern[32] ;

USHORT 
FileReq ( UBYTE *title, UBYTE *fname )
{
	return ( (USHORT) GetStringFromUser ( title, fname, FNAMESIZE)) ;
}
    
/* ===================================================
 * SetPattern()
 * ==================================================
 */


void 
SetPattern ( BYTE *pattern )
{
	strcpy ( Pattern, pattern );
}
