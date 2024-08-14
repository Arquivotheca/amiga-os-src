/*
 * mydebug.h - #include this file sometime after stdio.h
 * Set MYDEBUG to 1 to turn on debugging, 0 to turn off debugging
 */
#ifndef MYDEBUG_H
#define MYDEBUG_H

#define MYDEBUG	 1
 
#if MYDEBUG
/*
 * MYDEBUG User Options
 */

/* Set to 1 to turn second level D2(bug()) statements */
#define DEBUGLEVEL2	1

/* Set to a non-zero # of ticks if a delay is wanted after each debug message */
#define DEBTIME		0

/* Set to 1 for serial debugging (link with debug.lib) */
#define KDEBUG		0

/* Set to 1 for parallel debugging (link with ddebug.lib) */
#define DDEBUG		0

#endif /* MYDEBUG */


/* Prototypes for Delay, kprintf, dprintf. Or use proto/dos.h or functions.h. */
#include <clib/dos_protos.h>
void kprintf(UBYTE *fmt,...);
void dprintf(UBYTE *fmt,...);

/*
 * D(bug()), D2(bug()), DQ((bug()) only generate code if MYDEBUG is non-zero
 *
 * Use D(bug()) for general debugging, D2(bug()) for extra debugging that you
 * usually won't need to see, DQ(bug()) for debugging that you'll never want
 * a delay after (ie. debugging inside a Forbid, Disable, Task, or Interrupt)
 *
 * Some example uses:
 * D(bug("about to do xyz. variable = $%lx\n",myvariable)); 
 * D2(bug("v1=$%lx v2=$%lx v3=$%lx\n",v1,v2,v3)); 
 * DQ(bug("in subtask: variable = $%lx\n",myvariable));
 *
 * Set MYDEBUG above to 1 when debugging is desired and recompile the modules
 *  you wish to debug.  Set to 0 and recompile to turn off debugging.
 *
 * User options set above:
 * Set DEBUGDELAY to a non-zero # of ticks (ex. 50) when a delay is desired.
 * Set DEBUGLEVEL2 nonzero to turn on second level (D2) debugging statements
 * Set KDEBUG to 1 and link with debug.lib for serial debugging.
 * Set DDEBUG to 1 and link with ddebug.lib for parallel debugging.
 */



/* 
 * Debugging function automaticaly set to printf, kprintf, or dprintf
 */

#if KDEBUG
#define bug kprintf
#elif DDEBUG
#define bug dprintf
#else	/* else changes all bug's to printf's */
#define bug printf
#endif


/*
 * Debugging macros
 */

/* DQ (debug quick) never uses Delay.  Use in forbids,disables,tasks,interrupts */
#if MYDEBUG
#define D(x) (x); if(DEBTIME>0) Delay(DEBTIME)
#define DQ(x) (x)
#if DEBUGLEVEL2
#define D2(x) (x); if(DEBTIME>0) Delay(DEBTIME)
#else
#define D2(x) ;
#endif /* DEBUGLEVEL2 */
#else
#define D(x) ;
#define DQ(x) ;
#define D2(x) ;
#endif


#endif /* MYDEBUG_H */

