/*
******* Enforcer **************************************************************
*
*   NAME
*	Enforcer.RAMKICK
*
*   SEE ALSO
*	"A master's secrets are only as good as the
*	 master's ability to explain them to others."  -  Michael Sinz
*
*   BUGS
*	None?
*
*******************************************************************************
*/

#include	<exec/types.h>
#include	<exec/execbase.h>
#include	<exec/tasks.h>
#include	<exec/memory.h>
#include	<exec/alerts.h>
#include        <exec/ports.h>
#include        <exec/libraries.h>
#include	<exec/semaphores.h>
#include        <dos/dos.h>
#include        <dos/dosextens.h>
#include	<dos/rdargs.h>
#include	<devices/timer.h>
#include	<workbench/startup.h>
#include	<workbench/workbench.h>
#include	<libraries/configvars.h>

#include	<clib/exec_protos.h>
#include	<pragmas/exec_pragmas.h>

#include	<clib/dos_protos.h>
#include	<pragmas/dos_pragmas.h>

#include	<clib/icon_protos.h>
#include	<pragmas/icon_pragmas.h>

#include	<clib/expansion_protos.h>
#include	<pragmas/expansion_pragmas.h>

#include	<string.h>

/*
 * A private LVO that I will use called RawPutChar...
 * I need to do the prototype myself...
 */
VOID RawPutChar(UBYTE);
#pragma libcall SysBase RawPutChar 204 001

#define EXECBASE (*(struct ExecBase **)4)

/*
 * The MMU Frame we will use...
 */
struct MMU_Frame
{
	ULONG	mmu_Flag;	/* The mmu type flag */

	ULONG	mmu_CRP[2];	/* CRP for 030 is 2 longs...  For the 040 it is the root pointer and only 1 long used */
	ULONG	mmu_TC;		/* TC for both 030 and 040.  Accessed as a long, but only part of it is used on 040 */

	ULONG	mmu_CRP_OLD[2];
	ULONG	mmu_TC_OLD;

	ULONG	*mmu_LevelA;	/* This is the base table.  The root pointer will contain this one... */
	ULONG	*mmu_LevelB;	/* Note that on the 68040, these are just the invalid pages and the full pages are elsewhere */
	ULONG	*mmu_LevelC;	/* " */

	ULONG	mmu_Indirect;	/* The indirect MMU error descriptor ;^)   For the 68040 slimmy trick... */
	ULONG	*mmu_Magic;	/* Special memory area for magic code (such as the 68040 bus error remapping magic) */

	ULONG	*mmu_Page0;	/* For 68040 page 0 work */

	ULONG	mmu_InvalidA;	/* The invalid value at LevelA  (68040 only) */
	ULONG	mmu_InvalidB;	/* The invalid value at LevelB  (68040 only) */
	ULONG	mmu_InvalidC;	/* The invalid value at LevelC  (68040 only) */

	ULONG	mmu_TT[4];	/* Storage for TT regs (68040 only) */
};

/* Values for MMU_Frame.mmu_Flag */
#define	MMU_030		1	/* MMU setup 68030 or 68020+68851 */
#define	MMU_040		2	/* MMU setup 68040 */

/*
 * General flags for setting up the MMU
 */
#define	VALID		(0x01)
#define	NONCACHEABLE	(0x40)
#define	CACHEABLE	(0x00)
#define	INVALID		(0x00)
#define	WRITEPROTECT	(0x04)

/******************************************************************************/

/* External data from assembly */
extern	char	Copyright[];
extern	char	MyTask[];
extern	char	DateStr[];
extern	char	TimeStr[];
extern	ULONG	ROM_Addr;	/* The physical address of the ROM */
extern	ULONG	Bad_ReadValue;
extern	ULONG	Quiet_Flag;
extern	ULONG	ShowPC_Flag;
extern	ULONG	Small_Flag;
extern	ULONG	Tiny_Flag;
extern	ULONG	Parallel_Flag;
extern	ULONG	StackLines;
extern	ULONG	SegLines;
extern	ULONG	ARegCheck;
extern	ULONG	DRegCheck;

