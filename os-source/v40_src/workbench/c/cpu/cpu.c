/*
 * $Id: cpu.c,v 38.1 91/06/24 14:52:07 mks Exp $
 *
 * $Log:	cpu.c,v $
 * Revision 38.1  91/06/24  14:52:07  mks
 * New in V38 source tree
 * Added support for CACRF_CopyBack and CACRF_EnableE
 * 
 */

#include <exec/types.h>
#include <exec/execbase.h>
#include <internal/commands.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include "cpu_rev.h"

#define TEMPLATE "CACHE/S,BURST/S,NOCACHE/S,NOBURST/S,DATACACHE/S,\
DATABURST/S,NODATACACHE/S,NODATABURST/S,INSTCACHE/S,INSTBURST/S,\
NOINSTCACHE/S,NOINSTBURST/S,COPYBACK/S,NOCOPYBACK/S,EXTERNALCACHE/S,\
NOEXTERNALCACHE/S,FASTROM/S,NOFASTROM/S,TRAP/S,NOTRAP/S,\
NOMMUTEST/S,CHECK/K" CMDREV

#define OPT_CACHE	0
#define OPT_BURST	1
#define OPT_NOCACHE	2
#define OPT_NOBURST	3
#define OPT_DATACACHE	4
#define OPT_DATABURST	5
#define OPT_NODATACACHE	6
#define OPT_NODATABURST	7
#define OPT_INSTCACHE	8
#define OPT_INSTBURST	9
#define OPT_NOINSTCACHE	10
#define OPT_NOINSTBURST	11
#define	OPT_COPYBACK	12
#define	OPT_NOCOPYBACK	13
#define	OPT_EXTERNAL	14
#define	OPT_NOEXTERNAL	15

#define OPT_FASTROM	16
#define OPT_NOFASTROM	17
#define OPT_TRAP	18
#define OPT_NOTRAP	19
#define OPT_NOMMUTEST	20
#define OPT_CHECK	21
#define OPT_COUNT	22

#define	CRP_UPPER	(1L<<31)		/* Upper/lower limit mode */
#define CRP_LIMIT(x)	((ULONG)((x)&0x7fff)<<16)/* Upper/lower limit value */
#define CRP_SG		(1L<<9)			/* Indicates shared space */
#define CRP_DT_INVALID	0x00			/* Invalid root descriptor */
#define	CRP_DT_PAGE	0x01			/* Fixed offset, auto-genned */
#define CRP_DT_V4BYTE	0x02			/* Short root descriptor */
#define	CRP_DT_V8BYTE	0x03			/* Long root descriptor */

#define	TC_ENB		(1L<<31)		/* Enable the MMU */
#define	TC_SRE		(1L<<25)		/* For separate Supervisor */
#define	TC_FCL		(1L<<24)		/* Use function codes? */
#define	TC_PS(x)	((ULONG)((x)&0x0f)<<20)	/* Page size */
#define TC_IS(x)	((ULONG)((x)&0x0f)<<16)	/* Logical shift */
#define	TC_TIA(x)	((ULONG)((x)&0x0f)<<12)	/* Table indices */
#define	TC_TIB(x)	((ULONG)((x)&0x0f)<<8)
#define TC_TIC(x)	((ULONG)((x)&0x0f)<<4)
#define	TC_TID(x)	((ULONG)((x)&0x0f)<<0)

#define	PD_WP		(1L<<2)			/* Write protect it! */
#define PD_DT_INVALID	0x00			/* Invalid root descriptor */
#define	PD_DT_PAGE	0x01			/* Fixed offset, auto-genned */
#define PD_DT_V4BYTE	0x02			/* Short root descriptor */
#define	PD_DT_V8BYTE	0x03			/* Long root descriptor */

extern void __stdargs SetCACR(ULONG);
extern void __stdargs GetCRP(ULONG *);
extern void __stdargs SetCRP(ULONG *);
extern void __stdargs SetTC(ULONG);
extern void __stdargs SetVBR(ULONG);
extern void __stdargs SetSRP(ULONG *);
extern ULONG __stdargs GetCACR(void);
extern ULONG __stdargs GetTC(void);
extern ULONG __stdargs GetVBR(void);
extern ULONG __stdargs GetMMUType(void);

static void *AllocAligned(ULONG size, ULONG bound);
static BOOL CreateFastROM(void);
static BOOL CreateTrap(void);
static void DeleteFastROM(void);
static void MoveVBR(void);

#define SysBase (*((struct Library **) 4))

#define TrapTC (TC_ENB|TC_PS(8)|TC_IS(1)|TC_TIA(7)|TC_TIB(8)|TC_TIC(8)|TC_TID(0))
#define FastTC (TC_ENB|TC_PS(0x0F)|TC_IS(1)|TC_TIA(7)|TC_TIB(5)|TC_TIC(4)|TC_TID(0))

