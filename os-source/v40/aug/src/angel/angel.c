#include <dos.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/resident.h>
#include <exec/execbase.h>
#include <libraries/dos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include <string.h>
#include "mmu.h"
#include "angel_rev.h"

#define TAIL_PROTECT	1

/****************************************************************************/
/* The following variables are for producing an executable that detaches    */
/* itself from the CLI window.                                              */
/****************************************************************************/

long _stack = 4000;             /* a reasonable amount of stack space */
char *_procname = "Guardian-Angel";
long _priority = 0;             /* run at standard priority */
long _BackGroundIO = 1;         /* perform background I/O */
extern BPTR _Backstdout;        /* file handle we will write to with */

/****************************************************************************/
/* The following are external functions                                     */
/****************************************************************************/

void NewBusError();			
extern void NewAllocMem(), NewAllocVec(), NewFreeMem(), NewFreeVec();
extern void NewAllocAbs(), NewAvailMem(), NewRawPutChar(), NewPermit();
extern void __stdargs SetCACR(ULONG);
extern void __stdargs GetCRP(ULONG *);
extern void __stdargs SetCRP(ULONG *);
extern void __stdargs SetTC(ULONG);
extern void __stdargs SetTT0(ULONG);
extern void __stdargs SetTT1(ULONG);
extern void __stdargs SetMMU(ULONG *, ULONG);
extern void __stdargs SetVBR(ULONG);
extern void __stdargs SetSRP(ULONG *);
extern ULONG __stdargs GetCACR(void);
extern ULONG __stdargs GetTC(void);
extern ULONG __stdargs GetTT0(void);
extern ULONG __stdargs GetTT1(void);
extern ULONG __stdargs GetVBR(void);
extern ULONG __stdargs GetMMUType(void);
extern void __stdargs PFlush(ULONG);
extern void __stdargs PFlushA();

/****************************************************************************/
/* These are prototypes for functions defined within this module            */
/****************************************************************************/

struct MsgPort *port, *FindPort(), *CreatePort();
void DeletePort();

/****************************************************************************/
/* The following are variables used in the Bus Error vector substitution    */
/****************************************************************************/

ULONG *vector, OldVector = NULL;
extern ULONG Base;

/****************************************************************************/
/* These variables are used globally by the program                         */
/****************************************************************************/

APTR AllocMemPointer, AllocVecPointer, FreeMemPointer, FreeVecPointer;
APTR AllocAbsPointer, AvailMemPointer, RawPutCharPointer, PermitPointer;
BOOL Fast=FALSE,NoChip=FALSE,MultiProcessing=FALSE,MultiTasking=FALSE,
     Scratch=FALSE,Off=FALSE,On=FALSE,NoLeak=FALSE,Quiet=FALSE,Report=FALSE,
     TailProtect=FALSE;
BOOL ForbidFree=FALSE;		/* Did a free occurr during a Forbid()? */
ULONG *TableA = NULL, *TableB_FAST = NULL, *TableB_CHIP = NULL;

ULONG pages;		/* How many pages of fast ram does MMU FAST ram require? */

ULONG layers_start, layers_end;
struct Resident *resident;

ULONG FAST_region = NULL;	/* The MEMF_FAST memory region to be covered */
ULONG FAST_size = NULL;		/* Its size */
ULONG *FAST_table = NULL;	/* The table pointing at the first page */
ULONG FAST_last;		/* The last index of the table */
ULONG FAST_flags;		/* The flags of the FAST memory block */

ULONG CHIP_region = NULL;
ULONG CHIP_size = NULL;
ULONG *CHIP_table = NULL;
ULONG CHIP_last;
ULONG CHIP_flags;

ULONG Super_CRP[2], Super_TC, Super_TT0, Super_TT1;

/****************************************************************************/
/* These two system variables should always be declared like this           */
/****************************************************************************/

extern struct Library *DOSBase;
extern struct Library *SysBase;
#define EXECBASE ((struct ExecBase *)SysBase)

/****************************************************************************/
/* From now on, a page is available if it is of type PD_DT_INVALID or if
/* the bit 7 is set.
/****************************************************************************/

/* Valid() tests to see if it's not protected.
 * Invalid() tests to see if it's protected and alloc'd.
 * Available() tests to see if it's protected or free.
 * IsFree() tests to see if it's free.
 */
