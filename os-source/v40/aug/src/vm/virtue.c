/***************************************************************************/
/* Virtual memory handler, Copyright Valentin Pepelea July 1989. Please do */
/* do not propagate, I will do that myself in due time.			   */
/*									   */
/* Usage: virtue RAMamount DeviceName DeviceUnit DiskOffset DiskEnd	   */
/*                 in K      ASCII     integer    in bytes   bytes	   */
/*									   */
/* to compile:	lc [-d3] [-w2] -b0 -v virtue.c				   */
/*		asm [-d] virtueasm.a					   */
/*		asm [-d] memroutines.a					   */
/*		blink with virtuelink.lnk				   */
/*									   */
/* This version has been written as a programming example, not for speed.  */
/* Some macros are also shown, commented out; they would speed up          */
/* processing speed somewhat, but it would be best to re-write it in       */
/* assembler.								   */
/***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/semaphores.h>
#include <devices/trackdisk.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

/***************************************************************************/
/* Here are the CRP definitions.  The CRP register is 64 bits long, but    */
/* only the first 32 bits are control bits, the next 32 bits provide the   */
/* base address of the table.						   */
/***************************************************************************/

#define	CRP_UPPER	(1L<<31)		 /* Upper/lower limit mode */
#define CRP_LIMIT(x)	((ULONG)((x)&0x7fff)<<16)/* Upper/lower limit value */
#define CRP_SG		(1L<<9)			 /* Indicates shared space */
#define CRP_DT_INVALID	0x00			 /* Invalid root descriptor */
#define	CRP_DT_PAGE	0x01			 /* Fixed offset */
#define CRP_DT_V4BYTE	0x02			 /* Short root descriptor */
#define	CRP_DT_V8BYTE	0x03			 /* Long root descriptor */

/***************************************************************************/
/* Here are the TC definitions.  The TC register is 32 bits long.	   */
/***************************************************************************/

#define	TC_ENB		(1L<<31)		/* Enable the MMU */
#define	TC_SRE		(1L<<25)		/* For separate Supervisor */
#define	TC_FCL		(1L<<24)		/* Use function codes? */
#define	TC_PS(x)	((ULONG)((x)&0x0f)<<20)	/* Page size */
#define TC_IS(x)	((ULONG)((x)&0x0f)<<16)	/* Logical shift */
#define	TC_TIA(x)	((ULONG)((x)&0x0f)<<12)	/* Table indices */
#define	TC_TIB(x)	((ULONG)((x)&0x0f)<<8)
#define TC_TIC(x)	((ULONG)((x)&0x0f)<<4)
#define	TC_TID(x)	((ULONG)((x)&0x0f)<<0)

/***************************************************************************/
/* Here are the page descriptor definitions, for short descriptors only,   */
/* since that's all I'm using at this point.				   */
/***************************************************************************/
   
#define	PD_ADDR(x)	((ULONG)(x)&~0x0f)	/* Translated Address */
#define	PD_WP		(1L<<2)			/* Write protect it! */
#define PD_DT_INVALID	0x00			/* Invalid root descriptor */
#define	PD_DT_PAGE	0x01			/* Fixed offset, auto-genned */
#define PD_DT_VALID	PD_DT_PAGE
#define PD_DT_V4BYTE	0x02			/* Short root descriptor */
#define	PD_DT_V8BYTE	0x03			/* Long root descriptor */
#define PD_USED		0x08
#define PD_MODIFIED	0x10
#define PD_LOCK		0x20
#define	PD_CACHE_INH	0x40
#define	PD_GATE		0x80

/***************************************************************************/

typedef unsigned short	PhysicalPage;	/* Physical pages are of this type */
typedef unsigned short	VirtualPage;	/* Virtual pages are of this type */
typedef unsigned char	AgeType;	/* Ages are of this type */

struct	MsgPort		*diskport = NULL;
struct	IOStdReq	*diskreq = NULL;
struct	SignalSemaphore	FaultSemaphore, MemorySemaphore;

AgeType *Age = NULL;
PhysicalPage limit, *Ptable = NULL;
ULONG *mmuVtable = NULL, *LittleTable = NULL, OldVector = NULL;