/*----------------------------------------------------------------------*/
/* Structure used by the pattern matching routines                      */
/*----------------------------------------------------------------------*/

struct uAnchor
{
   struct AnchorPath uA_AP;
   BYTE   uA_Path[256];
};

int main(void)
{
   long               opts[OPT_COUNT];
   struct DosLibrary *DOSBase;
   struct RDargs     *rdargs;
   int                rc;
   int                code;
   char              *curarg;    /* Current /M argument being processed */
   ULONG              mmu, cacr, tc, *a, *b;
   rc = RETURN_OK;

   if (DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB,DOSVER))
   {
      memset((char *)opts, 0, sizeof(opts));
      rdargs = ReadArgs(TEMPLATE, opts, NULL);

      if (rdargs == NULL)
      {
         PrintFault(IoErr(), NULL);
      }
      else
      {
         cacr = CacheControl(0L,0L);

         if (opts[OPT_NOCOPYBACK]) cacr &= ~CACRF_CopyBack;
         else if (opts[OPT_COPYBACK]) cacr |= CACRF_CopyBack;
         if (opts[OPT_NOEXTERNAL]) cacr &= ~CACRF_EnableE;
         else if (opts[OPT_EXTERNAL]) cacr |= CACRF_EnableE;

         if (opts[OPT_NODATACACHE]) cacr &= ~CACRF_EnableD;
         else if (opts[OPT_DATACACHE]) cacr |= CACRF_EnableD;
         if (opts[OPT_NODATABURST]) cacr &= ~CACRF_DBE;
         else if (opts[OPT_DATABURST]) cacr |= CACRF_DBE;

         if (opts[OPT_NOINSTCACHE]) cacr &= ~CACRF_EnableI;
         else if (opts[OPT_INSTCACHE]) cacr |= CACRF_EnableI;
         if (opts[OPT_NOINSTBURST]) cacr &= ~CACRF_IBE;
         else if (opts[OPT_INSTBURST]) cacr |= CACRF_IBE;

         if (opts[OPT_NOCACHE]) cacr &= ~(CACRF_EnableD | CACRF_EnableI);
         else if (opts[OPT_CACHE]) cacr |= CACRF_EnableD | CACRF_EnableI;
         if (opts[OPT_NOBURST]) cacr &= ~(CACRF_DBE | CACRF_IBE);
         else if (opts[OPT_BURST]) cacr |= CACRF_DBE | CACRF_IBE;

         CacheControl((cacr|CACRF_WriteAllocate),0xFFFFFFFF);

         if (!opts[OPT_NOMMUTEST] && (mmu = GetMMUType()))
         {
            if ((tc = GetTC()) & TC_ENB)
            {
               if (opts[OPT_NOFASTROM] && (tc==FastTC)) DeleteFastROM();
               else if (opts[OPT_NOTRAP] && (tc==TrapTC))
               {
                  DeleteFastROM();
                  a = (ULONG *)(GetVBR()+64);
                  b = (ULONG *)(GetVBR()+8);
                  if ( (a[0]&0xFFFF0000) != (b[0]&0xFFFF0000))
                  {
                     if (System("devs:trap-handler",NULL) != 0)
                     {
                        PutStr("Warning: could not remove trap handler. MMU off.\n");
                        rc = RETURN_WARN;
                     }
                  }
               }
            }
            else
            {
               if (opts[OPT_FASTROM] && !CreateFastROM())
               {
                  PutStr("Warning: can't get memory for FASTROM\n");
                  rc = RETURN_WARN;
               }
               if (opts[OPT_TRAP] && !opts[OPT_FASTROM])
               {
                  a = (ULONG *)(GetVBR()+64);
                  b = (ULONG *)(GetVBR()+8);
                  if ( (a[0]&0xFFFF0000) == (b[0]&0xFFFF0000))
                  {
                     if (System("devs:trap-handler",NULL) != 0)
                     {
                        PutStr("Error: no bus error handler installed\n");
                        FreeArgs(rdargs);
                        CloseLibrary((struct Library *)DOSBase);
                        return(RETURN_FAIL);
                     }
                  }
                  if (!CreateTrap())
                  {
                     PutStr("Warning: can't get memory for TRAP\n");
                     rc = RETURN_WARN;
                  }
               }
            }
         }

         PutStr("System: ");

         if (EXECBASE->AttnFlags&AFF_68040) { PutStr("68040 "); }
         else if (EXECBASE->AttnFlags&AFF_68030) { PutStr("68030 "); }
         else if (EXECBASE->AttnFlags&AFF_68020) { PutStr("68020 "); }
         else if (EXECBASE->AttnFlags&AFF_68010) { PutStr("68010 "); }
         else PutStr("68000 ");

         if (EXECBASE->AttnFlags&AFF_68882) { PutStr("68882 "); }
         else if (EXECBASE->AttnFlags&AFF_68881) PutStr("68881 ");

         if (mmu == 68851L)
         {
            PutStr("68851 ");
         }

         if (mmu && ((tc = GetTC()) & TC_ENB))
         {
            if (tc & TC_PS(0x07))
               PutStr("FastROM ");
            else PutStr("Trap ");
         }

         /*--------------------------------------------------------------*/
         /* We better read the CACR register again, because our previous */
         /* write to it might not have succeded.                         */
         /*--------------------------------------------------------------*/

         cacr = CacheControl(0L,0L);

         PutStr("(INST: ");
         if (!(cacr & CACRF_EnableI))  PutStr("No");
         PutStr("Cache");

         if (EXECBASE->AttnFlags&AFF_68030 || EXECBASE->AttnFlags&AFF_68040)
         {
            PutStr(" ");
            if (!(cacr & CACRF_IBE))
               PutStr("No");
            PutStr("Burst) (DATA: ");
            if (!(cacr & CACRF_EnableD))
               PutStr("NoCache ");
            else
               PutStr("Cache ");

            if (cacr & CACRF_CopyBack) PutStr("CopyBack");
            else
            {
               if (!(cacr & CACRF_DBE))
                  PutStr("No");
               PutStr("Burst");
            }
         }
         PutStr(")");

         if (cacr & CACRF_EnableE) PutStr(" (External Cache)");
         PutStr("\n");

         /*-------------------------------------------------------------*/
         /* This is where we search out the CHECK parameters. If any of */
         /* the parameters (CPU's) do not match, we set the return code */
         /* to RETURN_WARN.                                             */
         /*-------------------------------------------------------------*/

         code = 0x00;
         if (curarg = (char *)opts[OPT_CHECK])
         {
            if (!strcmpi(curarg,"68010"))
                     code |= AFF_68010;
            else if (!strcmpi(curarg,"68020"))
                     code |= AFF_68020;
            else if (!strcmpi(curarg,"68030"))
                     code |= AFF_68030;
            else if (!strcmpi(curarg,"68040"))
                     code |= AFF_68040;
            else if (!strcmpi(curarg,"68881"))
                     code |= AFF_68881;
            else if (!strcmpi(curarg,"68882"))
                     code |= AFF_68882;
            else if (!strcmpi(curarg,"mmu"))
                     if (mmu == 0) rc = RETURN_WARN;
                     else code |= AFF_68010;  /* There must be a >68000 */
            else if (!strcmpi(curarg,"fpu"))
                     code |= AFF_68881;
            else VPrintf("Warning: invalid %s keyword\n",(long *)&curarg);
         }
         if (!(code&EXECBASE->AttnFlags))
            rc = RETURN_WARN;
         FreeArgs(rdargs);
      }
      CloseLibrary((struct Library *)DOSBase);
   }
   else
   {
      rc = RETURN_FAIL;
      OPENFAIL;
   }
   return(rc);
}

