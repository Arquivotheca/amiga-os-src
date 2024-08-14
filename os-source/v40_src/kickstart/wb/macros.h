/*
 * $Id: macros.h,v 38.1 91/06/24 11:37:00 mks Exp $
 *
 * $Log:	macros.h,v $
 * Revision 38.1  91/06/24  11:37:00  mks
 * Initial V38 tree checkin - Log file stripped
 * 
 */

#ifdef undef

#define MAX(a,b)    ((a)>(b)?(a):(b))
#define MIN(a,b)    ((a)<(b)?(a):(b))
#define ABS(x)      ((x<0)?(-(x)):(x))

#else

#include <string.h>

#define min __builtin_min
#define max __builtin_max
#define abs __builtin_abs
extern int min(int, int);
extern int max(int, int);
extern int abs(int);

#endif undef