PhysicalPage ppages;		/* The number of physical pages in use */
VirtualPage vpages;		/* The number of virtual pages in use */
ULONG pbase = NULL;		/* Starting address of physical pages */
ULONG vbase = 0x02000000;	/* Starting address of virtual pages */
ULONG dbase;			/* The offset of the first sector */
ULONG pagesize;			/* The number of bytes in a page */
char *DeviceName;		/* Name of hard disk device driver */
ULONG DeviceUnit;		/* Unit number of device, look in mountlist */

	/* These are defined in the assembly module */

extern void Forbiden();		/* Test if multitasking forbiden */
extern void Disabled();		/* Test if interrupts disabled */
extern void BusError();		/* Entry of bus error handler */
extern void PFlush(ULONG);	/* Flush descriptor out of ATC */
extern void PFlushA();		/* Flush out entire ATC */
extern void SetTC();		/* Set the TC MMU register */
extern void SetCRP();		/* Set the CPR MMU register */

void AdvanceAge();
void AddQueue();
PhysicalPage GetQueue();
void SwapIn();
void SwapOut();
PhysicalPage GetFreePage();
void __interrupt PageFault();
static void *AllocAligned();
struct MsgPort *MakePort();
void CleanUp();

#define	MEMF_VIRTUAL	(1<<3)
#define TB_VMFRIENDLY	8
#define TF_VMFRIENDLY	(1<<8)
#define SIGB_VM		12
#define SIGF_VM		(1<<12)

#define SysBase (*((struct Library **) 4))
struct DosLibrary *DOSBase;

/***************************************************************************/
/* Here come the Queue operations and definitions. If a page becomes very  */
/* old, it is put in a queue. Thus searching for a free page upon a page   */
/* fault is not as onerous. We are implementing circular queues here.      */
/***************************************************************************/

#define MaxQueue 50 /* 50 */
#define MinFree 10  /* 10 */
#define MaxAge 20

struct Queue
    {
	int		head,tail;
	int		count;
	PhysicalPage	element[MaxQueue];
    }			/* the queue hold MaxQueue-1 entries */
			/* one entry gets sacrificed */
    queue;

void InitQueue()
{
    queue.head = MaxQueue-1;
    queue.tail = MaxQueue-1;
    queue.count = 0;
}

/* Make sure the queue is not full before calling */

void AddQueue(PhysicalPage element)
{
    queue.count++;    
    if (queue.head == (MaxQueue-1))	
	  queue.head=0;
	else
	  queue.head++;
    queue.element[queue.head] = element;
}

/* Make sure the queue is not empty before calling */

PhysicalPage GetQueue()
{
    PhysicalPage thispage;
    if (queue.tail == (MaxQueue-1))
	  queue.tail=0;
	else
	  queue.tail++;
    thispage = queue.element[queue.tail];
    queue.count--;
    return(thispage);
}

/***************************************************************************/
/* Given a physical page, this function returns the number of the virtual  */
/* page it stores.                                                         */
/***************************************************************************/

#define GetVPage(x)	( (VirtualPage)Ptable[(x)] )


/***************************************************************************/
/* Given a virtual page, this function returns the physical page number in */
/* which it is stored. It returns garbage if "vpage" is invalid.           */
/***************************************************************************/

#define GetPPage(x)	( (PhysicalPage)((mmuVtable[(x)]-pbase)/pagesize) )


/*************************************************************************/
/* This procedure invalidates a given virtual page without affecting its */
/* modified (M) or gate (G, previously modified) or any other bit.       */
/*************************************************************************/

#define Invalidate(x)  ( mmuVtable[(x)] &= 0xFFFFFFFC )


/***************************************************************************/
/* This procedure swaps in a given page.                                   */
/***************************************************************************/

void SwapIn(VirtualPage vpage, PhysicalPage ppage)
{
    diskreq->io_Length = pagesize;
    diskreq->io_Data = (APTR)((ULONG)ppage*pagesize+pbase);
    diskreq->io_Command = CMD_READ;
    diskreq->io_Offset = (ULONG)vpage*pagesize+dbase;
    DoIO(diskreq);

    if(diskreq->io_Error)
        Alert(0xA3000003,0L);
}

/***************************************************************************/
/* This procedure swaps out a given page.                                  */
/***************************************************************************/