#define IsFree(x)	( (x) & 0x80 )
#define IsProtected(x)	( ((x)&0x03)==PD_DT_INVALID )

/* Validate() marks as not protected and alloc'd.
 * Invalidate() marks as protected and alloc'd.
 * Free() marks as not-alloc'd.
 */
#define Protect(x)	( (x) &= ~0x03 )
#define Unprotect(x)	( (x) |= PD_DT_PAGE )
#define	Alloc(x)	( (x) &= ~PD_FREE )
#define Free(x)		( (x) |= PD_FREE )
/****************************************************************************/
/* This function, upon a Ctrl-C, re-establishes the old Bus Error vector in */
/* preparation of the removal of this program                               */
/****************************************************************************/

void CXBRK()
{
    Disable();
    vector = (ULONG *)(GetVBR()+8);
    vector[0] = (ULONG)OldVector;
    Enable();
}

/****************************************************************************/
/* This replaces the Lattice "stricmp()" function, plus it's a better form  */
/* for my needs here. (written by Dave Haynie)                              */
/****************************************************************************/

static BOOL striequ(char *s1, char *s2)
{
   BOOL aok;
   
   while (*s1 && *s2 && (aok = (*s1++ & 0xdf) == (*s2++ & 0xdf)));
   return (BOOL) (!*s1 && aok);
}

/****************************************************************************/
/* This function allocates a block of memory according to the required      */
/* alignement.                                                              */
/****************************************************************************/

static void *AllocAligned(ULONG size, ULONG bound, ULONG flags)
{
   void *mem, *aligned;

   if (!(mem = (void *)AllocMem(size+bound,flags))) return NULL;
   Forbid();
   aligned = (void *)(((ULONG)mem + bound - 1) & ~(bound - 1));
   FreeMem(mem,size+bound);
   mem = (void *)AllocAbs(size,aligned);
   Permit();
   return mem;
}

#define TableTC (TC_ENB|TC_PS(8)|TC_IS(0)|TC_TIA(0xE)|TC_TIB(0xA)|TC_TIC(0)|TC_TID(0))
#define TC_SuperKickstart 0x80f08630

