/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
* |_o_o|\\ Copyright (c) 1989 The Software Distillery.                    *
* |. o.| ||          All Rights Reserved                                  *
* | .  | ||          Written by Doug Walker                               *
* | o  | ||          The Software Distillery                              *
* |  . |//           235 Trillingham Lane                                 *
* ======             Cary, NC 27513                                       *
*                    BBS:(919)-471-6436                                   *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "netcomm.h"
#include "proto.h"
#include "memdb.h"

#ifdef AllocMem
#undef AllocMem
#endif

#ifdef FreeMem
#undef FreeMem
#endif

#define MWT_CHIP 0
#define MWT_FAST 1

struct MWGlobal
{
   LONG flags;          /* Various MWF_ flags, see below                */
   LONG num[2];         /* Current number of allocations, chip and fast */
   LONG sum[2];         /* Current amount allocated, chip and fast      */
   LONG max[2];         /* Max amount allocated, chip and fast          */
   LONG lim[2];         /* Limit on allocations, chip and fast          */
   struct MWAlc *first; /* List of active memory allocations            */
   struct MWAlc *freed; /* List of free memory extents                  */
   struct MWAlc *lfree; /* Last allocation freed with free()            */
   struct Task *task;   /* Pointer to owning task's Task structure      */
};

struct MWAlc
{
   struct MWAlc *next;  /* Next memory block in chain           */
   LONG size;           /* Size of allocation in bytes          */
   LONG flags;          /* MEMF_ Flags memory was allocated with*/
   LONG myflags;        /* my flags, see defines below          */
   char *file;          /* Filename containing allocation point */
   LONG line;           /* Line number of allocation            */
   char *ffile;         /* Filename of free point               */
   char *fline;         /* Line number of free point            */
   char header[4];      /* Header sentinal                      */
   char memory[4];      /* Actual allocation comes here         */
                        /* 4 extra bytes covers trailer sentinal*/
};

/* Defines for use with MWAlc.myflags */
/* if myflags&MWF_REPMASK == MWF_REPORTED,   */
/* This alloc already reported as trashed    */
/* Use multiple bits in case the 'myflags'   */
/* are trashed, odds are better of detecting */
/* If we ever need more myflag bits, just    */
/* define MWF_REPMASK not to include them    */

#define MWF_REPORTED 0xaa55aa55
#define MWF_REPMASK  0xffffffff

#define MWHEADER     "MWHD"  /* Beginning sentinal                       */
#define MWTRAILER    "MWTR"  /* Trailing sentinal                        */

#define MWATRASH     0xaa  /* Trash allocated memory with this           */
#define MWFTRASH     0x55  /* Trash freed memory with this               */

void MWPurge (GLOBAL);
void MWPrint (GLOBAL, struct MWAlc *, int, LONG, LONG);

struct MWGlobal mwg = {0L};  /* Make sure the 'flags' field is 0's */

/* Initialization routine - should be called once before any memory */
/* is allocated                                                     */

void MWInit(GLOBAL global, LONG flags)
{
   if(mwg.flags & MWF_ACTIVE)
   {
      BUG(("*******\n*******MemWatch ERROR: MWInit called twice\n*******\n"))
      return;
   }

   memset((char *)&mwg, 0, sizeof(struct MWGlobal));

   mwg.flags = flags|MWF_ACTIVE;

   mwg.lim[MWT_CHIP] = mwg.lim[MWT_FAST] = 0x7fffffff;

   mwg.task = FindTask(0L);

   BUG(("MWInit: Memory debugging initialized\n"))
}

/* Termination routine - should be called after all memory usage is done */

void MWTerm(GLOBAL global)
{
   int msgdone = 0;

   BUG(("MWTerm: Terminating memory debugging\n"))

   MWCheck(global);

   /* no need to trash the mem we may be about to free for good */
   mwg.flags |= MWF_NOFTRASH;

   if(!(mwg.flags & MWF_NOFREE))
   {
      while(mwg.first)
      {
         if(!msgdone)
         {
            msgdone = 1;
            BUG(("******ERROR: The following allocations were not freed:\n"))
            MWPrint(global, mwg.first, 0, 0, 0);
            BUGR("Unfreed memory on exit!");
         }
         MWFreeMem(global, mwg.first->memory, mwg.first->size, 0);
      }
   }

   MWPurge(global);  /* Really free all mem on the 'free' chain */

   memset((char *)&mwg, 0, sizeof(struct MWGlobal));
}

/* Validity check routine - checks all known allocations for overwrites */
/* Called from every alloc and free routine, plus when specifically     */
/* invoked                                                              */

