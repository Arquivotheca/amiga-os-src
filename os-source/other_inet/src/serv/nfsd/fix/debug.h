/* -----------------------------------------------------------------------
 * debug.h   NFSD
 *
 * $Locker:$
 *
 * $Id:$
 *
 * $Revision:$
 *
 * $Log:$
 *
 * $Header:$
 *
 *------------------------------------------------------------------------
 */

#ifdef DEBUG
	#define DB(x) debug(x)
	#define DBS(x,y) debug_s(x,y)
	#define DBSS(x,y,z) debug_ss(x,y,z)
	#define WR(x,y,z) Write((x),(y),(z))
#else
	#define DB(x) ;
	#define DBS(x,y) ;
	#define DBSS(x,y,z) ;
	#define WR(x,y,z) ;
#endif
