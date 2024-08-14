#define	PD_ADDR(x)	((ULONG)(x)&~0x0f)	/* Translated Address */
#define	PD_WP		(1L<<2)			/* Write protect it! */
#define PD_DT_INVALID	0x00			/* Invalid root descriptor */
#define	PD_DT_PAGE	0x01			/* Fixed offset, auto-genned */
#define PD_DT_V4BYTE	0x02			/* Short root descriptor */
#define	PD_DT_V8BYTE	0x03			/* Long root descriptor */

   for (i = 0; i < 126; i++) MMUTable[i] = PD_ADDR(i<<17)|PD_DT_PAGE;
   MMUTable[126] = PD_ADDR(ROM32)|PD_WP|PD_DT_PAGE;
   MMUTable[127] = PD_ADDR(((ULONG)ROM32)+0x20000L)|PD_WP|PD_DT_PAGE;


Instead, build the translation table like this:

#define PD_CI 

   for (i=0;  i<16;  i++) MMUTable[i] = PD_ADDR(i<<17> | PD_CI | PD_DT_PAGE;
   for (i=16; i<84;  i++) MMUTable[i] = PD_ADDR(i<<17) | PD_DT_PAGE;
   for (i=84; i<120; i++) MMUTable[i] = PD_ADDR(i<<17) | PD_CI | PD_DT_PAGE;

#ifdef FASTROM	/* if you still want the FASTROM feature */
   for (i=120; i<126;  i++) MMUTable[i] = PD_ADDR(i<<17) | PD_DT_PAGE;
   MMUTable[126] = PD_ADDR(ROM32)|PD_WP|PD_DT_PAGE;
   MMUTable[127] = PD_ADDR(((ULONG)ROM32)+0x20000L)|PD_WP|PD_DT_PAGE;
#else		/* If you prefer the Frances remap hack */
   for (i=16; i<126;  i++) MMUTable[i] = PD_ADDR(i<<17) | PD_DT_PAGE;
#endif

