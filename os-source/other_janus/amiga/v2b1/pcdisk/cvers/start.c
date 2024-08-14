/************************************************************************
 * PCDisk.c
 * Code to allow the PC side to access files on the Amiga side.
 *
 * Copyright (c) 1987, Commodore Amiga Inc., All rights reserved
 ************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include "/j2500/janus.h"
#include "/j2500/setupsig.h"
#include "/j2500/services.h"
#include "pcddata.h"
#include "pcdisk.h"

struct JanusAmiga *JanusBase = 0;

struct FileTable ft;
int open_files=0;
char   *buffer=0;

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
#ifdef DEBUG
      puts("Unable to open Janus.Library");
#endif
      goto cleanup;
   }
#ifdef DEBUG
   printf("JanusBase at %lx\n",JanusBase);
#endif

   signum = AllocSignal(-1);
   if(signum == -1)
   {
#ifdef DEBUG
      puts("No free signal bits");
#endif
      goto cleanup;
   }
#ifdef DEBUG
   printf("Signum = %ld\n",signum);
#endif
   sigmask = 1 << signum;

   sigstruct = (struct SetupSig *)SetupJanusSig(16,signum,
                             sizeof(struct AmigaDiskReq),
                             MEMF_PARAMETER|MEM_WORDACCESS);
   if(sigstruct==0)
   {
#ifdef DEBUG
      puts("Error getting SetupSig struct ");
#endif
      goto cleanup;
   }
#ifdef DEBUG
   printf("Sigstruct at %lx\n",sigstruct);
#endif

   buffer = (char *)AllocJanusMem(PCDISK_BUFFER_SIZE,MEMF_BUFFER|MEM_BYTEACCESS);
   if(buffer==0)
   {
#ifdef DEBUG
      puts("Could not allocate disk buffer");
#endif
      goto cleanup;
   }
#ifdef DEBUG
   printf("Buffer at %lx\n",buffer);
#endif

   ReqPtr = (struct AmigaDiskReq *)sigstruct->ss_ParamPtr;
#ifdef DEBUG
   printf("ReqPtr = %lx\n",ReqPtr);
#endif
   ReqPtr->adr_BufferOffset = JanusMemToOffset(buffer);
#ifdef DEBUG
   printf("adr_Buffer Offset = %lx\n",(int)ReqPtr->adr_BufferOffset);
#endif

   /* Initialize open file table */
   for(x=0;x<MAXFILES;x++)
   {
      ft.File[x].fh=0;
      ft.File[x].cur_pos=0;
   }

   MainProg(sigmask,sigstruct,ReqPtr);  /* This function never returns */

cleanup:
   if(buffer)  FreeJanusMem(buffer,PCDISK_BUFFER_SIZE);
   if(sigstruct) CleanupJanusSig(sigstruct);
   if(signum!= -1) FreeSignal(signum);
   if(JanusBase) CloseLibrary(JanusBase);
}
