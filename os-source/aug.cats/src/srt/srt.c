
;/* srt.c - Execute me to compile me with Lattice 5.04
LC -b0 -cfistq -v -j73 srt.c
Blink FROM LIB:astartup.obj,srt.o,srta.o TO srt LIBRARY LIB:amiga.lib,LIB:LC.lib
quit
*/

/*
 * srt.c v1.32 - Installs srta.asm Wedge to replace common requester text
 *
 *            Carolyn Scheppner   CBM  07/88
 *            Copyright 1988 Commodore-Amiga, Inc.
 *
 * Replaces requester text from file of form:
 *
 * N=linecount
 * "old text"  "new text"
 * "old text"  "new text"
 *  etc.
 *
 *  Special feature:
 *   If your replacement text for "Software error - task held" is "DEBUG",
 *   standard software error requesters will be replaced by a full width
 *   display of the name, address, and registers of the crashed task.
 *   This feature is version dependent and will probably break under 1.4.
 *
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <workbench/startup.h>

#ifdef LATTICE
#include <proto/all.h>
#include <stdlib.h>
#include <string.h>
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif

void cleanexit(UBYTE *s, LONG err);
void cleanup(void);
void printf(BYTE *fmt,...);
 

#define SBUFSZ  256
#define LVOAutoRequest (-348L)

extern  LONG startFix, endFix, useCount;
extern  struct WBStartup *WBenchMsg;

BOOL  FromWb;
LONG  ptrBytes, tLen, tfile=0, startLock=0, newLock=0, rtCount=0;
struct Library *IntuitionBase = NULL;
ULONG OldFunc;
UBYTE sbuf[256] = {0}, **tptrs, *reqtext;

struct SRTPort {
   struct MsgPort srtPort;
   ULONG          *Wedge;
   ULONG          WedgeSize;
   ULONG          RealFunc;
   ULONG          MyFunc;
   ULONG          *ptrToUseCount;
   UBYTE          **ptrToTextPtrs;
   UBYTE          *ptrToTextArea;
   char           Name[48];
   };

struct SRTPort *srtp;
char *srtpName = "CAS_CBM_SRT_Port_v1.32";
char *format = "N=linecount\n\"old\" \"new\"\n\"old\" \"new\"\netc.\n";
char *Copyright="Copyright 1988 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *defname = "s:srt.text";

void main(int argc, char **argv)
   {
   struct WBArg *arg;
   LONG   fSize, wSize, rLen;
   ULONG  fixsize, *reloc;
   char   *tfname;
   int    j, k;

   FromWb = (argc==0) ? TRUE : FALSE;
   
   tfname = defname;

   if((argc>1)&&(*argv[1]=='?'))
      {
      if(!FromWb)
         {
         printf("%s v1.32   %s\n",argv[0],Copyright);
         printf("Usage: %s file|RESET (default file s:srt.text)\n",argv[0]);
         printf("File format:\n%s",format);
         }
      cleanexit("",RETURN_WARN);
      }
   else if((FromWb)&&(WBenchMsg->sm_NumArgs > 1))
      {
      arg = WBenchMsg->sm_ArgList;
      arg++;
      tfname  = (char *)arg->wa_Name;
      newLock = arg->wa_Lock;
      startLock = CurrentDir(newLock);
      }
   else if(argc > 1)  tfname = argv[1];

   if(!(IntuitionBase=OpenLibrary("intuition.library",0)))
      cleanexit("Can't open Intuition",RETURN_FAIL);

   if(stricmp(tfname,"RESET"))
      {
      if(!(tfile = Open(tfname, MODE_OLDFILE)))
         {
         if(!FromWb) printf("Input file %s not found.  Type  %s ?  for usage.\n",
                                tfname,argv[0]);
         cleanexit(" ",RETURN_FAIL);
         }

      Seek(tfile,0,OFFSET_END);

      fSize = Seek(tfile,0,OFFSET_BEGINING);
      rLen = Read(tfile,sbuf,SBUFSZ-8);
      if((fSize < 1)||(rLen < 1))
         cleanexit("Can't read input file",RETURN_FAIL);

      for(k=0; (k < rLen)&&((sbuf[k]==' ')||(sbuf[k]=='\n')); k++);
      k++;
      if(sbuf[k] != '=')
         cleanexit("File must start with N=linecount",RETURN_FAIL);
      k++;
      for(j=k; (j < rLen)&&(sbuf[j]!='\n'); j++);
      sbuf[j] = NULL;
      j++;
      tLen = fSize - j;

      if((!(rtCount=atoi(&sbuf[k])))||(tLen < 7))
         cleanexit("File must contain: N=linecount<LF> then \"old\" \"new\"<LF> lines",
                     RETURN_FAIL);
      }

   Forbid();
   /* If present already, remove, whether resetting or installing */
   if(srtp = (struct SRTPort *)FindPort(srtpName))
      {
      if(*(srtp->ptrToUseCount))
         {
         Permit();
         cleanexit("SRT may not be removed or changed while in use",
                     RETURN_WARN);
         }
      /* Try to unwedge */
      OldFunc = (ULONG)SetFunction(IntuitionBase,LVOAutoRequest,srtp->RealFunc);


      if(OldFunc != srtp->MyFunc)
         {
         /* Someone else has changed the vector */
         /* We put theirs back - can't exit yet  */
         SetFunction(IntuitionBase, LVOAutoRequest, OldFunc);
         Permit();
         cleanexit("Can't remove SRT - another SetFunction present",
                     RETURN_WARN);
         }
      else
         {
         RemPort(srtp);
         while(*(srtp->ptrToUseCount))  Delay(20);
         FreeMem(srtp->Wedge, srtp->WedgeSize);
         }
      }
   Permit();


   /* If not just resetting, install new requester text */
   if(stricmp(tfname,"RESET"))
      {
      fixsize = (ULONG)&endFix - (ULONG)&startFix;
      ptrBytes = (rtCount << 3) + 16;
      wSize = fixsize+sizeof(struct SRTPort)+ptrBytes+tLen+16;

      if(!(reloc = (ULONG *)AllocMem(wSize,MEMF_PUBLIC|MEMF_CLEAR)))
         {
         cleanexit("Not enough memory",RETURN_FAIL);
         }
      else
         {
         /* Copy relocatable assembler code to alloc'd memory */
         CopyMem(&startFix,reloc,fixsize);

         /* Text ptrs next, then SRTPort... Set up port */
         tptrs = (UBYTE **)((ULONG)reloc + (ULONG)fixsize);

         srtp = (struct SRTPort *)((ULONG)(tptrs) + ptrBytes);
         reqtext = (UBYTE *)((ULONG)srtp + sizeof(struct SRTPort));

         srtp->ptrToTextPtrs = tptrs;
         srtp->ptrToTextArea = reqtext;
         srtp->ptrToUseCount =
            (ULONG *)((ULONG)reloc+((ULONG)&useCount-(ULONG)&startFix));

         srtp->Wedge = (ULONG *)reloc;
         srtp->WedgeSize = wSize;
         strcpy(srtp->Name,srtpName);
         srtp->srtPort.mp_Node.ln_Name = srtp->Name;
         srtp->srtPort.mp_Node.ln_Type = NT_MSGPORT;

         /* Seek past N= line */
         Seek(tfile,fSize-tLen,OFFSET_BEGINING);
         rLen = Read(tfile,reqtext,tLen);

         for(k=0,j=0; (k<tLen)&&(j<(rtCount<<1)); j++)
            {
            /* Seek to first quote of match string */
            while((k<tLen)&&(reqtext[k]!=0x22)) k++;
            k++;
            /* ptr to match string */
            tptrs[j] = &reqtext[k];
            /* find end of match string */
            while((k<tLen)&&(reqtext[k]!=0x22)) k++;
            /* NULL terminate the string */
            if(k<tLen) reqtext[k] = NULL, k++;

            /* Now find replacement string */
            j++;
            /* Seek to initial quote of replacement string */
            while((k<tLen)&&(reqtext[k]!=0x22)) k++;
            k++;
            /* ptr to replacement string */
            tptrs[j] = &reqtext[k];
            /* find end of replacement string */
            while((k<tLen)&&(reqtext[k]!=0x22)) k++;
            /* NULL terminate the string */
            if(k<tLen) reqtext[k] = NULL, k++;
            }

         Forbid();
         /* If we find another srtPort at this point, Free ours and abort */
         if(FindPort(srtpName))
            {
            Permit();
            FreeMem(srtp->Wedge,srtp->WedgeSize);
            cleanexit("Aborted... Another SRT installation occurred",
                         RETURN_WARN);
            }
         else
            {
            /* else Add our port and SetFunction to our code */
            AddPort(srtp);

            /* Store old AutoRequest in first long of reloc */
            /* New function entry is just past that long        */
            *reloc =
             (ULONG)SetFunction(IntuitionBase, LVOAutoRequest,
              ((ULONG)reloc) + 4);
            srtp->RealFunc = (ULONG)(*reloc);
            srtp->MyFunc = ((ULONG)reloc) + 4;
            }
         Permit();

         }
      }
   cleanup();
   }


void cleanexit(UBYTE *s, LONG err)
   {
   if((*s)&&(!FromWb))
      {
      printf("%s \n",s);
      }
   cleanup();
   exit(err);
   }

void cleanup()
   {
   if(tfile)   Close(tfile);
   if(newLock) CurrentDir(startLock);
   if(IntuitionBase)  CloseLibrary(IntuitionBase);
   }


int atoi( char *s )
   {
   int num = 0;
   int neg = 0;

   if( *s == '+' ) s++;
   else if( *s == '-' ) {
       neg = 1;
       s++;
   }

   while( *s >= '0' && *s <= '9' ) {
       num = num * 10 + *s++ - '0';
   }

   if( neg ) return( - num );
   return( num );
   }

