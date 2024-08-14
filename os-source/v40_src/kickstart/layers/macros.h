/******************************************************************************
*
*	$Id: macros.h,v 38.2 91/08/02 10:41:37 mks Exp $
*
******************************************************************************/

#define MAX(a,b)        ((a)>(b)?(a):(b))
#define MIN(a,b)        ((a)<(b)?(a):(b))
#define ABS(x)  ((x<0)?(-(x)):(x))

#define SHORTMIN(a,b)	(SHORT)(MIN((SHORT)(a),(SHORT)(b)))
#define SHORTMAX(a,b)	(SHORT)(MAX((SHORT)(a),(SHORT)(b)))