void SwapOut(VirtualPage vpage, PhysicalPage ppage)
{
    diskreq->io_Length = pagesize;
    diskreq->io_Data = (APTR)((ULONG)ppage*pagesize+pbase);
    diskreq->io_Command = CMD_WRITE;
    diskreq->io_Offset = (ULONG)vpage*pagesize+dbase;
    DoIO(diskreq);

    if(diskreq->io_Error)
        Alert(0xA3000003,0L);
}

/***************************************************************************/
/* This functions tells us wether a given virtual page is valid or not.    */
/***************************************************************************/

#define Valid(x)	( ((mmuVtable[(x)]&0x03)==PD_DT_PAGE) )


/***************************************************************************/
/* This function tells us wether a given virtual page has been modified   */
/* since the last time it was saved to disk.                               */
/***************************************************************************/

#define	Modified(x)	( ((mmuVtable[(x)]&(PD_MODIFIED|PD_GATE)) >= PD_MODIFIED) )


/***************************************************************************/
/* This function returns a free page from the queue if possible, or steals */
/* one if not.                                                             */
/***************************************************************************/

PhysicalPage GetFreePage()
{
    while(queue.count==0)
	AdvanceAge();
    return(GetQueue());
}

/***************************************************************************/
/* This routine returns the page number where the page fault occured.      */
/***************************************************************************/

#define GetPage(x)	( (VirtualPage)(((x)-vbase)/pagesize) )


/***************************************************************************/
/* This procedure advances the age of a page that has not been used, by    */
/* one. If it has been used, its counter is reduced to 64, if it has been  */
/* modified, then to 0. If it reached age 255, it is added to the queue of */
/* free pages but without being invalidated nor swapped out.               */
/***************************************************************************/

void AdvanceAge()
{
    PhysicalPage i,j;
    VirtualPage vpage;
    for (i=1; i<ppages; i++)
	{
	    vpage = GetVPage(i);

	    switch (mmuVtable[vpage]&(PD_DT_VALID|PD_USED|PD_MODIFIED))
		{
		    case PD_DT_VALID:		/* Not used, and valid? */
			Age[i]++;
			if ((Age[i]>MaxAge))
			    {
			    if (queue.count < (MaxQueue)) 
				{
				    AddQueue(i);
				    Invalidate(vpage);
				    PFlush(((ULONG)vpage*pagesize)
					     +vbase);
				    if (Modified(vpage))
					SwapOut(vpage,i);
				    Age[i] = (AgeType)0;
				}
			    }
			break;
						/* Modified, used & valid? */

		    case (PD_DT_VALID|PD_USED|PD_MODIFIED):
			Age[i]=(AgeType)2;
			mmuVtable[vpage]=(mmuVtable[vpage]&0xFFFFFFE7)|PD_GATE;
			break;

		    case (PD_DT_VALID|PD_USED):		/* Used, and valid? */
			Age[i]=min((AgeType)8,Age[i]);
			mmuVtable[vpage]=mmuVtable[vpage]&0xFFFFFFF7;
			break;
		}				/* Do nothing if invalid */

	}
    PFlushA();					/* Flush out entire ATC */
}

/***************************************************************************/
/* After the page fault has occured, the assembler routines will transfer  */
/* the exception stack frame onto the user stack, go into user mode, and   */
/* put the address of the fault on the stack. Then this routine gets       */
/* called.                                                                 */
/***************************************************************************/

void __interrupt PageFault(ULONG faultaddress)
{
    PhysicalPage	ppage;
    VirtualPage		vpage;
    ULONG		error;

    Forbiden();				/* Guru if multitasking forbidden */
    Disabled();				/* Guru if interrupts disabled    */

    ObtainSemaphore(&FaultSemaphore);
    vpage = GetPage(faultaddress);	/* On what page did fault occur?  */

/*	KPrintF("Page fault: %d\n",vpage);
	Delay(500);
	KPrintF("Continuing...\n");
*/
    if (!Valid(vpage))			/* Has some other task solved the */
	{				/* page fault previously?	  */

/*	    if (!(diskport = CreatePort(0L,0L)))
		Alert(0xA3000000,0L);
	    if (!(diskreq = CreateStdIO(diskport)))
		Alert(0xA3000001,0L);
	    if (error = OpenDevice(DeviceName,DeviceUnit,diskreq,0L))
		Alert(0xA3000002,0L);
*/
	    diskport->mp_SigTask = FindTask(0L); /* Transfer ownership */

	    ppage=GetFreePage();	/* Give me a free physical page   */
	    SwapIn(vpage,ppage);	/* Get it from the hard disk      */

	    mmuVtable[vpage] =	((ULONG)ppage*pagesize+pbase) | PD_DT_PAGE;
	    Ptable[ppage] =	vpage;
	    Age[ppage] =	(AgeType)8;

	    /* Insert here spurious update, least recently used algorithm */

/*	    CloseDevice(diskreq);
	    DeleteStdIO(diskreq);
	    DeletePort(diskport);
*/
	}
    ReleaseSemaphore(&FaultSemaphore);
}