void MWCheck(GLOBAL global)
{
   struct MWAlc *mwa;
   char *tmpchar;
   int header, trailer;

   for(mwa=mwg.first; mwa; mwa=mwa->next)
   {
      if( (mwa->myflags & MWF_REPMASK) == MWF_REPORTED) continue;

      if(header=memcmp((char *)&mwa->header, MWHEADER, 4))
      {
         BUG(("MemWatch ERROR: Header trashed\n"))
      }

      if(trailer=memcmp(mwa->memory+mwa->size, MWTRAILER, 4))
      {
         BUG(("MemWatch ERROR: Trailer trashed\n"))
      }

      if(header || trailer)
      {
         mwa->myflags |= MWF_REPORTED;
         MWPrint(global, mwa, 0, 0, 0);
         if(header && trailer) {BUGR("NET: trashed header/trailer!");}
         else if(header)       {BUGR("NET: trashed header!");        }
         else if(trailer)      {BUGR("NET: trashed trailer!");       }
      }
   }

   for(mwa=mwg.freed; mwa; mwa=mwa->next)
   {
      if( (mwa->myflags & MWF_REPMASK) == MWF_REPORTED) continue;

      for(header=0, tmpchar=mwa->memory; 
          header<mwa->size; 
          header++, tmpchar++)
      {
         if(*tmpchar != MWFTRASH)
         {
            mwa->myflags |= MWF_REPORTED;
            BUG(("MemWatch ERROR: Freed memory modified\n"))
            MWPrint(global, mwa, 0, 0, 0);
            BUGR("NET: Modified freed memory!")
            break;
         }
      }
   }
}
void *MWAllocMem(GLOBAL global, long size, long flags, char *file, long line)
{
   struct MWAlc *hd;
   char *tmpchar;
   int memtype;

   if(!(mwg.flags & MWF_ACTIVE)) return(NULL);

   memtype = (flags & MEMF_CHIP ? MWT_CHIP : MWT_FAST);
   if(mwg.sum[memtype] + size > mwg.lim[memtype])
   {
      /* Over the limit, fail it */
      BUG(("MemWatch: %s", memtype == MWT_CHIP ? "CHIP" : "FAST"))
      BUG(("memory allocation exceeds MWLimit amount\n"))
      return(NULL);
   }

   while(!(tmpchar = (char *)AllocMem(sizeof(struct MWAlc)+size, flags)))
   {
      if(mwg.freed) MWPurge(global);
      else return(NULL);
   }
   
   hd = (struct MWAlc *)tmpchar;
   hd->size = size;
   hd->flags = flags;
   hd->myflags = 0L;
   hd->file = file;
   hd->line = line;
   memcpy((char *)&hd->header, MWHEADER, 4);
   tmpchar += (sizeof(struct MWAlc)+size-4);
   memcpy(tmpchar, MWTRAILER, 4);
   if(!(flags & MEMF_CLEAR) && !(mwg.flags & MWF_NOATRASH))
      memset(hd->memory, MWATRASH, size);   /* Trash the memory */

   hd->next = mwg.first;
   mwg.first = hd;

   if((mwg.sum[memtype] += size) > mwg.max[memtype]) 
      mwg.max[memtype] = mwg.sum[memtype];
   ++(mwg.num[memtype]);

   return((char *)hd->memory);
}

void MWFreeMem(GLOBAL global, void *mem, long size, int internal)
{
   struct MWAlc *mwa, *prev;
   int memtype;

   if(!(mwg.flags & MWF_ACTIVE)) return;

   for(prev = NULL, mwa = mwg.first; 
       mwa && mwa->memory != mem; 
       prev = mwa, mwa = mwa->next);

   if(!mwa)
   {
      MWPrint(global, NULL, 1, (LONG)mem, size);
      BUGR("MWFreeMem: Freeing unknown memory")
      return;
   }
   else if(!internal && mwa->size != size)
   {
      MWPrint(global, NULL, 2, size, 0);
      MWPrint(global, mwa,  0,    0, 0);
      BUGR("MWFreeMem: Free wrong length")
   }

   memtype = (mwa->flags & MEMF_CHIP ? MWT_CHIP : MWT_FAST);
   mwg.sum[memtype] -= mwa->size;
   --mwg.num[memtype];

   if(prev) prev->next = mwa->next;
   else     mwg.first = mwa->next;

   if(!(mwg.flags & MWF_NOFTRASH))
      memset(mwa->memory, MWFTRASH, mwa->size);  /* Trash it */

   if(mwg.flags & MWF_NOFKEEP)
     FreeMem((char *)mwa, size + sizeof(struct MWAlc));
   else
   {
      mwa->next = mwg.freed;
      mwg.freed = mwa;
   }
}


void MWPurge(GLOBAL global)
{
   struct MWAlc *cur, *next;

   for(cur=mwg.freed; cur; cur=next)
   {
      next = cur->next;
      FreeMem(cur, cur->size + sizeof(struct MWAlc));
   }
   mwg.freed = NULL;
}

