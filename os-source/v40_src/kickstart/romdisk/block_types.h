/* block_types.h */

#define ST_ROOT		1
#define ST_USERDIR	2
#define ST_SOFTLINK	3	/* looks like dir, but is stored like file */
#define ST_LINKDIR	4
#define ST_FILE		-3	/* must be negative for FIB! */
#define ST_LINKFILE	-4