extern	UBYTE	*Intro;
extern	ULONG	DoDateStamp;

extern	struct	ExecBase	*SysBase;
extern	struct	Library		*Lib68040;

extern	ULONG	SegTracker;

/* External functions from assembly... */
ULONG __asm Test_MMU(void);
BOOL __asm MMU_On(register __a0 struct MMU_Frame *);
BOOL __asm MMU_Off(register __a0 struct MMU_Frame *);

/******************************************************************************/

/* Internal functions... */
static void Free_MMU_Frame(struct MMU_Frame *mmu);
static struct MMU_Frame *Mark_Address(struct MMU_Frame *mmu,ULONG addr,ULONG size,ULONG orBits);
static struct MMU_Frame *Map_ROM(struct MMU_Frame *mmu,ULONG addr);
static struct MMU_Frame *Mark_Invalid(struct MMU_Frame *mmu);
static struct MMU_Frame *Init_MMU_Frame(ULONG MMU_Flag);

/******************************************************************************/

ULONG cmd(void)
{
struct	Library		*ExpansionBase;
struct	MMU_Frame	*mmu;

	SysBase = EXECBASE;

	{

		/*
		 * Open the 68040.library
		 */
		Lib68040=OpenLibrary("68040.library",0);

		/*
		 * If all is OK, start up the system as needed...
		 */
		{
			ULONG		MMU_Flag;
		struct	SignalSemaphore	*sem;

			/* Various flags... */
			Quiet_Flag=FALSE;
			Tiny_Flag=FALSE;
			Small_Flag=FALSE;
			ShowPC_Flag=FALSE;
			Parallel_Flag=FALSE;
			Intro=NULL;
			DoDateStamp=FALSE;
			ARegCheck=TRUE;
			DRegCheck=FALSE;

			if (sem=FindSemaphore("SegTracker"))
			{
				sem++;
				SegTracker=*(ULONG *)sem;
			}

			/* Set the stack lines... */
			StackLines=6;	/* Default to 6... */
			SegLines=StackLines << 3;

			/*
			 * MMU_Flag is:
			 *
			 *	1 - For standard 68020/68030
			 *	2 - For 68040
			 */
			MMU_Flag=Test_MMU();
			if (mmu=Init_MMU_Frame(MMU_Flag))
			{
			struct	MemHeader	*mem;

				/*
				 * Special case the first page of CHIP RAM
				 */
				mmu=Mark_Address(mmu,0,0x1000,VALID | NONCACHEABLE);

				/*
				 * Map in the free memory
				 */
				Forbid();
				mem=(struct MemHeader *)SysBase->MemList.lh_Head;
				while (mem->mh_Node.ln_Succ)
				{
					mmu=Mark_Address(mmu,(ULONG)(mem->mh_Lower),(ULONG)(mem->mh_Upper)-(ULONG)(mem->mh_Lower),((MEMF_CHIP & TypeOfMem(mem->mh_Lower)) ? (NONCACHEABLE | VALID) : (CACHEABLE | VALID)));
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
							mmu=Mark_Address(mmu,(ULONG)(cd->cd_BoardAddr),cd->cd_BoardSize,VALID | NONCACHEABLE);
						}
					}
					CloseLibrary(ExpansionBase);
				}

				/*
				 * Now for the control areas...
				 */
				mmu=Mark_Address(mmu,0x00B80000,0x00080000,VALID | NONCACHEABLE);
				mmu=Mark_Address(mmu,0x00D80000,0x00080000,VALID | NONCACHEABLE);
				mmu=Mark_Address(mmu,0x00F00000,0x00100000,VALID | CACHEABLE | WRITEPROTECT);
				mmu=Mark_Address(mmu,0x00E00000,0x00080000,VALID | CACHEABLE | WRITEPROTECT);
				mmu=Map_ROM(mmu,ROM_Addr);

				/*
				 * If the credit card resource, make the addresses valid...
				 */
				if (OpenResource("card.resource"))
				{
					mmu=Mark_Address(mmu,0x00600000,0x00440002,VALID | NONCACHEABLE);
				}

				/*
				 * Check if we should mark low memory invalid...
				 */
				mmu=Mark_Invalid(mmu);

				/*
				 * If all OK still, we go and install ourselves...
				 */
				if (MMU_On(mmu))
				{
					mmu=NULL;
				}

				/*
				 * Free up the tables now...
				 */
				Free_MMU_Frame(mmu);
			}
		}

		CloseLibrary(Lib68040);
	}
	return(0);
}