#define ALCMSG \
  "0x00000000 length 00000000 allocated line 00000 file xxxxxxxxxxxxxxx\n"
 /*0         1         2         3         4         5         6*/
 /*0123456789012345678901234567890123456789012345678901234567890*/
#define ALC_MEM  2
#define ALC_LEN 18
#define ALC_LIN 42
#define ALC_FIL 53

#define FREMSG \
  "Invalid FreeMem call, addr 0x00000000 length 00000000\n"
 /*0         1         2         3         4         5         6*/
 /*0123456789012345678901234567890123456789012345678901234567890*/
#define FRE_MEM 29
#define FRE_LEN 45

#define LENMSG \
  "FreeMem called with length 00000000 on the following allocation:\n"
 /*0         1         2         3         4         5         6*/
 /*0123456789012345678901234567890123456789012345678901234567890*/
#define LEN_LEN 27
  
#define USEMSG \
  "00000000 bytes in 00000000 allocations\n"
 /*0         1         2         3         4         5         6*/
 /*0123456789012345678901234567890123456789012345678901234567890*/
#define USE_SIZ 0
#define USE_ALC 18

#define MAXMSG \
  "Max chip usage = 00000000 bytes; Max fast usage = 00000000 bytes\n\n"
 /*0         1         2         3         4         5         6*/
 /*0123456789012345678901234567890123456789012345678901234567890*/
#define MAX_CHP 17
#define MAX_FST 50

#define DIGITS "0123456789ABCDEF"

static void fmtdec(char *, long, LONG);
static void fmthex(char *, LONG);

static char *msgs[] =
{
   ALCMSG, FREMSG, LENMSG, USEMSG, MAXMSG
};

void MWPrint(GLOBAL global, struct MWAlc *mwa, int msgnum, LONG val1, LONG val2)
{
   char *buffer;
   int i;

   if(!(mwg.flags & MWF_ACTIVE)) return;

   buffer = msgs[msgnum];

   switch(msgnum)
   {
      case 0:
         fmthex(msgs[0]+ALC_MEM, (LONG)mwa->memory);
         fmtdec(msgs[0]+ALC_LEN, 8, mwa->size);
         fmtdec(msgs[0]+ALC_LIN, 5, mwa->line);

         if( (i = strlen(mwa->file)) > 15) i=15;
         memcpy(msgs[0]+ALC_FIL, mwa->file, i);
         msgs[0][ALC_FIL+i]   = '\n';
         msgs[0][ALC_FIL+i+1] = '\0';
         break;

      case 1:
         fmthex(msgs[1]+FRE_MEM, val1);
         fmtdec(msgs[1]+FRE_LEN, 8, val2);
         break;

      case 2:
         fmtdec(msgs[2]+LEN_LEN, 8, val1);
         break;

      case 3:
         fmtdec(msgs[3]+USE_SIZ, 8, val1);
         fmtdec(msgs[3]+USE_ALC, 8, val2);
         break;

      case 4:
         fmtdec(msgs[4]+MAX_CHP, 8, val1);
         fmtdec(msgs[4]+MAX_FST, 8, val2);
         break;

      default:
         /* What do we do here?  Give up, I guess*/
         return;
   }

   BUGR(buffer)
}

static void fmtdec(buf, len, val)
char *buf;
long len;
LONG val;
{
   int i;
   for(i=1; i<=len; i++, val/=10)
      buf[len-i] = (val ? DIGITS[val%10] : ' ');
}

static void fmthex(buf, val)
char *buf;
LONG val;
{
   int i, j;
   union
   {
      LONG l;
      char c[4];
   } u;

   u.l = val;
   for(i=j=0; i<4; i++)
   {
      buf[j++] = DIGITS[(u.c[i]&0xf0)>>4];
      buf[j++] = DIGITS[u.c[i]&0x0f];
   }
}

void MWReport(GLOBAL global, char *title, int level)
{
   struct MWAlc *mwa;

   if(!(mwg.flags & MWF_ACTIVE)) return;

   MWCheck(global);

   if(level == MWR_NONE) return;

   BUG(("********** MEMORY USAGE SUMMARY\n%s\n", title ? title : ""))

   BUG(("\n\nCurrent chip usage = "))
   MWPrint(global, NULL, 3, mwg.sum[MWT_CHIP], mwg.num[MWT_CHIP]);

   BUG(("Current fast usage = "))
   MWPrint(global, NULL, 3, mwg.sum[MWT_FAST], mwg.num[MWT_FAST]);

   MWPrint(global, NULL, 4, mwg.max[MWT_CHIP], mwg.max[MWT_FAST]);

   if(level == MWR_SUM) return;

   BUG(("********** MEMORY USAGE DETAIL\n\n"))

   /*           0x00000000 [00000] ... */
   for(mwa=mwg.first; mwa; mwa=mwa->next)
      MWPrint(global, mwa, 0, 0, 0);

   BUG(("\n**********\n\n"))
}