BOOL CreateTable()
{
    ULONG i, j, round, myCRP[2], myTC;
    ULONG oldVBR, newVBR;
    ULONG blocks;		/* How many blocks of fast ram does that require? */
    ULONG base;			/* Temporary base variable */

    /***********************************************************************/
    /* If the Exec has not moved the vector table away from page zero yet, */
    /* then I better do it myself.                                         */
    /***********************************************************************/

    if (!(oldVBR = (ULONG)GetVBR()))
    {
	if (newVBR = (ULONG)AllocMem(1024,MEMF_PUBLIC))
	{
	    CopyMemQuick((ULONG *)oldVBR,(ULONG *)newVBR,1024);
	    SetVBR(newVBR);
	}
	else return(FALSE);
    }
    /***********************************************************************/
    /* Let's allocate memory for the translation tables.                   */
    /***********************************************************************/

    round = 256;

    Forbid();
	FAST_size = AvailMem(MEMF_LARGEST|MEMF_FAST)-1024;
	FAST_region = (ULONG)AllocAligned(FAST_size,256,MEMF_FAST);
    Permit();
    FAST_flags = TypeOfMem((APTR)FAST_region);
    if (!FAST_region) return(FALSE);
    if (!(TableA = AllocAligned(1024*4,16,MEMF_CHIP))) /* Allocate 1024 entries */
    {
	FreeMem((APTR)FAST_region,FAST_size);
	return(FALSE);
    }
    if (!(TableB_CHIP = AllocAligned(8192*4,16,MEMF_CHIP))) /* Allocate 8192 entries */
    {
	FreeMem((APTR)FAST_region,FAST_size);
	FreeMem(TableA,16*4);
	return(FALSE);
    }
    pages = (FAST_size/262144);
    if (pages*262144 != FAST_size) pages++;
    pages = pages*262144/256;

    if (!(TableB_FAST = AllocAligned(pages*4,16,MEMF_CHIP))) /* Allocate for FAST mem */
    {
	FreeMem((APTR)FAST_region,FAST_size);
	FreeMem(TableB_CHIP,8192*4);
	FreeMem(TableA,16*4);
	return(FALSE);
    }

    if (NoChip)
    {
	CHIP_size = 0;
	CHIP_region = NULL;
	CHIP_flags = NULL;
    }
    else
    {
	Forbid();
	CHIP_size = AvailMem(MEMF_LARGEST|MEMF_CHIP)-256;
	CHIP_region = (ULONG)AllocAligned(CHIP_size,256,MEMF_CHIP);
	Permit();
	CHIP_flags = TypeOfMem((APTR)CHIP_region);
    }

    /***********************************************************************/
    /* Let's set up the translation tables. Easy! Well...                  */
    /***********************************************************************/
    if (GetTC() == TC_SuperKickstart)
    {
	GetCRP(Super_CRP);
	Super_TC = GetTC();
	Super_TT0 = GetTT0();
	Super_TT1 = GetTT1();

	for (i=0; i<8; i++)
	{
	    TableA[i] = ((ULONG)TableB_CHIP + (i*262144/256*4)) | PD_DT_V4BYTE;
	}
	for (i=8; i<64; i++) TableA[i] = (i*262144) | PD_DT_PAGE;
	TableA[47] = (47*262144) | PD_DT_PAGE | PD_CI;
	TableA[55] = (55*262144) | PD_DT_PAGE | PD_CI;
	TableA[56] = (56*262144) | PD_DT_PAGE | PD_CI;
	TableA[57] = (57*262144) | PD_DT_PAGE | PD_CI;
	TableA[58] = (58*262144) | PD_DT_PAGE | PD_CI;
	TableA[59] = (59*262144) | PD_DT_PAGE | PD_CI;
	TableA[62] = 0x07F80000 | PD_DT_PAGE | PD_WP;
	TableA[63] = 0x07FC0000 | PD_DT_PAGE | PD_WP;

	for (i=64; i<448; i++) TableA[i] = (i*262144) | PD_DT_INVALID;
	for (i=488; i<576; i++) TableA[i] = (i*262144) | PD_DT_PAGE;
	for (i=576; i<1024; i++) TableA[i] = (i*262144) | PD_DT_INVALID;
	TableA[504] = (504*262144) | PD_DT_PAGE;
	TableA[508] = (508*262144) | PD_DT_PAGE;
	TableA[509] = (509*262144) | PD_DT_PAGE;
	TableA[510] = (510*262144) | PD_DT_PAGE | PD_WP;
	TableA[511] = (511*262144) | PD_DT_PAGE | PD_WP;

	for (i=0; i<4; i++) TableB_CHIP[i] = PD_DT_INVALID;
	for (i=4; i<8192; i++) TableB_CHIP[i] = (256 * i) | PD_DT_PAGE;

	blocks = (pages*256)/262144;
	if (blocks*262144 != (pages*256)) blocks++;
	base = FAST_region&0xFFFC0000;
	j = base/262144;

	for (i=0; i<blocks; i++)
	{
	    TableA[j+i] = ((ULONG)TableB_FAST + (i*262144/256*4)) | PD_DT_V4BYTE;
	}

	for (i=0; i<pages; i++)
	{
	    TableB_FAST[i] = ((FAST_region&0xFFFC0000) + (i*256)) | PD_DT_PAGE;
	}
    }
    else
    {
	for (i=0; i<8; i++)
	{
	    TableA[i] = ((ULONG)TableB_CHIP + (i*262144/256*4)) | PD_DT_V4BYTE;
	}
	for (i=8; i<64; i++) TableA[i] = (i*262144) | PD_DT_PAGE;
	TableA[47] = (47*262144) | PD_DT_PAGE | PD_CI;
	TableA[55] = (55*262144) | PD_DT_PAGE | PD_CI;
	TableA[56] = (56*262144) | PD_DT_PAGE | PD_CI;
	TableA[57] = (57*262144) | PD_DT_PAGE | PD_CI;
	TableA[58] = (58*262144) | PD_DT_PAGE | PD_CI;
	TableA[59] = (59*262144) | PD_DT_PAGE | PD_CI;
	TableA[62] = (62*262144) | PD_DT_PAGE | PD_WP;
	TableA[63] = (63*262144) | PD_DT_PAGE | PD_WP;
	for (i=64; i<1024; i++) TableA[i] = (i*262144) | PD_DT_INVALID;

	for (i=0; i<4; i++) TableB_CHIP[i] = PD_DT_INVALID;
	for (i=4; i<8192; i++) TableB_CHIP[i] = (256 * i) | PD_DT_PAGE;

	blocks = (pages*256)/262144;
	if (blocks*262144 != (pages*256)) blocks++;
	base = FAST_region&0xFFFC0000;
	j = base/262144;

	for (i=0; i<blocks; i++)
	{
	    TableA[j+i] = ((ULONG)TableB_FAST + (i*262144/256*4)) | PD_DT_V4BYTE;
	}

	for (i=0; i<pages; i++)
	{
	    TableB_FAST[i] = ((FAST_region&0xFFFC0000) + (i*256)) | PD_DT_PAGE;
	}
    }

    FAST_table = &TableB_FAST[(FAST_region&0x0003FFFF)/256];
    FAST_last = FAST_size/256;

    CHIP_table = &TableB_CHIP[CHIP_region/256];
    CHIP_last = CHIP_size/256;

KPrintF("TableA: %lx   TableB_FAST: %lx   TableB_CHIP: %lx \n",TableA,TableB_FAST,TableB_CHIP);
KPrintF("FAST_region: %lx   FAST_size: %lx   FAST_last: %ld \n",
	 FAST_region,FAST_size,FAST_last);
KPrintF("FAST_table[0]: %lx   FAST_table[last]: %lx \n",
	 FAST_table[0],FAST_table[FAST_last]);
KPrintF("CHIP_region: %lx   CHIP_size: %lx   CHIP_last: %ld \n",
	 CHIP_region,CHIP_size,CHIP_last);
KPrintF("CHIP_table[0]: %lx   CHIP_table[last]: %lx \n",
	 CHIP_table[0],CHIP_table[CHIP_last]);

    for (i=0; i<FAST_last; i++)
    {
	Protect(FAST_table[i]);
	Free(FAST_table[i]);
    }
    for (i=0; i<CHIP_last; i++)
    {
	Protect(CHIP_table[i]);
	Free(CHIP_table[i]);
    }

    myCRP[0] = CRP_LIMIT(0x400) | CRP_SG | CRP_DT_V4BYTE;
    myCRP[1] = (ULONG)TableA;
    myTC = TableTC;

    /***********************************************************************/
    /* Not only should the following be mutually excluded by grabing the   */
    /* MMU semaphore, but the entire translation table process should be   */
    /* too. So let's just pray nobody else is mucking with the MMU         */
    /* meanwhile. I'll fix this one day.                                   */
    /***********************************************************************/

    Disable();
	if (EXECBASE->AttnFlags&AFF_68030)
	{
	    SetTT0(0L);
	    SetTT1(0L);
	}
	SetMMU(myCRP,myTC);
    Enable();
    return(TRUE);
}

