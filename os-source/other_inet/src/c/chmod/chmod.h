/* -----------------------------------------------------------------------
 * chmod.h
 *
 * $Locker:  $
 *
 * $Id: chmod.h,v 1.1 92/11/24 15:57:53 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Log:	chmod.h,v $
 * Revision 1.1  92/11/24  15:57:53  bj
 * Initial revision
 * 
 *
 * $Header: Hog:Other/inet/src/c/chmod/RCS/chmod.h,v 1.1 92/11/24 15:57:53 bj Exp $
 *
 *------------------------------------------------------------------------
 */

#ifndef CHMOD_H
#define CHMOD_H

#ifdef DEBUG
#define DB(x) PutStr(x)
#define DBp(x,y) printf((x),(y))
#define DBp2(x,y,z) printf((x),(y),(z))
#else
#define DB(x) ;
#define DBp(x,y) ;
#define DBp2(x,y,z) ;
#endif

struct glob {
	struct DosLibrary *g_DOSBase ;
	BOOL g_verbose ;
	char *g_permissions ;
	char **g_files ;
	BOOL g_mask_is_numeric ;
	int  g_indent ;
#ifdef DODIRS	
	BOOL g_all ;
#endif	
	} ;

#ifdef DODIRS
 #define TEMPLATE "PERMISSIONS/A,FILES/A/M,-A=ALL/S,-V=VERBOSE/S"
 #define OPT_P     0
 #define OPT_F     1
 #define OPT_A     2
 #define OPT_V     3
 #define OPT_COUNT 4
#else
 #define TEMPLATE "PERMISSIONS/A,FILES/A/M,-V=VERBOSE/S"
 #define OPT_P     0
 #define OPT_F     1
 #define OPT_V     2
 #define OPT_COUNT 3
#endif


#ifdef DODIRS
 #define T2 "FILES/M/A,-A=ALL/S,-V=VERBOSE/S"
 #define OPT_F2     0
 #define OPT_COUNT2 1
#else
 #define T2 "FILES/M/A,-V=VERBOSE/S"
 #define OPT_F2     0
 #define OPT_COUNT2 1
#endif



#endif /* CHMOD_H */

