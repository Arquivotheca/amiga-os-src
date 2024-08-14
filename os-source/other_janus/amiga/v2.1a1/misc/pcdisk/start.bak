/************************************************************************
 * PCDisk.c
 * Code to allow the PC side to access files on the Amiga side.
 *
 * Copyright (c) 1987, Commodore Amiga Inc., All rights reserved
 ************************************************************************/

#include <janus/janus.h>
#include <exec/memory.h>
#include "pcddata.h"
#include <stdio.h>

#define D(x) ;
/* #define printf kprintf */

struct JanusAmiga *JanusBase = 0;

struct FileTable ft;
int open_files=0;
char   *buffer=0;
char   *rbuffer=0;

#define DEBUG_OFF 0
#define DEBUG_CON 1
#define DEBUG_SER 2

UBYTE Debug    = DEBUG_CON;
FILE  *console = 0;

#define DMA_OFF   0
#define DMA_ON    1

UBYTE DMA      = DMA_OFF;

void main();

void main()
{
   int                  signum      = -1;
   int                  sigmask     = 0;
   struct SetupSig      *sigstruct  = 0;
   struct AmigaDiskReq  *ReqPtr     = 0;
   int                  x;

   if(Debug)
   {
      if(Debug==DEBUG_CON)
      {
         if((console=fopen("CON:0/0/640/200/PCDisk Debug Window","w"))==0)
         {
            return;
         }
      }
   }

   JanusBase = (struct JanusAmiga *) OpenLibrary(JANUSNAME,0);
   if(!JanusBase)
   {
      D( puts("Unable to open Janus.Library"); )
      goto cleanup;
   }

   if(Debug)
      dprintf("JanusBase @ 0x%lx\n",JanusBase);

   signum = AllocSignal(-1);
   if(signum == -1)
   {
      D( puts("No free signal bits"); )
      goto cleanup;
   }
   if(Debug)
      dprintf("Signum = %ld\n",signum);
   sigmask = 1 << signum;

   sigstruct = (struct SetupSig *)SetupJanusSig(JSERV_PCDISK,signum,
                             sizeof(struct AmigaDiskReq),
                             MEMF_PARAMETER|MEM_WORDACCESS);
   if(sigstruct==0)
   {
      D( puts("Error getting SetupSig struct "); )
      goto cleanup;
   }
   if(Debug)
      dprintf("Sigstruct @ 0x%lx\n",sigstruct);

   buffer = (char *)AllocJanusMem(PCDISK_BUFFER_SIZE,MEMF_BUFFER|MEM_BYTEACCESS);
   if(buffer==0)
   {
      D( puts("Could not allocate disk buffer"); )
      goto cleanup;
   }
   if(Debug)
      dprintf("buffer @ 0x%lx\n",buffer);

   if( ! DMA)
   {
      rbuffer = (char *)AllocMem(PCDISK_BUFFER_SIZE,MEMF_CHIP|MEMF_CLEAR);
      if(rbuffer==0)
      {
         D( puts("Could not allocate disk rbuffer"); )
         goto cleanup;
      }
      if(Debug)
         dprintf("rbuffer @ 0x%lx\n",buffer);
   }

   ReqPtr = (struct AmigaDiskReq *)sigstruct->ss_ParamPtr;
   ReqPtr->adr_BufferOffset = JanusMemToOffset(buffer);
   if(Debug)
   {
      dprintf("ReqPtr = 0x%lx\n",ReqPtr);
      dprintf("adr_Buffer Offset = 0x%lx\n",(int)ReqPtr->adr_BufferOffset);
   }

   /* Initialize open file table */
   for(x=0;x<MAXFILES;x++)
   {
      ft.File[x].fh=0;
      ft.File[x].cur_pos=0;
   }

   MainProg(sigmask,sigstruct,ReqPtr);  /* This function never returns */

cleanup:
   if(rbuffer) FreeMem(rbuffer,PCDISK_BUFFER_SIZE);
   if(buffer)  FreeJanusMem(buffer,PCDISK_BUFFER_SIZE);
   if(sigstruct) CleanupJanusSig(sigstruct);
   if(signum!= -1) FreeSignal(signum);
   if(JanusBase) CloseLibrary(JanusBase);
   if(console) fclose(console);
}