void DeleteTable()
{
    if ((TableA[62]&0xFFFFFF00) == 0x07F80000)
    {
	SetMMU(Super_CRP,Super_TC);
	if (EXECBASE->AttnFlags&AFF_68030)
	{
	    SetTT0(Super_TT0);
	    SetTT1(Super_TT1);
	}
    }
    else
    {
	SetTC(0L);
    }
    FreeMem(TableA,1024*4);
    FreeMem(TableB_CHIP,8192*4);
    FreeMem(TableB_FAST,pages*4);
}

APTR __asm C_AllocMem(register __d0 ULONG byteSize, register __d1 ULONG requirements)
{
    ULONG pages;	/* the number of 256 byte pages needed */
    ULONG free;		/* the number of free pages found so far */
    APTR  address;	/* starting address of free page range */
    ULONG i,j;		/* counters */
    ULONG first_index;	/* first index of allocated free page list */
    ULONG *ZeroTable;	/* Table in which to zero out all cells */
    ULONG *Table;	/* Translation table of free pages */
    ULONG Table_last;	/* The last index of the free page table */

    /* KPrintF("AllocMem: %ld  %08lx \n",byteSize,requirements); */

    if ((requirements&(FAST_flags|0xFFFF0000)) == requirements)
    {
	Table = FAST_table;
	Table_last = FAST_last;
    }
    else if ((requirements&(CHIP_flags|0xFFFF0000)) == requirements)
    {
	Table = CHIP_table;
	Table_last = CHIP_last;
    }
    else
    {
	/* KPrintF("Memory requirements not available from MMU table\n"); */
	return(0);
    }

    pages = byteSize/256;
    if ((pages*256) != byteSize) pages++;

    if (TailProtect)
    {
	/* Allocate an extra page to catch tail-trashers */
	pages++;
    }

    Forbid();
	address = (ULONG *)(Table[0]&0xFFFFFF00);
	first_index = 0; free = 0;

	for (i=0; i<Table_last; i++)
	{
	    if (IsFree(Table[i]))
	    {
		free++;
		if (free==pages) i = Table_last;
	    }
	    else
	    {
		free = 0;
		first_index = i+1;
		address = (ULONG *)(Table[first_index]&0xFFFFFF00);
	    }
	}

	if (free!=pages)
	{
	    KPrintF("Not enough free pages \n");
	    Permit();
	    return(NULL);
	}

	for (i=first_index; i<(first_index+pages); i++)
	{
	    Alloc( Table[i] );
	    Unprotect( Table[i] );
	    /* KPrintF("+"); */

	}

	if (requirements&MEMF_CLEAR)
	{
	    for (i=first_index; i<(first_index+pages); i++)
	    {
		ZeroTable = (ULONG *)(Table[i]&0xFFFFFF00);
		for (j=0; j<64; j++) ZeroTable[j] = NULL;
	    }
	}

    if (TailProtect)
    {
	/* Mark the last page as Invalid */
	Protect( Table[first_index+pages-1] );
    }

	PFlushA();			/* flush the descriptors out of the ATC */
    Permit();

    if (TailProtect)
    {
	ULONG offset;

	/* Munge the address we return so that the returned block
	 * is right-aligned to the last page (while still being
	 * longword aligned)
	 */
	offset = ( ( (byteSize % 256) + 3 ) & ~3 );
	if (offset)
	{
	    address = (APTR) ( ((ULONG) address) + 256 - offset );
	}
    }
    return(address);
}