#include	<exec/resident.h>

struct	Resident	myTag=
{
	RTC_MATCHWORD,
	&myTag,
	(APTR) Free_MMU_Frame,
	RTF_COLDSTART,
	37,
	0,
	90,
	"Enforcer RAMKick",
	"Enforcer RAMKick",
	(APTR) cmd,
};


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

/* Constants */
#define LEVELA_BITS_030		8	/* Increment in init loop is manually calculated */
#define LEVELB_BITS_030		6
#define LEVELC_BITS_030		8
#define LEVELA_SIZE_030		(1<<LEVELA_BITS_030)
#define LEVELB_SIZE_030		(1<<LEVELB_BITS_030)
#define LEVELC_SIZE_030		(1<<LEVELC_BITS_030)
#define LEVELA_ALLOC_030	(LEVELA_SIZE_030*4)
#define LEVELB_ALLOC_030	(LEVELB_SIZE_030*4)
#define LEVELC_ALLOC_030	(LEVELC_SIZE_030*4)

#define PAGE_SIZE_040	(0x1000)
#define	ALLOC_GRAN_040	(0x01FF)
#define	LEVELA_SIZE_040	(128)
#define	LEVELB_SIZE_040	(128)
#define	LEVELC_SIZE_040	(64)
#define	LEVELA_ALLOC_040	(4*LEVELA_SIZE_040)
#define	LEVELB_ALLOC_040	(4*LEVELB_SIZE_040)
#define	LEVELC_ALLOC_040	(4*LEVELC_SIZE_040)

#define	TABLE_MASK	(0xFFFFFFF3)

/*
 * Clean up the MMU Frame as needed...
 */
