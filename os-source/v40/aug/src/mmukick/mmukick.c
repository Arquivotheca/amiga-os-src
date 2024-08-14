#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos.h>
#include "execbase.h"

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

/* #define ROMBASE		0x00F80000L */

#define ROMSIZE		0x00080000L

/* #define Reset		__emit 0x4e70 */

extern void __stdargs SetCACR(ULONG);
extern void __stdargs GetCRP(ULONG *);
extern void __stdargs SetCRP(ULONG *);
extern void __stdargs SetTC(ULONG);
extern void __stdargs SetVBR(ULONG);
extern ULONG __stdargs GetCACR(void);
extern ULONG __stdargs GetTC(void);
extern ULONG __stdargs GetVBR(void);
extern ULONG __stdargs GetMMUType(void);

static void *AllocAligned(ULONG size, ULONG bound);
static void *AllocAlignedChip(ULONG size, ULONG bound);
BOOL CreateKickChipRAM(int argc, char *argv[]);
BOOL CreateKickRAM(int argc, char *argv[]);
BOOL CreateTable(long);

int main(int argc, char *argv[])
{
   ULONG	mmu, tc, crp[2], avail;
   ULONG	*ROM32 = NULL, MMUtable = NULL;
   long		temp;

         if (mmu = GetMMUType())
         {
            if ((tc = GetTC()) & TC_ENB)
            {
               printf("MMU was set up \n");
               if (!( ROM32 = (ULONG *)AllocAligned(ROMSIZE+1024,1024*32) ))
               {
                  printf("Error: Could not reallocate the ROM image! \n");
                  printf("Error: System is corrupt - likely to scram! \n");
                  exit(10);
               }
               else
               {
                  CopyMemQuick((ULONG *)0xF00000,ROM32,0x80000);
                  if (!CreateTable((long)ROM32))
                  {
                     printf("Error: unable to recreate translation table! \n");
                  }
               }
            }
            else
            {
               avail = AvailMem(MEMF_CHIP);
               if (avail > 0x80000)
               {
                  if (!CreateKickChipRAM(argc,argv))
                  {
                     printf("Error: Can't get memory for KickChipRAM\n");
                     exit(10);
                  }
               }
               else
               {
                  if (!CreateKickRAM(argc,argv))
                  {
                     printf("Error: Can't get memory for KickRAM\n");
                     exit(10);
                  }
               }                  
            }
         }
         else
         {
            printf("Error: Where did the MMU go? \n");
         }
}

/*************************************************************************/
/* Here's the MMU support stuff.                                         */
/*************************************************************************/

static void *AllocAligned(ULONG size, ULONG bound)
{
   void *mem, *aligned;

   if (!(mem = (void *)AllocMem(size+bound,0L))) return NULL;   
   Forbid();
   aligned = (void *)(((ULONG)mem + bound - 1) & ~(bound - 1));
   FreeMem(mem,size+bound);
   mem = (void *)AllocAbs(size,aligned);
   Permit();
   return mem;
}

static void *AllocAlignedChip(ULONG size, ULONG bound)
{
   void *mem, *aligned;

   if (!(mem = (void *)AllocMem(size+bound,MEMF_CHIP))) return NULL;   
   Forbid();
   aligned = (void *)(((ULONG)mem + bound - 1) & ~(bound - 1));
   FreeMem(mem,size+bound);
   mem = (void *)AllocAbs(size,aligned);
   Permit();
   return mem;
}

