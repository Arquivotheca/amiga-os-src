
/* $Id: watch.h,v 1.3 91/04/24 20:55:00 peter Exp $	*/

#define	WATCH_PAUSE		1
#define	WATCH_EVALUATE		(1<<1)
#define	WATCH_SYM_ADDR		(1<<2)


#define	WWAIT	if (watch & WATCH_PAUSE) getchar()