/*************************************************************************/
/* Here's the MMU support stuff.                                         */
/*************************************************************************/

/* #define ROMBASE		0x00F80000L */
#define ROMSIZE		0x00080000L

static void *AllocAligned(ULONG size, ULONG bound)
{
   void *mem, *aligned;

   if (!(mem = AllocMem(size+bound,0L))) return NULL;
   Forbid();
   aligned = (void *)(((ULONG)mem + bound - 1) & ~(bound - 1));
   FreeMem(mem,size+bound);
   mem = AllocAbs(size,aligned);
   Permit();
   return mem;
}

BOOL CreateFastROM()
{
   ULONG i, round, myCRP[2], myTC, temp;
   ULONG *Autovector, *ROMbase, *ROM32;
   ULONG *TableA, *TableB;

   round = 1024*32;
   ROM32 = AllocAligned(ROMSIZE,round);
   TableA = AllocAligned(128*4,16);
   TableB = AllocAligned(32*4,16);

   if (!ROM32 || !TableA || !TableB)
   {
      if (ROM32)  FreeMem(ROM32,ROMSIZE);
      if (TableA) FreeMem(TableA,128*4);
      if (TableB) FreeMem(TableB,32*4);
      return FALSE;
   }

   /***********************************************************************/
   /* If the Exec has not moved the vector table away from page zero yet, */
   /* then I better do it myself.                                         */
   /***********************************************************************/

   MoveVBR();

   Autovector = (ULONG *)(GetVBR()+64);		   /* Get address of autovectors */
   ROMbase = (ULONG *)(Autovector[0]&0xFFFF0000);  /* assumes ROM is aligned */

   CopyMemQuick(ROMbase,ROM32,ROMSIZE);

   /***********************************************************************/
   /* Let's set up the translation tables. Easy! Well...                  */
   /***********************************************************************/

   for (i = 0; i < 31; i++)
   {
      temp = (ULONG)(1024*512*i);
      if (temp == (ULONG)ROMbase) TableB[i] = (ULONG)ROM32 | PD_DT_PAGE;
      else TableB[i] = temp | PD_DT_PAGE;
   }
   TableB[31] = (ULONG)ROM32 | PD_DT_PAGE;	/* Why not? */

   myTC = FastTC;

   TableA[0] = (ULONG)TableB | PD_DT_V4BYTE;
   for (i=1; i<128; i++) TableA[i] = (0x1000000 * i) | PD_DT_PAGE;

   myCRP[0] = 0x80000000 | CRP_SG | CRP_DT_V4BYTE;
   myCRP[1] = (ULONG)TableA;

   /***********************************************************************/
   /* Not only should the following be mutually excluded by grabing the   */
   /* MMU semaphore, but the entire translation table process should be   */
   /* too. So let's just pray nobody else is mucking with the MMU         */
   /* meanwhile. I'll fix this one day.                                   */
   /***********************************************************************/

   SetCRP(myCRP);
   SetTC(myTC);

   return(TRUE);
}