/*************************************************************************/
/* This procedure allocates a memory block aligned as we want it. Author */
/* is from Dave Haynie. When using <proto/exec.h> remember that there is */
/* a bug in it, AllocAbs should be (void *), not just (void).            */
/*************************************************************************/

static void *AllocAligned(ULONG size, ULONG bound)
{
   void *mem, *aligned;

   if (!(mem = (void *)AllocMem(size+bound,NULL))) return NULL;
   Forbid();
   aligned = (void *)(((ULONG)mem + bound - 1) & ~(bound - 1));
   FreeMem(mem,size+bound);
   mem = (void *)AllocAbs(size,aligned);
   Permit();
   return mem;
}

/***************************************************************************/
/* CleanUp will deallocate all resources. Make sure that when declared,    */
/* pointer are initialized to zero, otherwise a bogus deallocation will    */
/* occur. (Guru #81000009 = free twice)                                    */
/***************************************************************************/

void CleanUp(ULONG err)
{
    if (pbase)		FreeMem((APTR)pbase,ppages*pagesize);
    if (Age)		FreeMem(Age,ppages);
    if (LittleTable)	FreeMem(LittleTable,16);
    if (Ptable)		FreeMem(Ptable,ppages*2);
    if (mmuVtable)	FreeMem(mmuVtable,limit);
    exit(err);
}

/***************************************************************************/
/* I do not like allocating structures from assembler, (too system         */
/* specific) so I use C stubs.                                             */
/***************************************************************************/

void Obtain()
{
    ObtainSemaphore(&MemorySemaphore);
}

void Release()
{
    ReleaseSemaphore(&MemorySemaphore);
}

/***************************************************************************/
/* My personal version of CreatePort(). With a little twist. No signal     */
/* gets furiously allocated.                                               */
/***************************************************************************/

struct MsgPort *MakePort(char *name, long pri)
{
    struct MsgPort *port;

    port = (struct MsgPort *)
      AllocMem((ULONG)sizeof(struct MsgPort),(ULONG)MEMF_CLEAR|MEMF_PUBLIC);
    if (!port) return(NULL);

    port->mp_Node.ln_Name = name;
    port->mp_Node.ln_Pri = pri;
    port->mp_Node.ln_Type = NT_MSGPORT;
    port->mp_Flags = PA_SIGNAL;
    port->mp_SigBit = SIGB_VM;
    port->mp_SigTask = (struct Task *)FindTask(0L);

    if (name)
	AddPort(port);
	else
	NewList(&(port->mp_MsgList));

    return(port);
}

/***************************************************************************/
/* Here is the main() of the program. All it does is set up the MMU        */
/* translation tables, and then updates the age of the virtual pages which */
/* are loaded in memory every 10 seconds or so.                            */
/***************************************************************************/ 