APTR __asm C_AllocVec(register __d0 ULONG byteSize, register __d1 ULONG requirements)
{
	return(NULL);
}

BOOL __asm C_FreeMem(register __a1 APTR memoryBlock, register __d0 ULONG byteSize)
{
    ULONG pages;	/* the number of 256 byte pages needed */
    ULONG first_index;	/* the first index of the page of the allocated block */
    ULONG i;		/* counters */
    BYTE Count;		/* Forbid nest count */
    ULONG *Table;
    ULONG Table_last;
    ULONG Table_region;
    ULONG Table_size;
    ULONG requirements;

/*
KPrintF(".");
KPrintF("FreeMem memoryBlock: %lx   byteSize: %ld \n",memoryBlock,byteSize);
KPrintF("region: %lx   region size: %ld \n",FAST_region,FAST_size);
*/

    pages = byteSize/256;
    if ((pages*256) != byteSize) pages++;

    if (TailProtect)
    {
	ULONG offset;

	/* Count the extra page we used to catch tail-trashers */
	pages++;

	/* Get the real address back */
	offset = ( ( (byteSize % 256) + 3 ) & ~3 );
	if (offset)
	{
	    memoryBlock = (APTR) ( ((ULONG) memoryBlock) + offset - 256 );
	}
    }

    requirements = TypeOfMem(memoryBlock);
    if ((requirements&(FAST_flags|0xFFFF0000)) == requirements)
    {
	Table = FAST_table;
	Table_last = FAST_last;
	Table_region = FAST_region;
	Table_size = FAST_size;
    }
    else if ((requirements&(CHIP_flags|0xFFFF0000)) == requirements)
    {
	Table = CHIP_table;
	Table_last = CHIP_last;
	Table_region = CHIP_region;
	Table_size = CHIP_size;
    }
    else return(NULL);

    if (((ULONG)memoryBlock<Table_region) || ((ULONG)memoryBlock>(Table_region+Table_size)))
	return(NULL);

    if (((ULONG)memoryBlock&0x000000FF) != 0)
    {
	KPrintF("Non-aligned address parameter, FreeMem(0x%lx,%ldL)\n",memoryBlock,byteSize);
	if (((struct ExecBase *)getreg(REG_A6))->ThisTask->tc_Node.ln_Name)
	    KPrintF("Task: %s\n\n",((struct ExecBase *)getreg(REG_A6))->ThisTask->tc_Node.ln_Name);
	return(NULL);
    }

    first_index = (((ULONG)memoryBlock-(ULONG)Table_region)/256);

    Count = ((struct ExecBase *)getreg(REG_A6))->TDNestCnt;
    if (Count>=0)
    {
	/* KPrintF("FreeMem: Forbid nest count is %lx \n",(ULONG)Count); */
	ForbidFree=TRUE;
    }

    Forbid();

    if (TailProtect)
    {
	/* Revalidate the last one to not confuse the freeing tests */
	Unprotect( Table[first_index+pages-1] );
    }

	for (i=first_index; i<(first_index+pages); i++)
	{
	if ( IsFree( Table[i] ) || IsProtected( Table[i] ) )
	    {
		SetTC(0L);
		KPrintF("Alert $F1000005, corrupt MMU free page table \n");
		Alert(0xF1000005);
	    }
	    else
	    {
		Free(Table[i]);
		if (!ForbidFree)
		{
			Protect(Table[i]);
		}
		/* KPrintF("-%lx ",Table[i]); */
		/* KPrintF("-"); */
	    }
	}
	PFlushA();		/* flush all descriptors out of the ATC */
    Permit();
    return(TRUE);
}

