/*
 ******************************************************************************
 *
 * 68040 MMU Support Code
 *
 * $Id: mmu.c,v 1.4 93/01/18 09:53:40 mks Exp $
 *
 * $Log:	mmu.c,v $
 * Revision 1.4  93/01/18  09:53:40  mks
 * Added code needed to track the nest counts of the COPYBACK cache pages
 * 
 * Revision 1.3  92/11/30  13:27:29  mks
 * Added special case for small size...
 *
 * Revision 1.2  92/08/26  09:24:28  mks
 * Minor cleanup...
 *
 * Revision 1.1  92/08/25  19:28:37  mks
 * Initial revision
 *
 ******************************************************************************
 *
 * This code generates the "harmless" MMU tables for the 68040
 * such that Zorro-III I/O cards work better and that physical
 * bus errors do not happen.
 */

#include	<exec/types.h>
#include	<exec/execbase.h>
#include	<exec/tasks.h>
#include	<exec/memory.h>
#include	<exec/alerts.h>
#include        <exec/ports.h>
#include        <exec/libraries.h>
#include	<exec/semaphores.h>
#include	<libraries/configvars.h>

#include	<clib/exec_protos.h>
#include	<pragmas/exec_pragmas.h>

#include	<clib/expansion_protos.h>
#include	<pragmas/expansion_pragmas.h>

#define EXECBASE (*(struct ExecBase **)4)
#define	SysBase	EXECBASE

/*
 * The MMU Frame we will use...
 */
struct MMU_Frame
{
	ULONG	mmu_CRP[2];	/* For the 040 it is the root pointer */
	ULONG	mmu_TC;		/* TC for 040.  Accessed as a long, but only part of it is used on 040 */

	ULONG	*mmu_LevelA;	/* This is the base table.  The root pointer will contain this one... */
	ULONG	*mmu_LevelB;	/* Note that on the 68040, these are just the invalid pages and the full pages are elsewhere */
	ULONG	*mmu_LevelC;	/* " */

	ULONG	mmu_Indirect;	/* The indirect MMU error descriptor ;^)   For the 68040 slimmy trick... */
	ULONG	*mmu_Magic;	/* Special memory area for magic code (such as the 68040 bus error remapping magic) */

	ULONG	mmu_InvalidA;	/* The invalid value at LevelA  (68040 only) */
	ULONG	mmu_InvalidB;	/* The invalid value at LevelB  (68040 only) */
	ULONG	mmu_InvalidC;	/* The invalid value at LevelC  (68040 only) */

struct	MinList	mmu_NestCounts;	/* Nest counts for the CachePreDMA/CachePostDMA calls */
};

/*
 * This is the structure that will contain 1 WORD of nest count for each
 * page in the array...
 */
struct	NestCount
{
struct	MinNode	nc_Node;
	ULONG	nc_Low;
	ULONG	nc_High;
	/* UWORD nc_Count[x] where X is the number of pages */
};

/*
 * General flags for setting up the MMU
 */
#define	VALID		(0x01)
#define	NONCACHEABLE	(0x40)
#define	CACHEABLE	(0x00)
#define	COPYBACK	(0x20)

/*****************************************************************************/

#define PAGE_SIZE_040		(0x1000)
#define	ALLOC_GRAN_040		(0x01FF)
#define	LEVELA_SIZE_040		(128)
#define	LEVELB_SIZE_040		(128)
#define	LEVELC_SIZE_040		(64)
#define	LEVELA_ALLOC_040	(4*LEVELA_SIZE_040)
#define	LEVELB_ALLOC_040	(4*LEVELB_SIZE_040)
#define	LEVELC_ALLOC_040	(4*LEVELC_SIZE_040)

#define	TABLE_MASK	(0xFFFFFFF3)

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/*
 * Clean up the MMU Frame as needed...
 */
