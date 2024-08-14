/*
 * Common structure shared between sliplogin and internet slip driver.  This
 * is used to pass configuration information into inet.library
 */

#define DEVNMSIZE	64
struct slipvars {
	char	sl_devname[DEVNMSIZE];	/* name to OpenDevice() on	*/
	long	sl_unit;		/* unit number for OpenDevice	*/
	long	sl_flags;		/* flags for OpenDevice		*/
	long	sl_baud;		/* baudrate to run at		*/
};