BOOL __asm C_FreeVec(register __a1 APTR memoryBlock)
{
	return(FALSE);
}

APTR __asm C_AllocAbs(register __d0 ULONG byteSize, register __a1 APTR location)
{
	KPrintF("AllocAbs() called\n");
	return(NULL);
}

ULONG __asm C_AvailMem(register __d1 ULONG requirements)
{
    ULONG i, free=0, size=0, largest=0;
    ULONG *Table;
    ULONG Table_last;

    if ((requirements&(FAST_flags|MEMF_LARGEST)) == requirements)
    {
	Table = FAST_table;
	Table_last = FAST_last;
    }
    else if ((requirements&(CHIP_flags|MEMF_LARGEST)) == requirements)
    {
	Table = CHIP_table;
	Table_last = CHIP_last;
    }
    else return(0);

    Forbid();
	for (i=0; i<Table_last; i++)
	{
	    if (IsFree(Table[i]))
	    {
		free++;
		size++;
	    }
	    else
	    {
		if (size>largest) largest=size;
		size = 0;
	    }
	}
	if (size>largest) largest=size;
    Permit();

    if (requirements&MEMF_LARGEST) return(largest*256);
    else return(free*256);
}

VOID C_Permit()
{
    int i;		/* A simple counter */

    if (((struct ExecBase *)getreg(REG_A6))->TDNestCnt==-1)
    {
	KPrintF("Whoa! TDNestCnt about to go from -1 to -2!\n");
    }

    if (ForbidFree)
    {
	if (((struct ExecBase *)getreg(REG_A6))->TDNestCnt==0)
	{
	    ForbidFree = FALSE;
	    for (i=0; i<FAST_last; i++)
	    {
		if ( (IsFree(FAST_table[i])) && (!IsProtected(FAST_table[i])) )
		{
		    Protect(FAST_table[i]);
		}
	    }
	    for (i=0; i<CHIP_last; i++)
	    {
		if ( (IsFree(CHIP_table[i])) && (!IsProtected(CHIP_table[i])) )
		{
		    Protect(CHIP_table[i]);
		}
	    }
	    PFlushA();
	}
    }
}

VOID FreeMMUMemory()
{
    int i;

    Forbid();
    for (i=0; i<FAST_last; i++)
    {
	if ( IsFree(FAST_table[i]) )
	{
	    FreeMem((APTR)(FAST_table[i]&0xFFFFFF00),256);
	}
    }
    for (i=0; i<CHIP_last; i++)
    {
	if ( IsFree(CHIP_table[i]) )
	{
	    FreeMem((APTR)(CHIP_table[i]&0xFFFFFF00),256);
	}
    }
    Permit();
}

