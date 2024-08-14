/******************************************************************************
*
*	$Id: tune.h,v 39.0 91/08/21 17:22:43 chrisg Exp $
*
******************************************************************************/


/* tunable parameters for amiga rom library */

/*	if dspins overflow in copper list MakeView, get this many more */
/*  instructions for next block of copper instructions */
/*	20 is enough for a standard 5 bit plane image */

#define	DSPINS	40
#define	COPINSINC	16
/* the folllowing are just guesses, RJ needs to come with good #s here */
#define CLRINS	32
#define SPRINS	32

/* if no user copper list is allocated this is its default start size */
#define UCLSIZE		16

