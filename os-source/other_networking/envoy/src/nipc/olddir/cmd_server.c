
#include <stdio.h>
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/semaphores.h>
#include <exec/ports.h>
#include <dos/dos.h>
#include "pragmas/nipc_pragmas.h"
#include "nipc.h"

struct Library *NIPCBase;

main(argc,argv)
int argc;
char **argv;
{

 void *server;
 ULONG sigbit;
 ULONG srctags[8]={ENT_Name,(ULONG) "remote command server",ENT_Public,0,ENT_AllocSignal,0L,TAG_DONE,0};
 int filecount=0;

 srctags[5] = (ULONG) &sigbit;
 sigbit = 0;
 NIPCBase = (struct Library *) OpenLibrary("nipc.library",0L);
 /* Yes, I'm aware that I'm counting on c.o to open the dos.library.
  * This is hack, anyways.
  */

 printf("local createentitya\n");

 server = (void *) CreateEntityA(&srctags);

 printf("While()\n");

 while (TRUE)
   {
   ULONG rcvd;
   struct Transaction *xaction;
   while (TRUE)
      {
      xaction = (struct Transaction *) GetTransaction(server);
      printf("GetT()=%lx\n",xaction);
      if (xaction)
         {
         char cmd[256], fname[80];
         char yawn[80];
         ULONG outf;
         printf("received a transaction.  %lx %lx\n",xaction,xaction->trans_SourceEntity);
         memcpy(&cmd,xaction->trans_RequestData,xaction->trans_ReqDataActual);
         cmd[xaction->trans_ReqDataActual]=0;
         sprintf(fname,"t:cmdtmp%d",filecount++);
         outf = Open(fname,MODE_NEWFILE);
         printf("ste size %lx\n",xaction->trans_ReqDataActual);
         printf("string to execute: %s\n",xaction->trans_RequestData);
         Execute(xaction->trans_RequestData,0L,outf);
         Close(outf);
         outf = Open(fname,MODE_OLDFILE);
         if (!outf)
            printf("Couldn't open file ..\n");
         xaction->trans_RespDataActual =
            Read(outf,xaction->trans_ResponseData,xaction->trans_RespDataLength);
         ((UBYTE *)xaction->trans_ResponseData)[xaction->trans_RespDataActual++]=0;
         Close(outf);
//         DeleteFile(fname);
         yawn[0]=0;
         if (GetEntityName(xaction->trans_SourceEntity,&yawn,80))
            printf("Call from entity '%s'\n",&yawn);
         if (GetHostName(xaction->trans_SourceEntity,&yawn,80))
            printf("Call from host '%s'\n",&yawn);

         ReplyTransaction(xaction);
         }
      else
         break;
      }

   printf("Wait()\n");
   rcvd = Wait (SIGBREAKF_CTRL_C | (1 << sigbit) );
   if (rcvd & SIGBREAKF_CTRL_C) break;
   }

 printf("DeleteEntity()\n");
 DeleteEntity(server);

 printf("return()\n");
}