#define MSG1  "Guardian-Angel removed\n"
#define MSG2  "Guardian-Angel installed\n"
#define MSG3  "Trouble allocating memory\n"
#define MSG4  "Guardian-Angel already installed\n"
#define MSG5  "Guardian-Angel not yet installed\n"
#define MSG6  "\2337mGuardian-Angel V1.8\2330m\n"
#define MSG7  "Copyright 1990 Commodore-Amiga, Inc.\n"
#define MSG8  "This utility will report illegal memory accesses to page zero,\n"
#define MSG9  "free memory, memory above 16 Megabytes, writes to ROM, and optionally\n"
#define MSG10 "check for tail trashing. (9600 baud serial output).\n"
#define MSG11 "\2337mUsage: Angel [?|On|Off|Quiet|Report|NoChip|Tail]\2330m\n"
#define MSG12 "MMU trapping turned off (AllocMem still SetFunction'ed though)\n"
#define MSG13 "MMU trapping turned back on\n"
#define MSG14 "MMU already in use\n"
#define MSG15 "Guardian-Angel can not be made quiet under SuperKickstart\n"
#define MSG16 "Layers library not found\n"
#define MSG17 "Layers library version is below 36.92\n"
#define MSG18 "Error: unrecognized keyword\n"

void Introduce()
{
    Delay(20);
    Write(_Backstdout, "\n", strlen("\n"));
    Write(_Backstdout, MSG6, strlen(MSG6));
    Write(_Backstdout, MSG7, strlen(MSG7));
    Write(_Backstdout, MSG8, strlen(MSG8));
    Write(_Backstdout, MSG9, strlen(MSG9));
    Write(_Backstdout, MSG10, strlen(MSG10));
}


