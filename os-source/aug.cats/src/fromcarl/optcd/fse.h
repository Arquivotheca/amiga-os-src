/*  :ts=8 bk=0
 *
 * fse.h:	FileSystem Event types.
 *
 * Leo L. Schwab					2128
 * Kenneth Yeast					2713
 */

/* FSE events to monitor								*/
/*				Code	   Event		Info			*/
#define	FSEB_PACKETS		0	/* All DOS Packets	Type/Arg1/Arg2/Arg3	*/
#define	FSEB_FILE		1	/* File Read		Block/Length (blocks)	*/
#define	FSEB_DIR_NOCACHE	2	/* Dir Read		Block			*/
#define	FSEB_DIR_CACHE		3	/*  " (Cached)		Block			*/
#define	FSEB_BLOCK_NOCACHE	4	/* Block Read		Block			*/
#define	FSEB_BLOCK_CACHE	5	/*  " (Cached)		Block			*/
#define	FSEB_LOW_READ		6	/* Lowest level read	Block/Length (blocks)	*/

#define	FSEF_PACKETS		( 1 << FSEB_PACKETS	  )
#define	FSEF_FILE		( 1 << FSEB_FILE	  )
#define	FSEF_DIR_NOCACHE	( 1 << FSEB_DIR_NOCACHE	  )
#define	FSEF_DIR_CACHE		( 1 << FSEB_DIR_CACHE	  )
#define	FSEF_BLOCK_NOCACHE	( 1 << FSEB_BLOCK_NOCACHE )
#define	FSEF_BLOCK_CACHE	( 1 << FSEB_BLOCK_CACHE	  )
#define	FSEF_LOW_READ		( 1 << FSEB_LOW_READ	  )