BOOL CreateKickChipRAM(int argc, char *argv[])
{
   ULONG i, myCRP[2], myTC, temp;
   ULONG *Autovector, *ROMbase, *ROM32 = NULL, *CHIPROM = NULL;
   ULONG *TableA = NULL, *TableB = NULL, *TableC = NULL;
   long  actual;
   struct FileHandle *file;

   CHIPROM = (ULONG *)AllocAbs(ROMSIZE+1024,0x30000);
   printf("CHIPROM: %lx   AllocSize: %lx \n",CHIPROM,ROMSIZE+1024);

   if (!CHIPROM)
   {
      return(FALSE);
   }

   /* TableA = AllocAligned(128*4,16); */
   /* TableB = AllocAligned(32*4,16);  */

   TableA = (ULONG *)((ULONG)CHIPROM+ROMSIZE);
   TableB = (ULONG *)((ULONG)TableA + 512);	/* + (128*4); */
   TableC = (ULONG *)((ULONG)TableB + 128);	/* + (32*4); */

   printf("TableA: %lx  TableB: %lx  TableC: %lx \n",TableA,TableB,TableC);

   Autovector = (ULONG *)(GetVBR()+64);			/* Get address of autovectors */
   ROMbase = (ULONG *)(Autovector[0]&0xFFFF0000);	/* assumes ROM is aligned */

   printf("ROMbase: %lx \n",ROMbase);

   file = (struct FileHandle *)Open(argv[1],MODE_OLDFILE);
   if (!file)
   {
      printf("Error: Could not open kick file! \n");
      FreeMem(CHIPROM,ROMSIZE+1024);
      return(FALSE);
   }

   actual = (ULONG)Seek(file,8L,OFFSET_BEGINNING);
   actual = (ULONG)Read(file,(char *)CHIPROM,(LONG)ROMSIZE); /* (128*1024)); */
   if (actual != ROMSIZE)
   {
      printf("Error: could not load 512K in chip ram! \n");
      Close(file);
      FreeMem(CHIPROM,ROMSIZE+1024);
      return(FALSE);
   }

   Close(file);

   /***********************************************************************/
   /* Let's set up the translation tables. Easy! Well...                  */
   /***********************************************************************/

   /*****************/
   /* Set up TableC */
   /*****************/

   for (i=0; i<16; i++)
   {
      temp = (ULONG)CHIPROM + (1024*32*i);
      TableC[i] = temp | PD_DT_PAGE;
   }

   for (i=16; i<16; i++)
   {
      temp = (ULONG)CHIPROM + (1024*32*i);
      TableC[i] = temp | PD_DT_PAGE;
   }

   /*****************/
   /* Set up TableB */
   /*****************/

   for (i=0; i<30; i++)
   {
      temp = (ULONG)(1024*512*i);
      TableB[i] = temp | PD_DT_PAGE;
   }

   TableB[30] = (ULONG)TableC | PD_DT_V4BYTE;
   TableB[31] = (ULONG)(1024*512*31) | PD_DT_PAGE;

   /*****************/
   /* Set up TableA */
   /*****************/

   TableA[0] = (ULONG)TableB | PD_DT_V4BYTE;
   for (i=1; i<128; i++) TableA[i] = (0x1000000 * i) | PD_DT_PAGE;

   myTC = TC_ENB | TC_PS(0x0F) | TC_IS(1) |
          TC_TIA(7)|TC_TIB(5)|TC_TIC(4)|TC_TID(0);

   myCRP[0] = 0x80000000 | CRP_SG | CRP_DT_V4BYTE;
   myCRP[1] = (ULONG)TableA;

      SetCRP(myCRP);
      SetTC(myTC);
      Reboot();

   return(TRUE);
}