void main(argc, argv)
long int argc;
char *argv[];
{
    PhysicalPage i;
    VirtualPage j;
    ULONG myTC, myCRP[2], *vector, dend, error, *here;

    /* Get the parameters */

    DOSBase = (struct DosLibrary *)OpenLibrary("dos.library",NULL);

    pagesize = 512;
    limit = (PhysicalPage)(0x01000000/pagesize);

    ppages = atoi(argv[1])*1024/pagesize;
    DeviceName = argv[2];
    DeviceUnit = atoi(argv[3]);
    dbase = atoi(argv[4]);
    dend = atoi(argv[5]);
    vpages = (dend-dbase)/pagesize;

    /* Allocate the memory blocks */

    if( !(mmuVtable = AllocAligned(limit*4,16)) )
	CleanUp(10);
    if( !(Ptable = AllocMem(ppages*2,MEMF_PUBLIC)) )
	CleanUp(11);
    if( !(LittleTable = AllocAligned(16,16)) )
	CleanUp(12);
    if( !(Age = AllocMem(ppages,MEMF_PUBLIC)) )
	CleanUp(13);
    if( !(pbase = (ULONG)AllocAligned(ppages*pagesize,pagesize)) )
	CleanUp(14);

    InitQueue();
    FaultSemaphore.ss_Link.ln_Name = "PageFault";
    FaultSemaphore.ss_Link.ln_Pri = 0;
    InitSemaphore(&FaultSemaphore);	/* No need to add it to the system. */
    MemorySemaphore.ss_Link.ln_Name = "MemoryList";
    MemorySemaphore.ss_Link.ln_Pri = 0;
    InitSemaphore(&MemorySemaphore);	/* AddSemaphore is buggy */
    MemRoutines();			/* Substitue my own memory routines */
					/* like AllocMem, FreeMem, etc... */

    /* Initialize the translation table */

    for (i = 0; i < ppages; i++)
	{
	    mmuVtable[i] = ((ULONG)i*pagesize+pbase)|PD_DT_PAGE;
	    Ptable[i] = (PhysicalPage)i;
	    Age[i] = (AgeType)8;
	}
    for (i = ppages; i < (PhysicalPage)vpages; i++)
	{
	    mmuVtable[i] = (ULONG)PD_DT_INVALID;
	}
    for (i = (PhysicalPage)vpages; i < limit; i++)
	{
	    mmuVtable[i] = PD_WP|PD_DT_PAGE;
	}
    LittleTable[0] = 0L | PD_DT_PAGE;
    LittleTable[1] = 0x01000000 | PD_DT_PAGE;
    LittleTable[2] = ((ULONG)mmuVtable)| PD_DT_V4BYTE;
    LittleTable[3] = 0x03000000 | PD_DT_PAGE;

    /* Initialize the CRP pointer. */

    myCRP[0] = 0x80000000 | CRP_SG | CRP_DT_V4BYTE;
    myCRP[1] = (ULONG)LittleTable;
    SetCRP(myCRP);

    /* Initialize TC register. Warning, use 512 page size only! (for now) */

    myTC = TC_ENB | TC_PS(0x09) | TC_IS(0x06) | 
	   TC_TIA(0x2) | TC_TIB(0xF) | TC_TIC(0x0) | TC_TID(0x0);
    SetTC(myTC);

    /* Make Bus Error autovector point to our assembler handler */

    vector = (ULONG *)0x00000008;
    OldVector = vector[0];		/* Store old bus error vector	*/
    vector[0] = (ULONG)&BusError;	/* Make new bus error vector	*/

/*    AddMemList(vpages*pagesize,MEMF_FAST|MEMF_VIRTUAL,-5L,(APTR)vbase,NULL); */


/*	for(here=(ULONG *)0x2000000; here<(ULONG *)0x2080000;here=(here+128))
	    {
		PageFault(here);
		j = GetPage(here);
		mmuVtable[j] |= (PD_MODIFIED | PD_USED);
	    }
*/

    if (!(diskport = MakePort(0L,0L)))
	Alert(0xA3000000,0L);
    if (!(diskreq = CreateStdIO(diskport)))
	Alert(0xA3000001,0L);
    if (error = OpenDevice(DeviceName,DeviceUnit,diskreq,0L))
	Alert(0xA3000002,0L);

/* Insert here, periodic update LRU algorithm */

    while(TRUE)
    {
	Delay(15);		/* Execute three times per second */

	if(queue.count < MinFree)
	{
	    ObtainSemaphore(&FaultSemaphore);
		diskport->mp_SigTask = FindTask(0L);  /* Transfer ownership */
		if ((queue.count < MinFree) && (queue.count < (ppages-1)))
		{
		    while((queue.count < MaxQueue) && (queue.count < (ppages-1)))
			AdvanceAge();
		}
	    ReleaseSemaphore(&FaultSemaphore);
	}
    }

    CloseDevice(diskreq);
    DeleteStdIO(diskreq);
    DeletePort(diskport);

}
