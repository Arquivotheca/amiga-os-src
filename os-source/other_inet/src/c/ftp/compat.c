/* ---------------------------------------------------------------------------------
 * compat.c  (Manx 3.6)
 *
 * $Locker:  $
 *
 * $Id: compat.c,v 1.1 90/11/06 18:06:22 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/ftp/RCS/compat.c,v 1.1 90/11/06 18:06:22 bj Exp $
 *
 *-----------------------------------------------------------------------------------
 */




/*
** compatibility routines for ftp client.  These generally do things
** that apply only to Unix or just don't matter on the Amiga.
*/

chmod()
{
	return 0;
}