void __asm Free_MMU_Frame(register __a0 struct MMU_Frame *mmu)
{
ULONG	i;
ULONG	j;
ULONG	*temp1;

	if (mmu)
	{
		/* 68040 MMU Frame Cleanup */
		if ((mmu->mmu_LevelA) && (mmu->mmu_LevelB) && (mmu->mmu_LevelC) && (mmu->mmu_Magic))
		{
			for (i=0;i<LEVELA_SIZE_040;i++) if ((mmu->mmu_LevelA[i]&TABLE_MASK) != mmu->mmu_InvalidA)
			{
				temp1=(ULONG *)(mmu->mmu_LevelA[i] & ~ALLOC_GRAN_040);
				for (j=0;j<LEVELB_SIZE_040;j++) if ((temp1[j]&TABLE_MASK) != mmu->mmu_InvalidB)
				{
					FreeMem((ULONG *)(temp1[j] & ~ALLOC_GRAN_040),LEVELC_ALLOC_040);
				}
				FreeMem(temp1,LEVELB_ALLOC_040);
			}
		}
		if (mmu->mmu_LevelA) FreeMem(mmu->mmu_LevelA,LEVELA_ALLOC_040);
		if (mmu->mmu_LevelB) FreeMem(mmu->mmu_LevelB,LEVELB_ALLOC_040);
		if (mmu->mmu_LevelC) FreeMem(mmu->mmu_LevelC,LEVELC_ALLOC_040);
		if (mmu->mmu_Magic)  FreeMem(mmu->mmu_Magic,PAGE_SIZE_040);

		while (temp1=(ULONG *)RemHead((struct List *)&(mmu->mmu_NestCounts))) FreeVec(temp1);

		/* Free the MMU Frame */
		FreeVec(mmu);
	}
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

static void *AllocAligned(ULONG size,ULONG chunk)
{
void	*data;

	if (data=AllocMem(size+chunk,MEMF_PUBLIC))
	{
		Forbid();
		FreeMem(data,size+chunk);
		data=AllocAbs(size,(APTR)((((ULONG)data)+chunk)&(~chunk)));
		Permit();
	}
	return(data);
}

static struct MMU_Frame *Mark_Address(struct MMU_Frame *mmu,ULONG addr,ULONG size,ULONG orBits)
{
BOOL	rc=FALSE;	/* Assume failure */
ULONG	i;
ULONG	temp1;
ULONG	temp2;
ULONG	indexa;
ULONG	indexb;
ULONG	indexc;
ULONG	*levelb;
ULONG	*levelc;

	orBits |= VALID;

	/*kprintf("Mark $%08lx size $%08lx as $%02lx\n",addr,size,orBits);*/

	if (mmu)
	{
		temp1=addr >> 12;	/* Remove page offset... */
		temp2=(addr+size-1) >> 12;

		if (temp1>temp2) rc=TRUE;	/* Special case small size */

		while (temp1 <= temp2)
		{
			rc=FALSE;			/* Assume we failed first... */
			indexa=temp1 >> 13;		/* Top 7 bits */
			indexb=(temp1 >> 6) & 0x7F;	/* Middle 7 bits */
			indexc=temp1 & 0x3F;		/* Bottom 6 bits */

			if (mmu->mmu_LevelA[indexa] == mmu->mmu_InvalidA)
			{
				/* We need a new LevelB here... */
				if (levelb=AllocAligned(LEVELB_ALLOC_040,ALLOC_GRAN_040))
				{
					mmu->mmu_LevelA[indexa]=(((ULONG)levelb) | 0x03);	/* Link it in */
					for (i=0;i<LEVELB_SIZE_040;i++) levelb[i]=mmu->mmu_InvalidB;
				}
			}

			if (mmu->mmu_LevelA[indexa] != mmu->mmu_InvalidA)
			{
				levelb=(ULONG *)(mmu->mmu_LevelA[indexa] & ~ALLOC_GRAN_040);	/* Get table pointer */

				if (levelb[indexb] == mmu->mmu_InvalidB)
				{
					/* We need a new LevelC here... */
					if (levelc=AllocAligned(LEVELC_ALLOC_040,ALLOC_GRAN_040))
					{
						levelb[indexb]=((ULONG)levelc) | 0x03;	/* Link it in */
						for (i=0;i<LEVELC_SIZE_040;i++) levelc[i]=mmu->mmu_InvalidC;
					}
				}

				if (levelb[indexb] != mmu->mmu_InvalidB)
				{
					levelc=(ULONG *)(levelb[indexb] & ~ALLOC_GRAN_040);	/* Get table pointer */

					if (levelc[indexc] == mmu->mmu_InvalidC) levelc[indexc]=(temp1 << 12);
					levelc[indexc] |= orBits;

					/* Special case:  Check if we marked both cache and non-cache */
					/* non-cache wins every time, so force it to win! */
					if ((levelc[indexc] & 0x60) == 0x60) levelc[indexc]-=0x20;

					rc=TRUE;	/* Operation worked... */
				}
			}
			temp1++;
		}
	}

	if (!rc)
	{
		Free_MMU_Frame(mmu);
		mmu=NULL;
	}

	return(mmu);
}

/*
 * This routine will add a nest-count array to the memory range given.
 * The range will be masked to page sizes...
 */
static struct MMU_Frame *Add_NestCount(struct MMU_Frame *mmu,ULONG low,ULONG high)
{
ULONG	size;

	if (mmu)
	{
	struct	NestCount	*nc;

		low = (low & ~(PAGE_SIZE_040 - 1)) >> 12;
		high = ((high + PAGE_SIZE_040 - 2) & ~(PAGE_SIZE_040 - 1)) >> 12;

		size = (high - low + 1);

		if (nc=AllocVec(sizeof(struct NestCount)+(size << 1),MEMF_PUBLIC|MEMF_CLEAR))
		{
			nc->nc_Low=low;
			nc->nc_High=high;
			AddTail((struct List *)&(mmu->mmu_NestCounts),(struct Node *)nc);
		}
		else
		{
			Free_MMU_Frame(mmu);
			mmu=NULL;
		}
	}

	return(mmu);
}

/*
 * This routine will map the ROM addresses to the physical
 * addresses given here...
 */
struct MMU_Frame *Map_ROM(struct MMU_Frame *mmu,ULONG addr)
{
ULONG	temp1;
ULONG	*level;

	mmu=Mark_Address(mmu,0x00F80000,0x00080000,CACHEABLE);

	/*
	 * Check if we need to do a special mapping...
	 */
	if (mmu) if (0x00F80000 != addr)
	{
		/*
		 * The 68040 case is a bit harder...
		 */
		temp1=0x00F80000 >> 12;
		/* Starting at the ROM, do the full 512K space mapping... */
		while (temp1 <= (0x00FFFFFF >> 12))
		{
			level=(ULONG *)(mmu->mmu_LevelA[temp1 >> 13] & ~ALLOC_GRAN_040);	/* LevelB */
			level=(ULONG *)(level[(temp1 >> 6) & 0x7F] & ~ALLOC_GRAN_040);		/* LevelC */
			level[temp1 & 0x3F] = addr | CACHEABLE | VALID;	/* Cacheable, Resident */

			temp1++;	addr+=PAGE_SIZE_040;	/* Next page:  4K at a shot... */
		}
	}

	return(mmu);
}

static struct MMU_Frame *Init_MMU_Frame(void)
{
struct	MMU_Frame	*mmu;
	ULONG		i;
	BOOL		rc=FALSE;

	if (mmu=AllocVec(sizeof(struct MMU_Frame),MEMF_PUBLIC|MEMF_CLEAR))
	{
		/* Initialize the list structure */
		mmu->mmu_NestCounts.mlh_Head=(struct MinNode *)&(mmu->mmu_NestCounts.mlh_Tail);
		mmu->mmu_NestCounts.mlh_TailPred=(struct MinNode *)&(mmu->mmu_NestCounts.mlh_Head);

		mmu->mmu_LevelA=AllocAligned(LEVELA_ALLOC_040,ALLOC_GRAN_040);
		mmu->mmu_LevelB=AllocAligned(LEVELB_ALLOC_040,ALLOC_GRAN_040);
		mmu->mmu_LevelC=AllocAligned(LEVELC_ALLOC_040,ALLOC_GRAN_040);
		mmu->mmu_Magic=AllocAligned(PAGE_SIZE_040,PAGE_SIZE_040-1);

		if ((mmu->mmu_LevelA) && (mmu->mmu_LevelB) && (mmu->mmu_LevelC) && (mmu->mmu_Magic))
		{
			/* Now, set up the default invalid page pointers... */
			mmu->mmu_Indirect=((ULONG)(mmu->mmu_Magic)) | NONCACHEABLE | VALID;	/* Non-cacheable, Valid */
			mmu->mmu_InvalidC=((ULONG)&(mmu->mmu_Indirect)) | 0x02; /* Indirect to the invalid one... */
			mmu->mmu_InvalidB=((ULONG)(mmu->mmu_LevelC)) | 0x03; /* Resident table */
			mmu->mmu_InvalidA=((ULONG)(mmu->mmu_LevelB)) | 0x03; /* Resident table */

			for (i=0;i<LEVELC_SIZE_040;i++) mmu->mmu_LevelC[i]=mmu->mmu_InvalidC;
			for (i=0;i<LEVELB_SIZE_040;i++) mmu->mmu_LevelB[i]=mmu->mmu_InvalidB;
			for (i=0;i<LEVELA_SIZE_040;i++) mmu->mmu_LevelA[i]=mmu->mmu_InvalidA;

			mmu->mmu_TC=0x00008000;	/* Enable MMU with 4K pages */
			mmu->mmu_CRP[0]=(ULONG)mmu->mmu_LevelA;
			mmu->mmu_CRP[1]=(ULONG)mmu->mmu_LevelA;

			/* We now have a full 68040 MMU tree, everything is marked invalid... */
			rc=TRUE;
		}
	}

	if (!rc)
	{
		Free_MMU_Frame(mmu);
		mmu=NULL;
	}

	return (mmu);
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

struct MMU_Frame * __asm BuildMMU(register __d0 ULONG ROM_Addr)
{
struct	Library		*ExpansionBase;
struct	MMU_Frame	*mmu;
struct	MemHeader	*mem;


	mmu=Init_MMU_Frame();

	/*
	 * Now for the control areas...
	 */
	mmu=Mark_Address(mmu,0x00BC0000,0x00040000,NONCACHEABLE);
	mmu=Mark_Address(mmu,0x00D80000,0x00080000,NONCACHEABLE);

	/*
	 * Now for F-Space...
	 */
	mmu=Mark_Address(mmu,0x00F00000,0x00080000,CACHEABLE);

	/*
	 * Now for the ROM...
	 */
	mmu=Map_ROM(mmu,ROM_Addr);

	/*
	 * If the credit card resource, make the addresses valid...
	 */
	if (OpenResource("card.resource"))
	{
		mmu=Mark_Address(mmu,0x00600000,0x00440002,NONCACHEABLE);
	}

	/*
	 * If CDTV, make CDTV hardware valid...
	 */
	if (FindResident("cdstrap"))
	{
		mmu=Mark_Address(mmu,0x00E00000,0x00080000,NONCACHEABLE);
	}

	/*
	 * Check for ReKick/ZKick/KickIt
	 */
	if ((((ULONG)(SysBase->LibNode.lib_Node.ln_Name)) >> 16) == 0x20)
	{
		mmu=Mark_Address(mmu,0x00200000,0x00080000,CACHEABLE);
	}

	/*
	 * Special case the first page of CHIP RAM
	 */
	mmu=Mark_Address(mmu,0,0x1000,NONCACHEABLE);

	/*
	 * Now, put in the free memory
	 */
	Forbid();
	mem=(struct MemHeader *)SysBase->MemList.lh_Head;
	while (mem->mh_Node.ln_Succ)
	{
	ULONG	MemType;

		MemType=((MEMF_CHIP & TypeOfMem(mem->mh_Lower)) ? NONCACHEABLE : COPYBACK);
		mmu=Mark_Address(mmu,(ULONG)(mem->mh_Lower),(ULONG)(mem->mh_Upper)-(ULONG)(mem->mh_Lower),MemType);

		/*
		 * Only make table entries for the COPYBACK pages
		 */
		if (MemType==COPYBACK) mmu=Add_NestCount(mmu,(ULONG)(mem->mh_Lower),(ULONG)(mem->mh_Upper));

		mem=(struct MemHeader *)(mem->mh_Node.ln_Succ);
	}
	Permit();

	/*
	 * Map in the autoconfig boards
	 */
	if (ExpansionBase=OpenLibrary("expansion.library",0))
	{
	struct	ConfigDev	*cd=NULL;

		while (cd=FindConfigDev(cd,-1L,-1L))
		{
			/* Skip memory boards... */
			if (!(cd->cd_Rom.er_Type & ERTF_MEMLIST))
			{
				mmu=Mark_Address(mmu,(ULONG)(cd->cd_BoardAddr),cd->cd_BoardSize,NONCACHEABLE);
			}
		}
		CloseLibrary(ExpansionBase);
	}

	return (mmu);
}
