/* cli_init.h */

/* longs! */
#define FH_STACK	210
#define CON_STACK	800
#define RAM_STACK	300
#define SER_STACK	800
#define PAR_STACK	800
#define PRT_STACK	1000

#define TASKTABLESIZE	20
#define MAXUNITS	3

#define PREFSIZE	232
#define PREFFLAG	1

#define CLI_INITIAL_FAIL_LEVEL	10
#define CLI_INITIAL_STACK	(4096 >> 2)

#define DISK_SECTOR_SIZE	512

/* defines for fixed limits on cli structure stuff.  FIX! */

#define	NAMEMAX		26	/* was 25 - REJ */
#define PROMPTMAX	15
#define SETMAX		20
#define FILEMAX		10