BOOL CreateKickRAM(int argc, char *argv[])
{
   ULONG i, myCRP[2], myTC, temp;
   ULONG *Autovector, *ROMbase, *ROM16 = NULL, *CHIPROM = NULL;
   ULONG *TableA = NULL, *TableB = NULL, *TableC = NULL;
   long  actual;
   struct FileHandle *file;

/*
   CHIPROM = (ULONG *)AllocAlignedChip(ROMSIZE/2,32*1024);
   printf("CHIPROM: %lx   AllocSize: %lx \n",CHIPROM,ROMSIZE/2);
   if (!CHIPROM) return(FALSE);
*/
   CHIPROM = (ULONG *)AllocAbs(ROMSIZE/2,0x30000);
   printf("CHIPROM: %lx   AllocSize: %lx \n",CHIPROM,ROMSIZE/2);
   if (!CHIPROM) return(FALSE);

   ROM16 = (ULONG *)AllocAbs((ROMSIZE/2)+1024,(ULONG *)0xc30000);
   printf("ROM16: %lx   AllocSize: %lx \n",ROM16,(ROMSIZE/2)+1024);
   if (!ROM16)
   {
      FreeMem(CHIPROM,(ROMSIZE/2));
      return(FALSE);
   }

   /* TableA = AllocAligned(128*4,16); */
   /* TableB = AllocAligned(32*4,16);  */

   TableA = (ULONG *)((ULONG)ROM16+(ROMSIZE/2));
   TableB = (ULONG *)((ULONG)TableA + 512);	/* + (128*4); */
   TableC = (ULONG *)((ULONG)TableB + 128);	/* + (32*4); */

   printf("TableA: %lx  TableB: %lx  TableC: %lx \n",TableA,TableB,TableC);

   Autovector = (ULONG *)(GetVBR()+64);			/* Get address of autovectors */
   ROMbase = (ULONG *)(Autovector[0]&0xFFFF0000);	/* assumes ROM is aligned */

   printf("ROMbase: %lx \n",ROMbase);

   file = (struct FileHandle *)Open(argv[1],MODE_OLDFILE);
   if (!file)
   {
      printf("Error: Could not open kick file! \n");
      FreeMem(CHIPROM,ROMSIZE/2);
      FreeMem(ROM16,(ROMSIZE/2)+1024);
      return(FALSE);
   }

   actual = (ULONG)Seek(file,8L,OFFSET_BEGINNING);
   actual = (ULONG)Read(file,(char *)CHIPROM,(LONG)ROMSIZE/2);
   if (actual != ROMSIZE/2)
   {
      printf("Error: could not load 256K in chip ram! \n");
      Close(file);
      FreeMem(ROM16,(ROMSIZE/2)+1024);
      FreeMem(CHIPROM,ROMSIZE/2);
      return(FALSE);
   }

   actual = (ULONG)Seek(file,(ROMSIZE/2)+8L,OFFSET_BEGINNING);
   actual = (ULONG)Read(file,(char *)ROM16,(LONG)ROMSIZE/2);
   if (actual != ROMSIZE/2)
   {
      printf("Error: could not load 256K in C00000 ram! \n");
      Close(file);
      FreeMem(ROM16,(ROMSIZE/2)+1024);
      FreeMem(CHIPROM,ROMSIZE/2);
      return(FALSE);
   }

   Close(file);

   /***********************************************************************/
   /* Let's set up the translation tables. Easy! Well...                  */
   /***********************************************************************/

   /*****************/
   /* Set up TableC */
   /*****************/

   for (i=0; i<8; i++)
   {
      temp = (ULONG)CHIPROM + (1024*32*i);
      TableC[i] = temp | PD_DT_PAGE;
   }

   for (i=0; i<8; i++)
   {
      temp = (ULONG)ROM16 + (1024*32*i);
      TableC[i+8] = temp | PD_DT_PAGE;
   }

   /*****************/
   /* Set up TableB */
   /*****************/

   for (i=0; i<30; i++)
   {
      temp = (ULONG)(1024*512*i);
      TableB[i] = temp | PD_DT_PAGE;
   }

   TableB[30] = (ULONG)TableC | PD_DT_V4BYTE;
   TableB[31] = (ULONG)(1024*512*31) | PD_DT_PAGE;

   /*****************/
   /* Set up TableA */
   /*****************/

   TableA[0] = (ULONG)TableB | PD_DT_V4BYTE;
   for (i=1; i<128; i++) TableA[i] = (0x1000000 * i) | PD_DT_PAGE;

   myTC = TC_ENB | TC_PS(0x0F) | TC_IS(1) |
          TC_TIA(7)|TC_TIB(5)|TC_TIC(4)|TC_TID(0);

   myCRP[0] = 0x80000000 | CRP_SG | CRP_DT_V4BYTE;
   myCRP[1] = (ULONG)TableA;

      SetCRP(myCRP);
      SetTC(myTC);
      Reboot();

   return(TRUE);
}

BOOL CreateTable(long RamROM)
{
   ULONG i, myCRP[2], myTC, temp;
   ULONG *TableA = NULL, *TableB = NULL, *TableC = NULL;

   TableA = (ULONG *)((ULONG)RamROM+ROMSIZE);
   TableB = (ULONG *)((ULONG)TableA + 512);	/* + (128*4); */
   TableC = (ULONG *)((ULONG)TableB + 128);	/* + (32*4); */

   printf("TableA: %lx  TableB: %lx  TableC: %lx \n",TableA,TableB,TableC);

   /*****************/
   /* Set up TableC */
   /*****************/

   for (i=0; i<16; i++)
   {
      temp = (ULONG)RamROM + (1024*32*i);
      TableC[i] = temp | PD_DT_PAGE | PD_WP;
   }

   /*****************/
   /* Set up TableB */
   /*****************/

   for (i=0; i<30; i++)
   {
      temp = (ULONG)(1024*512*i);
      TableB[i] = temp | PD_DT_PAGE;
   }

   TableB[30] = (ULONG)TableC | PD_DT_V4BYTE;
   TableB[31] = (ULONG)(1024*512*31) | PD_DT_PAGE | PD_WP;

   /*****************/
   /* Set up TableA */
   /*****************/

   TableA[0] = (ULONG)TableB | PD_DT_V4BYTE;
   /* TableA[1] = (0x1000000 * i) | PD_DT_PAGE | PD_DT_WP; */
   for (i=1; i<128; i++) TableA[i] = (0x1000000 * i) | PD_DT_PAGE;

   myCRP[0] = 0x80000000 | CRP_SG | CRP_DT_V4BYTE;
   myCRP[1] = (ULONG)TableA;

   myTC = TC_ENB | TC_PS(0x0F) | TC_IS(1) |
          TC_TIA(7)|TC_TIB(5)|TC_TIC(4)|TC_TID(0);


      SetCRP(myCRP);


   return(TRUE);
}
