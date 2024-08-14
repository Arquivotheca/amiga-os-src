/* -----------------------------------------------------------------------
 * minrexx.h		handshake_src
 *
 * $Locker:  $
 *
 * $Id: minrexx.h,v 1.1 91/05/09 14:34:52 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/h/RCS/minrexx.h,v 1.1 91/05/09 14:34:52 bj Exp $
 *
 * $Log:	minrexx.h,v $
 * Revision 1.1  91/05/09  14:34:52  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

/*
 *   Includes for minrexx.c; please refer to that file for
 *   further documentation.
 */
#include <rexx/rxslib.h>
/*
 *   Maximum messages that can be pending, and the return codes
 *   for two bad situations.
 */
#define MAXRXOUTSTANDING (300)
#define RXERRORIMGONE (100)
#define RXERRORNOCMD (30)
#define RXERRORNOTMINE (1)
/*
 *   This is the association list you build up (statically or
 *   dynamically) that should be terminated with an entry with
 *   NULL for the name . . .
 */
struct rexxCommandList {
   char *name ;
   int  userdata ;
} ;
