/************************************************************************
 *
 * DJMount.c   -  mount command for jdisk.device
 *
 * Copyright (c) 1988, Commodore Amiga Inc., All rights reserved.
 *
 * 10-04-88 - BK - Converted to C lanquage
 ************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dosextens.h>
#include <libraries/filehandler.h>
#include <janus/janus.h>

#define  DEBUG_PCRUNNING   0
#define  DEBUG_WAITPC      0
#define  DEBUG_REQPTR      0
#define  DEBUG_INITERR     0
#define  DEBUG_NUMOFPARTS  0
#define  DEBUG_PARTTABLE   0
#define  DEBUG_PARTINFO    0

#define PARMPKTSIZE  (25*4)

char *name_array[8] = {
   "JH0",
   "JH1",
   "JH2",
   "JH3",
   "JH4",
   "JH5",
   "JH6",
   "JH7" };

char *name_array1[8] = {
   "JH0:",
   "JH1:",
   "JH2:",
   "JH3:",
   "JH4:",
   "JH5:",
   "JH6:",
   "JH7:" };


struct JanusBase     *JanusBase     = 0;
struct ExpansionBase *ExpansionBase = 0;

extern APTR DOSBase;

main(argc,argv)
int argc;
char *argv[];
{
   int    ReturnCode               = 0;
   int    Signal                   = -1;
   struct SetupSig      *SS        = 0;
   struct HardDskReq    *ReqPtr    = 0;
   int    Parts                    = 0;
   struct HDskPartition *PartTable = 0;
   struct HDskPartition *PartPtr   = 0;
   int    CPart                    = 0;
   ULONG  *ParmPkt                 = 0;
   struct DeviceNode    *DN        = 0;
   UBYTE  *StupidAssBCPLHName      = 0;
   UBYTE  FFSFlag                  = 0;

   if(argc!=2 && argc!=1)
   {
      printf("Usage:  DJMount [FFS]\n");
      return(20);
   }
   if(argc==2)
   {
      if (stricmp("FFS",argv[1])==0)
         FFSFlag = 1;
      else {
         printf("Usage:  DJMount [FFS]\n");
         return(20);
      }
   }

   if((JanusBase=(struct JanusBase *)OpenLibrary("janus.library",0))==0)
   {
      printf("Janus.library failed to open!!\n");
      ReturnCode=20;
      goto cleanup;
   }
   if((ExpansionBase=(struct ExpansionBase *)OpenLibrary("expansion.library",0))==0)
   {
      printf("Expansion.library failed to open!!\n");
      ReturnCode=20;
      goto cleanup;
   }
   if ((Signal=AllocSignal(-1))==-1)
   {
      printf("Couldn't get a Signal bit!!\n");
      ReturnCode=20;
      goto cleanup;
   }

   /*** This SHOULD fail if jdisk.device is up and running!! ***/
   /*** djmount used to look in the DOS device lists to see  ***/
   /*** if jh0: was already mounted. I found that jdisk uses ***/
   /*** setup sig if it is running so we assume this call    ***/
   /*** will fail if any unit is mounted!                    ***/
   if((SS=SetupJanusSig(JSERV_HARDDISK,Signal,sizeof(struct HardDskReq),
                        MEMF_PARAMETER|MEM_WORDACCESS))==0)
   {
      printf("Devices already mounted!!\n");
      ReturnCode=20;
      goto cleanup;
   }

/* Check if PC running */
   {  int x;
      for(x=0;x<50;x++)
      {
         if(((struct JanusAmiga *)MakeWordPtr(JanusBase->jb_ParamMem))->ja_HandlerLoaded!=0)
         {
#if DEBUG_PCRUNNING
            printf("PC is running\n");
#endif
            goto PC_Running;
         }
#if DEBUG_WAITPC
         printf("Waiting for PC handler\n");
#endif
         Delay(50);
      }
      printf("PC JanusHandler not running!\n");
      ReturnCode=20;
      goto cleanup;
   }
PC_Running:
   Delay(15*50);  /*** KLUDGE-O-MATIC ***/

   ReqPtr=(struct HardDskReq *)SS->ss_ParamPtr;
#if DEBUG_REQPTR
   printf("ReqPtr @ %lx\n",ReqPtr);
#endif

   ReqPtr->hdr_Fnctn=HDR_FNCTN_INIT;
   SendJanusInt(JSERV_HARDDISK);
   Wait(SS->ss_SigMask);
#if DEBUG_INITERR
   printf("hdr_Err = %ld\n",ReqPtr->hdr_Err);
#endif

#if DEBUG_NUMOFPARTS
   printf("%ld Partitions Found.\n",(ULONG)ReqPtr->hdr_Part);
#endif

   Parts=ReqPtr->hdr_Part;
   if(Parts==0)
   {
      printf("No Partitions found!!\n");
      ReturnCode=20;
      goto cleanup;
   }
   if((PartTable=(struct HDskPartition *)AllocJanusMem(
                                         sizeof(struct HDskPartition)*8,
                                         MEMF_BUFFER|MEM_WORDACCESS))==0)
   {
      printf("Could not alocate memory for PartTable!!\n");
      ReturnCode=20;
      goto cleanup;
   }
#if DEBUG_PARTTABLE
   printf("PartTable @ %lx\n",PartTable);