BOOL CreateTrap()
{
   ULONG i, myCRP[2], myTC;
   ULONG *TableA, *TableB, *TableC;

   TableA = AllocAligned(128*4,16);
   TableB = AllocAligned(256*4,16);
   TableC = AllocAligned(256*4,16);

   if (!TableA || !TableB || !TableC)
   {
      if (TableA) FreeMem(TableA,128*4);
      if (TableB) FreeMem(TableB,256*4);
      if (TableC) FreeMem(TableC,256*4);
      return FALSE;
   }

   /***********************************************************************/
   /* If the Exec has not moved the vector table away from page zero yet, */
   /* then I better do it myself.                                         */
   /***********************************************************************/

   MoveVBR();

   /***********************************************************************/
   /* Let's set up the translation tables. Easy! Well...                  */
   /***********************************************************************/


   TableB[0] = (ULONG)TableC | PD_DT_V4BYTE;
   for (i=1; i<248; i++) TableB[i] = (65536 * i) | PD_DT_PAGE;
   for (i=248; i<256; i++) TableB[i] = (65536 * i) | PD_WP | PD_DT_PAGE;

   TableC[0] = 0L | PD_DT_INVALID;
   for (i=1; i<256; i++) TableC[i] = (256 * i) | PD_DT_PAGE;

   myTC = TrapTC;

   TableA[0] = (ULONG)TableB | PD_DT_V4BYTE;
   for (i=1; i<64; i++) TableA[i] = (0x1000000 * i) | PD_DT_PAGE;
   for (i=64; i<128; i++) TableA[i] = PD_DT_INVALID;

   myCRP[0] = 0x80000000 | CRP_SG | CRP_DT_V4BYTE;
   myCRP[1] = (ULONG)TableA;

   /***********************************************************************/
   /* Not only should the following be mutually excluded by grabing the   */
   /* MMU semaphore, but the entire translation table process should be   */
   /* too. So let's just pray nobody else is mucking with the MMU         */
   /* meanwhile. I'll fix this one day.                                   */
   /***********************************************************************/

   SetCRP(myCRP);
   SetTC(myTC);

   return(TRUE);
}

void DeleteFastROM()
{
   ULONG myCRP[2], flag, *ROM32;
   ULONG *TableA, *TableB, *TableC;

   SetTC(0L);
   GetCRP(myCRP);
   TableA = (ULONG *)myCRP[1];
   TableB = (ULONG *)(TableA[0]&0xFFFFFFF0);
   TableC = (ULONG *)(TableB[0]&0xFFFFFFF0);

   flag = (ULONG)TableC&0xFFFFFF00;
   if (flag)
   {
      FreeMem(TableB,256*4);
      FreeMem(TableC,256*4);
   }
   else
   {
      ROM32 = (ULONG *)(TableB[31]&0xFFFFFF00);
      FreeMem(TableB,32*4);
      FreeMem(ROM32,ROMSIZE);
   }
   FreeMem(TableA,128*4);
}

void MoveVBR()
{
   ULONG oldVBR, newVBR;

   if (!(oldVBR = (ULONG)GetVBR()))
   {
      if (newVBR = (ULONG)AllocMem(1024,MEMF_PUBLIC))
      {
         CopyMemQuick((ULONG *)oldVBR,(ULONG *)newVBR,1024);
         SetVBR(newVBR);
      }
   }
}