static void Free_MMU_Frame(struct MMU_Frame *mmu)
{
ULONG	i;
ULONG	j;
ULONG	*temp1;

	if (mmu)
	{
		switch (mmu->mmu_Flag)
		{
			/* 68030 MMU Frame Cleanup */
		case MMU_030:	if (mmu->mmu_LevelA) FreeMem(mmu->mmu_LevelA,LEVELA_ALLOC_030);
				if (mmu->mmu_LevelB) FreeMem(mmu->mmu_LevelB,LEVELB_ALLOC_030);
				if (mmu->mmu_LevelC) FreeMem(mmu->mmu_LevelC,LEVELC_ALLOC_030);
				break;

			/* 68040 MMU Frame Cleanup */
		case MMU_040:	if ((mmu->mmu_LevelA) && (mmu->mmu_LevelB) && (mmu->mmu_LevelC) && (mmu->mmu_Magic))
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
				break;
		}

		/* Free the MMU Frame */
		FreeVec(mmu);
	}
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

	if (mmu)
	{
		switch (mmu->mmu_Flag)
		{
		case MMU_030:	temp1=addr >> 18;
				/* Crude hack routine here for Enforcer-like MMU tables...  Not a full general-perpose MMU thingy yet... */
				if (temp1 < (0xFF / 4))
				{
					temp2=(addr+size-1) >> 18;
					while (temp1 <= temp2)
					{
						if (!(mmu->mmu_LevelB[temp1] & 0x02)) mmu->mmu_LevelB[temp1] |= orBits;
						temp1++;
					}
				}
				else mmu->mmu_LevelA[temp1 >> 6] |= orBits;
				rc=TRUE;
				break;
			/*
			 * For the 68040 we need to maybe allocate another table
			 * if the address falls within that new table...
			 */
		case MMU_040:	/* First, figure out the replacement orBits */
				temp1=0x20;	/* Copyback, cacheable, writeable, invalid */
				if (orBits & VALID) temp1 |= 0x01;	/* Resident */
				if (orBits & NONCACHEABLE) temp1 += 0x20;	/* Make the 0x20 into a 0x40 */
				if (orBits & WRITEPROTECT) temp1 |= 0x04;	/* Write protected */
				orBits=temp1 | 0x400;	/* New orBits plus the global flag ;^)  (trick) */

				temp1=addr >> 12;	/* Remove page offset... */
				temp2=(addr+size-1) >> 12;

				if (temp1>temp2) rc=TRUE;	/* Small size is OK */

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
				break;
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
 * This routine will mark the invalid addresses as invalid
 * The whole address page is marked, not just the single address
 * since this would be impossible on the current MMUs
 * It will also make the indirect frame marked as invalid.
 */
static struct MMU_Frame *Mark_Invalid(struct MMU_Frame *mmu)
{
	if (mmu)
	{
		switch (mmu->mmu_Flag)
		{
			/*
			 * The 68030 case is rather simple...  ;^)
			 */
		case MMU_030:	mmu->mmu_LevelC[0]=0xBADF00D0;
				break;

			/*
			 * The 68040 case is a bit harder...
			 */
		case MMU_040:	/*
				 * First make sure the table is build
				 */
				mmu->mmu_Indirect=((ULONG)(mmu->mmu_Magic)) | 0x044; /* invalid, write protected, non-cached, serialized */
				if (mmu=Mark_Address(mmu,0,1,0))
				{
				ULONG	*level;

					/* Now we know the tree down to the address is there... */
					level=(ULONG *)(mmu->mmu_LevelA[0] & ~ALLOC_GRAN_040);	/* Get LevelB */
					level=(ULONG *)(level[0] & ~ALLOC_GRAN_040);		/* Get LevelC */
					level[0]=0x00000044;					/* Non-cached, serialized, write protected, invalid */
					mmu->mmu_Page0=level;					/* Store Page 0 address */
				}
				break;
		}
	}
	return(mmu);
}

/*
 * This routine will map the ROM addresses to the physical
 * addresses given here...
 */
static struct MMU_Frame *Map_ROM(struct MMU_Frame *mmu,ULONG addr)
{
ULONG	temp1;
ULONG	*level;

	mmu=Mark_Address(mmu,0x00F80000,0x00080000,VALID | CACHEABLE | WRITEPROTECT);

	/*
	 * Check if we need to do a special mapping...
	 */
	if (mmu) if (0x00F80000 != addr)
	{
		switch (mmu->mmu_Flag)
		{
			/*
			 * The 68030 case is rather simple...  ;^)
			 */
		case MMU_030:	mmu->mmu_LevelB[0xF8 / 4] = addr | 0x1D;		/* Cacheable, Writeprotected, Resident */
				mmu->mmu_LevelB[0xFC / 4] = (addr+0x00040000) | 0x1D;	/* Cacheable, Writeprotected, Resident */
				break;

			/*
			 * The 68040 case is a bit harder...
			 */
		case MMU_040:	temp1=0x00F80000 >> 12;
				/* Starting at the ROM, do the full 512K space mapping... */
				while (temp1 <= (0x00FFFFFF >> 12))
				{
					level=(ULONG *)(mmu->mmu_LevelA[temp1 >> 13] & ~ALLOC_GRAN_040);	/* LevelB */
					level=(ULONG *)(level[(temp1 >> 6) & 0x7F] & ~ALLOC_GRAN_040);		/* LevelC */
					level[temp1 & 0x3F] = addr | 0x425;	/* Global, Cacheable, Writeprotected, Resident */

					temp1++;	addr+=0x00001000;	/* Next page:  4K at a shot... */
				}
				break;
		}
	}

	return(mmu);
}

static struct MMU_Frame *Init_MMU_Frame(ULONG MMU_Flag)
{
struct	MMU_Frame	*mmu;
	ULONG		i;
	BOOL		rc=FALSE;

	if (mmu=AllocVec(sizeof(struct MMU_Frame),MEMF_PUBLIC|MEMF_CLEAR))
	{
		mmu->mmu_Flag=MMU_Flag;
		if (MMU_Flag==MMU_030)
		{
			mmu->mmu_LevelA=AllocAligned(LEVELA_ALLOC_030,0x0000000F);
			mmu->mmu_LevelB=AllocAligned(LEVELB_ALLOC_030,0x0000000F);
			mmu->mmu_LevelC=AllocAligned(LEVELC_ALLOC_030,0x0000000F);

			if ((mmu->mmu_LevelA) && (mmu->mmu_LevelB) && (mmu->mmu_LevelC))
			{
				/*
				 * Now build the base MMU table...
				 */
				for (i=0;i<LEVELA_SIZE_030;i++) mmu->mmu_LevelA[i]=(i << 24);
				for (i=0;i<LEVELB_SIZE_030;i++) mmu->mmu_LevelB[i]=(i << 18);
				for (i=0;i<LEVELC_SIZE_030;i++) mmu->mmu_LevelC[i]=(i << 10) | VALID | NONCACHEABLE;

				/*
				 * Connect up the tables.
				 */
				mmu->mmu_TC	=0x80A08680;
				mmu->mmu_CRP[0]	=0x80000002;
				mmu->mmu_CRP[1]	=(ULONG)mmu->mmu_LevelA;
				mmu->mmu_LevelA[0]=((ULONG)mmu->mmu_LevelB) | 0x02;
				mmu->mmu_LevelB[0]=((ULONG)mmu->mmu_LevelC) | 0x02;

				rc=TRUE;
			}
		}
		else if (MMU_Flag==MMU_040)
		{
			mmu->mmu_LevelA=AllocAligned(LEVELA_ALLOC_040,ALLOC_GRAN_040);
			mmu->mmu_LevelB=AllocAligned(LEVELB_ALLOC_040,ALLOC_GRAN_040);
			mmu->mmu_LevelC=AllocAligned(LEVELC_ALLOC_040,ALLOC_GRAN_040);
			mmu->mmu_Magic=AllocAligned(PAGE_SIZE_040,PAGE_SIZE_040-1);

			if ((mmu->mmu_LevelA) && (mmu->mmu_LevelB) && (mmu->mmu_LevelC) && (mmu->mmu_Magic))
			{
				/* First, set up the fake page memory to be what I want */
				for (i=0;i<(PAGE_SIZE_040/4);i++) mmu->mmu_Magic[i]=Bad_ReadValue;

				/* Now, set up the default invalid page pointers... */
				mmu->mmu_Indirect=((ULONG)(mmu->mmu_Magic)) | 0x041; /* valid, non-cached, serialized */
				mmu->mmu_InvalidC=((ULONG)&(mmu->mmu_Indirect)) | 0x02; /* Indirect to the invalid one... */
				mmu->mmu_InvalidB=((ULONG)(mmu->mmu_LevelC)) | 0x03; /* Resident table */
				mmu->mmu_InvalidA=((ULONG)(mmu->mmu_LevelB)) | 0x03; /* Resident table */

				for (i=0;i<LEVELC_SIZE_040;i++) mmu->mmu_LevelC[i]=mmu->mmu_InvalidC;
				for (i=0;i<LEVELB_SIZE_040;i++) mmu->mmu_LevelB[i]=mmu->mmu_InvalidB;
				for (i=0;i<LEVELA_SIZE_040;i++) mmu->mmu_LevelA[i]=mmu->mmu_InvalidA;

				mmu->mmu_TC=0x00008000;	/* Enable MMU with 4K pages */
				mmu->mmu_CRP[0]=(ULONG)mmu->mmu_LevelA;
				mmu->mmu_CRP[1]=(ULONG)mmu->mmu_LevelA;

				/* For simple games, make default PAGE0 point at the indirect page */
				mmu->mmu_Page0=&(mmu->mmu_Indirect);

				/* We now have a full 68040 MMU tree, everything is marked invalid... */
				rc=TRUE;
			}
		}
	}

	if (!rc)
	{
		Free_MMU_Frame(mmu);
		mmu=NULL;
	}

	return (mmu);
}