#endif

   ReqPtr->hdr_BufferAddr=JanusMemToOffset(PartTable);
   for(CPart=0;CPart<Parts;CPart++)
   {
      ReqPtr->hdr_Fnctn=HDR_FNCTN_INFO;
      ReqPtr->hdr_Count=sizeof(struct HDskPartition);
      ReqPtr->hdr_Part=CPart;
      SendJanusInt(JSERV_HARDDISK);
      Wait(SS->ss_SigMask);
      if ( sizeof(struct HDskPartition) != ReqPtr->hdr_Count)
         printf("Count wrong!!!\n");
      ReqPtr->hdr_BufferAddr += sizeof(struct HDskPartition);
   }
   if(SS)   /*** Gota free up the signal so jdisk can fire up below! ***/
   {
      CleanupJanusSig(SS);
      SS=0;
   }

   if((ParmPkt=(ULONG *)AllocMem(PARMPKTSIZE,MEMF_CLEAR))==0)
   {
      printf("ParmPkt memory alloc failed!!\n");
      ReturnCode=20;
      goto cleanup;
   }
   if(FFSFlag)
   {
      if((StupidAssBCPLHName=(UBYTE *)AllocMem(20,MEMF_CLEAR))==0)
      {
         printf("BCPL name string memory alloc failed!!\n");
         ReturnCode=20;
         goto cleanup;
      }
      StupidAssBCPLHName[0]=(UBYTE)16;
      StupidAssBCPLHName[1]='L';
      StupidAssBCPLHName[2]=':';
      StupidAssBCPLHName[3]='F';
      StupidAssBCPLHName[4]='a';
      StupidAssBCPLHName[5]='s';
      StupidAssBCPLHName[6]='t';
      StupidAssBCPLHName[7]='F';
      StupidAssBCPLHName[8]='i';
      StupidAssBCPLHName[9]='l';
      StupidAssBCPLHName[10]='e';
      StupidAssBCPLHName[11]='S';
      StupidAssBCPLHName[12]='y';
      StupidAssBCPLHName[13]='s';
      StupidAssBCPLHName[14]='t';
      StupidAssBCPLHName[15]='e';
      StupidAssBCPLHName[16]='m';
   }

   PartPtr=PartTable;
   for(CPart=0;CPart<Parts;CPart++)
   {
#if DEBUG_PARTINFO
      printf("PartPtr = %lx\n",PartPtr);
      printf("Partition %ld\n",CPart);
      printf("BaseCyl = %ld\n",PartPtr->hdp_BaseCyl);
      printf("EndCyl  = %ld\n",PartPtr->hdp_EndCyl);
      printf("Drive   = %lx\n",PartPtr->hdp_DrvNum);
      printf("Heads   = %ld\n",PartPtr->hdp_NumHeads);
      printf("Secs    = %ld\n",PartPtr->hdp_NumSecs);
#endif

      ParmPkt[0]=(ULONG)name_array[CPart];
      ParmPkt[1]=(ULONG)"jdisk.device";         /* Device name          */
      ParmPkt[2]=CPart;                         /* Device unit #        */
      ParmPkt[3]=0;                             /* Device open flags    */
      ParmPkt[4]=16;                            /* Environment length   */
      ParmPkt[5]=512>>2;                        /* Long words is block  */
      ParmPkt[6]=0;                             /* Sector origin        */
      ParmPkt[7]=PartPtr->hdp_NumHeads;         /* # of surfaces        */
      ParmPkt[8]=1;                             /* Secs/logical block   */
      ParmPkt[9]=PartPtr->hdp_NumSecs;          /* Secs per track       */
      ParmPkt[10]=2;                            /* Reserved blocks      */
      ParmPkt[11]=0;                            /* ?? unused            */
      ParmPkt[12]=0;                            /* Interleave           */
      ParmPkt[13]=0;                            /* Lower Cylinder       */
      ParmPkt[14]=PartPtr->hdp_EndCyl-PartPtr->hdp_BaseCyl-2;
                                                /* Upper Cylinder       */
      ParmPkt[15]=30;                           /* # of buffers         */
      ParmPkt[16]=0;                            /* buff mem type        */
      ParmPkt[17]=0x7FFFFFFF;                   /* Max Transfer         */
      ParmPkt[18]=0;                            /* Mask                 */
      ParmPkt[19]=0;                            /* Boot Pri             */
      if(FFSFlag)
         ParmPkt[20]=0x444F5301;                /* Dos Type FFS         */
      else
         ParmPkt[20]=0x444F5300;                /* Dos Type OldFileSys  */

      if(ParmPkt[14] < 3)
      {
         printf("Partitions must be at least 3 Cyls!!\n");
         ReturnCode=20;
         goto cleanup;
      }

      if((DN=(struct DeviceNode *)MakeDosNode(ParmPkt))==0)
      {
         printf("MakeDosNode failed!!\n");
         ReturnCode=20;
         goto cleanup;
      }
      /*** FFS PATCH ***/
      if(FFSFlag)
      {
         DN->dn_Handler = (((ULONG)StupidAssBCPLHName) >> 2L);
         DN->dn_GlobalVec = 0xffffffff;
         DN->dn_StackSize = 4000L;
      }

      if(AddDosNode(-128,1L,DN)==0)
      {
         printf("AddDosNode failed!!\n");
         ReturnCode=20;
         goto cleanup;
      }

      /* fire up the icon! */
      { struct FileLock *lock;
      if(lock=(struct FileLock *)Lock(name_array1[CPart],ACCESS_READ)) UnLock(lock);
      }


      PartPtr++;
   }

cleanup:

/*** Can't free this up cause device is not accessed until this program ***/
/*** is history ***/
/*
   if(StupidAssBCPLHName) FreeMem(StupidAssBCPLHName,20);
*/
   if(ParmPkt) FreeMem(ParmPkt,PARMPKTSIZE);
   if(PartTable) FreeJanusMem(PartTable,sizeof(struct HDskPartition)*8);
   if(SS) CleanupJanusSig(SS);
   if(Signal!=-1) FreeSignal(Signal);
   if(ExpansionBase) CloseLibrary(ExpansionBase);
   if(JanusBase) CloseLibrary(JanusBase);
   return(ReturnCode);
}
