/*
 * free mapentry bitmap
 */

extern char *freemap;

#define SHIFT 	3
#define MASK	((1<<SHIFT)-1)

#define	ISFREE(map)	(freemap[(map)>>SHIFT] & (1 << ((map) & MASK)))
#define MAKEFREE(map)	freemap[(map)>>SHIFT] |= (1 << ((map) & MASK))
#define MAKEUSED(map)	freemap[(map)>>SHIFT] &= ~(1 << ((map) & MASK))

nfsstat init_bitmap();