void main(int argc, char *argv[])
{
    ULONG myCRP[2];
    int i;
    char vstring[]=VERSTAG;

    if (argc  == 0)          /* called from Workbench, can't do anything */
	_exit(0L);

    if ((argc >= 2 && argv[1][0] == '?') || (argc==1))
    {
	Introduce();
	Write(_Backstdout, MSG11, strlen(MSG11));
	Close(_Backstdout);
	_exit(0L);
    }

    if (argc > 1)
    {
	for (i = 1; i < argc; i++)
	{
	    if (striequ(argv[i],"NoChip"))		NoChip=TRUE;
	    else if (striequ(argv[i],"Fast"))		Fast=TRUE;
	    else if (striequ(argv[i],"Multitask"))	MultiTasking=TRUE;
	    else if (striequ(argv[i],"Multiprocess"))	MultiProcessing=TRUE;
	    else if (striequ(argv[i],"Off"))		Off=TRUE;
	    else if (striequ(argv[i],"On"))		On=TRUE;
	    else if (striequ(argv[i],"Scratch"))	Scratch=TRUE;
	    else if (striequ(argv[i],"NoLeak"))		NoLeak=TRUE;
	    else if (striequ(argv[i],"Quiet"))		Quiet=TRUE;
	    else if (striequ(argv[i],"Report"))		Report=TRUE;
	    else if (striequ(argv[i],"Tail"))		TailProtect=TRUE;
	    else
	    {
		Delay(20);
		Write(_Backstdout, MSG18, strlen(MSG18));
		Write(_Backstdout, MSG11, strlen(MSG11));
		Close(_Backstdout);
		_exit(10L);
	    }
	}
    }

    if ((port=FindPort("Guardian-Angel")) != NULL)  /* I should put a Forbid() here */
    {
	if (Off)
	{
	    /* Write(_Backstdout, MSG1, strlen(MSG1)); */
		Signal(port->mp_SigTask,1<<(port->mp_SigBit));
		Write(_Backstdout, MSG1, strlen(MSG1));
	}
	else if (Quiet)
	{
	    GetCRP(Super_CRP);
	    TableA = (ULONG *)(Super_CRP[1]&0xFFFFFFF0);

	    if ((TableA[62]&0xFFFFFF00) == 0x07F80000)
	    {
		    Write(_Backstdout, MSG15, strlen(MSG15));
	    }
	    else
	    {
		SetTC(TableTC&0x7FFFFFFF);
		Write(_Backstdout, MSG12, strlen(MSG12));
	    }
	}
	else if (Report)
	{
	    GetCRP(myCRP);
	    SetMMU(myCRP,TableTC);

	    /* SetTC(GetTC()|0x80000000); */
	    Write(_Backstdout, MSG13, strlen(MSG13));
	}
	else
	{
	    Write(_Backstdout, MSG4, strlen(MSG4));
	}
	Close(_Backstdout);
	_exit(0L);
    }
    else if (Off || Quiet || Report)
    {
	Write(_Backstdout, MSG5, strlen(MSG5));
	Close(_Backstdout);
	_exit(10L);
    }

    if (!On)
    {
	Write(_Backstdout, MSG11, strlen(MSG11));
	Close(_Backstdout);
	_exit(10L);
    }

    if (GetTC()&TC_ENB)
    {
	if (GetTC() != TC_SuperKickstart)
	{
	    Write(_Backstdout, MSG14, strlen(MSG14));
	    Close(_Backstdout);
	    _exit(10L);
	}
    }

    Introduce();

    if (resident = (struct Resident *)FindResident("layers.library"))
    {
	if(resident->rt_Version < 36) {
	    KPrintF(MSG17);
	    Write(_Backstdout, MSG17, strlen(MSG17));
	    Close(_Backstdout);
	    _exit(10L);
	}
	/* KPrintF("Layers resident tag found \n"); */
	layers_start = (ULONG)resident;
	/* layers_end = (ULONG)resident->rt_EndSkip; */
	layers_end = layers_start+16000;
	KPrintF("\nlayers_start: %lx   ",layers_start);
	KPrintF("layers_end:   %lx \n",layers_end);
    }
    else
    {
	KPrintF("Layers resident tag *not* found \n");
	Write(_Backstdout, MSG16, strlen(MSG16));
	Close(_Backstdout);
	_exit(10L);
    }

Forbid();
    Disable();
	vector = (ULONG *)0x04;			/* Cache Execbase */
	Base = vector[0];
	vector = (ULONG *)(GetVBR()+8);
	OldVector = vector[0];
	vector[0] = (ULONG)&NewBusError;	/* Set up vector to new handler */
    Enable();

    if (!CreateTable() || !(port = CreatePort("Guardian-Angel",0)) )
    {
	Permit();
	CXBRK();
	Write(_Backstdout, MSG3, strlen(MSG3));
	Close(_Backstdout);
	_exit(10L);
    }

    Disable();
	ForbidFree = FALSE;
	AllocMemPointer = (APTR)SetFunction(SysBase,(LONG)-198,(APTR)&NewAllocMem);
	AllocVecPointer = (APTR)SetFunction(SysBase,(LONG)-684,(APTR)&NewAllocVec);
	FreeMemPointer = (APTR)SetFunction(SysBase,(LONG)-210,(APTR)&NewFreeMem);
	FreeVecPointer = (APTR)SetFunction(SysBase,(LONG)-690,(APTR)&NewFreeVec);
	AllocAbsPointer = (APTR)SetFunction(SysBase,(LONG)-204,(APTR)&NewAllocAbs);
	AvailMemPointer = (APTR)SetFunction(SysBase,(LONG)-216,(APTR)&NewAvailMem);
	RawPutCharPointer = (APTR)SetFunction(SysBase,(LONG)-516,(APTR)&NewRawPutChar);
	PermitPointer = (APTR)SetFunction(SysBase,(LONG)-138,(APTR)&NewPermit);
	CacheClearU(-1);
    Enable();
Permit();

    Write(_Backstdout, MSG2, strlen(MSG2));
    Close(_Backstdout);
    Wait(1<<(port->mp_SigBit));		/* Wait until told to remove thyself */

    Disable();
	AllocMemPointer = (APTR)SetFunction(SysBase,(LONG)-198,AllocMemPointer);
	AllocVecPointer = (APTR)SetFunction(SysBase,(LONG)-684,AllocVecPointer);
	FreeMemPointer = (APTR)SetFunction(SysBase,(LONG)-210,FreeMemPointer);
	FreeVecPointer = (APTR)SetFunction(SysBase,(LONG)-690,FreeVecPointer);
	AllocAbsPointer = (APTR)SetFunction(SysBase,(LONG)-204,AllocAbsPointer);
	AvailMemPointer = (APTR)SetFunction(SysBase,(LONG)-216,AvailMemPointer);
	RawPutCharPointer = (APTR)SetFunction(SysBase,(LONG)-516,RawPutCharPointer);
	PermitPointer = (APTR)SetFunction(SysBase,(ULONG)-138,PermitPointer);
	DeletePort(port);
	DeleteTable();
	FreeMMUMemory();
	CXBRK();
	CacheClearU(-1);
    Enable();

KPrintF("Exiting Guardian-Angel \n");

}
