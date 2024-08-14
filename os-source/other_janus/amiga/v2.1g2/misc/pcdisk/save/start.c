/************************************************************************
 * PCDisk.c
 * Code to allow the PC side to access files on the Amiga side.
 *
 * Copyright (c) 1987, Commodore Amiga Inc., All rights reserved
 ************************************************************************/

#include <janus/janus.h>
#include <exec/memory.h>
#include "pcddata.h"

#define D(x) ;
#define printf kprintf

struct JanusAmiga *JanusBase = 0;

struct FileTable ft;
int open_files=0;
char   *buffer=0;
char   *rbuffer=0;

void main();

void main()
{
   int    signum   = -1;
   int    sigmask  = 0;
   struct SetupSig *sigstruct = 0;
   struct AmigaDiskReq *ReqPtr = 0;
   int x;

   JanusBase = (struct JanusAmiga *) OpenLibrary(JANUSNAME,0);
   if(!JanusBase)
   {
      D( puts("Unable to open Janus.Library"); )
      goto cleanup;
   }
   D( printf("JanusBase at %lx\n",JanusBase); )

   signum = AllocSignal(-1);
   if(signum == -1)
   {
      D( puts("No free signal bits"); )
      goto cleanup;
   }
   D( printf("Signum = %ld\n",signum); )
   sigmask = 1 << signum;

   sigstruct = (struct SetupSig *)SetupJanusSig(JSERV_PCDISK,signum,
                             sizeof(struct AmigaDiskReq),
                             MEMF_PARAMETER|MEM_WORDACCESS);
   if(sigstruct==0)
   {
      D( puts("Error getting SetupSig struct "); )
      goto cleanup;
   }
   D( printf("Sigstruct at %lx\n",sigstruct); )

   buffer = (char *)AllocJanusMem(PCDISK_BUFFER_SIZE,MEMF_BUFFER|MEM_BYTEACCESS);
   if(buffer==0)
   {
      D( puts("Could not allocate disk buffer"); )
      goto cleanup;
   }
   D( printf("Buffer at %lx\n",buffer); )

/*
   rbuffer = (char *)AllocMem(PCDISK_BUFFER_SIZE,MEMF_CLEAR);
   if(rbuffer==0)
   {
      D( puts("Could not allocate disk rbuffer"); )
      goto cleanup;
   }
   D( printf("RBuffer at %lx\n",rbuffer); )
*/

   ReqPtr = (struct AmigaDiskReq *)sigstruct->ss_ParamPtr;
   D( printf("ReqPtr = %lx\n",ReqPtr); )
   ReqPtr->adr_BufferOffset = JanusMemToOffset(buffer);
   D( printf("adr_Buffer Offset = %lx\n",(int)ReqPtr->adr_BufferOffset); )

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
}
