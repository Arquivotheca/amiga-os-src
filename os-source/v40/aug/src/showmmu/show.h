/* ====================================================================== */

/* Define all bit components used for manipulation of the Cache Control
   Register. */

#define CACR_INST	(1L<<0)
#define CACR_DATA	(1L<<8)

#define CACR_WALLOC	5
#define CACR_BURST	4
#define CACR_CLEAR	3
#define CACR_ENTRY	2
#define CACR_FREEZE	1
#define CACR_ENABLE	0

/* ====================================================================== */

/* Define important bits used in various MMU registers. */

/* Here are the CRP definitions.  The CRP register is 64 bits long, but
   only the first 32 bits are control bits, the next 32 bits provide the
   base address of the table. */

#define	CRP_UPPER	(1L<<31)		/* Upper/lower limit mode */
#define CRP_LIMIT(x)	((ULONG)((x)&0x7fff)<<16)/* Upper/lower limit value */
#define CRP_SG		(1L<<9)			/* Indicates shared space */
#define CRP_DT_INVALID	0x00			/* Invalid root descriptor */
#define	CRP_DT_PAGE	0x01			/* Fixed offset, auto-genned */
#define CRP_DT_V4BYTE	0x02			/* Short root descriptor */
#define	CRP_DT_V8BYTE	0x03			/* Long root descriptor */

/* Here are the TC definitions.  The TC register is 32 bits long. */

#define	TC_ENB		(1L<<31)		/* Enable the MMU */
#define	TC_SRE		(1L<<25)		/* For separate Supervisor */
#define	TC_FCL		(1L<<24)		/* Use function codes? */
#define	TC_PS(x)	((ULONG)((x)&0x0f)<<20)	/* Page size */
#define TC_IS(x)	((ULONG)((x)&0x0f)<<16)	/* Logical shift */
#define	TC_TIA(x)	((ULONG)((x)&0x0f)<<12)	/* Table indices */
#define	TC_TIB(x)	((ULONG)((x)&0x0f)<<8)
#define TC_TIC(x)	((ULONG)((x)&0x0f)<<4)
#define	TC_TID(x)	((ULONG)((x)&0x0f)<<0)

/* Here are the page descriptor definitions, for short desctriptors only,
   since that's all I'm using at this point. */
   
#define	PD_ADDR(x)	((ULONG)(x)&~0x0f)	/* Translated Address */
#define	PD_WP		(1L<<2)			/* Write protect it! */
#define PD_DT_INVALID	0x00			/* Invalid root descriptor */
#define	PD_DT_PAGE	0x01			/* Fixed offset, auto-genned */
#define PD_DT_V4BYTE	0x02			/* Short root descriptor */
#define	PD_DT_V8BYTE	0x03			/* Long root descriptor */

/* ====================================================================== */
