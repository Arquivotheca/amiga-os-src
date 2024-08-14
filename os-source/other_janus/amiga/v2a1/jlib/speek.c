
/* **************************************************************************
 *
 * Services Peek Program  --  Dunps status info about Amiga Services
 *
 * Copyright (C) 1988, =Robert J. Mical=
 * All Rights Reserved.
 *
 * HISTORY      NAME            DESCRIPTION
 * -----------  --------------  --------------------------------------------
 * 1 Apr 88     =RJ Mical=      Created this file.
 *
 * *********************************************************************** */



#include "exec/types.h"
#include "exec/nodes.h"
#include "exec/lists.h"
#include "exec/ports.h"
#include "exec/memory.h"
#include "exec/interrupts.h"

#include "hardware/custom.h"
#include "hardware/intbits.h"

#include "janus/janus.h"
#include "janus/janusvar.h"
#include "janus/janusreg.h"
#include "janus/memrw.h"

#define SERVICE_TYPES

#include "janus/services.h"
#include "serviceint.h"


extern struct Custom custom;


UBYTE *GetJanusStart();


struct JanusAmiga *JanusBase;
struct ServiceBase *ServiceBase;
struct ServiceParam *ServiceParam;
struct ServiceParam *baServiceParam;

SHORT strlen();
UBYTE getchar();


UBYTE buffer[256];



VOID PrintCustomer(customer)
struct   ServiceCustomer *customer;
{
   printf("    Customer=$%lx ", customer);
   printf("Next=$%lx ", customer->NextCustomer);
   printf("Flags=$%lx ", customer->Flags);
   printf("Task=$%lx ", customer->Task);
   printf("SigBit=$%lx\n", customer->SignalBit);
}



VOID PrintServiceData(data)
struct ServiceData *data;
{
   struct ServiceData *bdata;
   struct ServiceCustomer *customer;

   Forbid();

   data = (struct ServiceData *)MakeWordPtr(data);
   bdata = (struct ServiceData *)MakeBytePtr(data);

   printf("ServiceData=$%lx\n  ", data);
   printf("ApplicationID=$%lx ", data->ApplicationID);
   printf("LocalID=$%lx ", data->LocalID);
   printf("Flags=$%lx ", data->Flags);
#ifdef   CHANNELING
   printf("ChannelNumber=%ld\n  ", bdata->ChannelNumber);
#endif   /* of ifdef CHANNELING */
#ifndef   CHANNELING
   printf("ServiceDataLock=$%02lx\n  ", bdata->ServiceDataLock);
#endif   /* of ifndef CHANNELING */
   printf("UserCount=%ld ", bdata->UserCount);
   printf("MemSize=%ld ", data->MemSize);
   printf("MemType=$%lx ", data->MemType);
   printf("AmigaMemPtr=$%lx ", data->AmigaMemPtr);
   printf("PCMemPtr=$%lx\n  ", data->PCMemPtr);
   printf("NextServiceData=$%lx ", data->NextServiceData);
   printf("FirstAmigaCustomer=$%lx\n", data->FirstAmigaCustomer);

   customer = (struct ServiceCustomer *)data->FirstAmigaCustomer;
   while (customer)
      {
      PrintCustomer(customer);
      customer = customer->NextCustomer;
      }

   Permit();
}



VOID PrintBytes(ptr, len, leader)
UBYTE *ptr;
SHORT len;
UBYTE *leader;
{
   SHORT i;

   printf("%s", leader);
   for (i = 0; i < len; i++)
      {
      printf("%02lx ", *ptr++);
      if ( ((i & 0x7) == 7) && (i != len - 1))
         printf("\n%s", leader);
      }
   printf("\n");
}



VOID PrintShorts(ptr, len, leader)
USHORT *ptr;
SHORT len;
UBYTE *leader;
{
   SHORT i;

   printf("%s", leader);
   for (i = 0; i < len; i++)
      {
      printf("%04lx ", *ptr++);
      if ( ((i & 0x3) == 3) && (i != len - 1))
         printf("\n%s", leader);
      }
   printf("\n");
}



VOID PrintServiceBase()
{
   UBYTE *ptr;

   printf("ServiceBase=%lx\n", ServiceBase);
   printf("  ServiceSemaphore = \n");
   ptr = (UBYTE *)(&ServiceBase->ServiceSemaphore);
   PrintBytes(ptr, sizeof(struct SignalSemaphore), "    ");

   printf("  ServiceParam=$%lx ", ServiceBase->ServiceParam);
   printf("TaskSigNum=%ld ", ServiceBase->TaskSigNum);
   printf("SetupSig=$%lx ", ServiceBase->SetupSig);
   printf("ServiceTask=$%lx\n", ServiceBase->ServiceTask);
}



VOID PrintServiceDatas()
{
   RPTR rdata;
   struct ServiceData *data;

   rdata = ServiceParam->FirstServiceData;
   while (rdata != 0xFFFF)
      {
      data = (struct ServiceData *)JanusOffsetToMem(rdata, 
            MEMF_PARAMETER | MEM_WORDACCESS);
      PrintServiceData(data);
      rdata = data->NextServiceData;
      }
}



VOID PrintServiceParam()
{
   printf("ServiceParam=$%lx (Janus offset=$%04lx)\n", ServiceParam, 
         JanusMemToOffset(ServiceParam));

   printf("  Lock=$%02lx\n", baServiceParam->Lock);

#ifdef   CHANNELING
   printf("  ChannelMasks=\n");
   PrintBytes(&baServiceParam->ChannelMasks[0], 32, "    ");

   printf("  PCToAmiga=\n");
   PrintBytes(&baServiceParam->PCToAmiga[0], SERVICE_IO_FIELDS, "    ");

   printf("  AmigaToPC=\n");
   PrintBytes(&baServiceParam->AmigaToPC[0], SERVICE_IO_FIELDS, "    ");

   printf("  PCAddsService=$%lx ", baServiceParam->PCAddsService);
   printf("AmigaAddsService=$%lx ", baServiceParam->AmigaAddsService);
#endif   /* of ifdef CHANNELING */
#ifndef   CHANNELING
   printf("             PCToAmiga= ");
   PrintShorts(&baServiceParam->PCToAmiga[0], SERVICE_IO_FIELDS, "");

   printf("             AmigaToPC= ");
   PrintShorts(&baServiceParam->AmigaToPC[0], SERVICE_IO_FIELDS, "");

   printf("         PCAddsService= ");
   PrintShorts(&ServiceParam->PCAddsService[0], 2, "");

   printf("      PCDeletesService= ");
   PrintShorts(&ServiceParam->PCDeletesService[0], 2, "");

   printf("      AmigaAddsService= ");
   PrintShorts(&ServiceParam->AmigaAddsService[0], 2, "");

   printf("   AmigaDeletesService= ");
   PrintShorts(&ServiceParam->AmigaDeletesService[0], 2, "");
#endif   /* of ifndef CHANNELING */

   printf("  FirstServiceData=$%lx\n", ServiceParam->FirstServiceData);
}



VOID PrintJanusBase()
{
   printf("JanusBase=$%lx\n", JanusBase);

   printf("  ja_LibNode=\n");
   PrintBytes(&JanusBase->ja_LibNode, sizeof(struct Library), "    ");

   printf("  ja_IntReq=$%lx ", JanusBase->ja_IntReq);
   printf("ja_IntEna=$%lx ", JanusBase->ja_IntEna);
   printf("ja_ParamMem=$%lx ", JanusBase->ja_ParamMem);
   printf("ja_IoBase=$%lx\n  ", JanusBase->ja_IoBase);
   printf("ja_ExpanBase=$%lx ", JanusBase->ja_ExpanBase);
   printf("ja_ExecBase=$%lx ", JanusBase->ja_ExecBase);
   printf("ja_DOSBase=$%lx\n  ", JanusBase->ja_DOSBase);
   printf("ja_SegList=$%lx ", JanusBase->ja_SegList);
   printf("ja_IntHandlers=$%lx\n", JanusBase->ja_IntHandlers);

   printf("  ja_IntServer=\n");
   PrintBytes(&JanusBase->ja_IntServer, sizeof(struct Interrupt), "    ");

   printf("  ja_ReadHandler=\n");
   PrintBytes(&JanusBase->ja_ReadHandler, sizeof(struct Interrupt), "    ");

   printf("  ja_KeyboardRegisterOffset=$%lx ", JanusBase->ja_KeyboardRegisterOffset);
   printf("ja_ATFlag=$%lx ", JanusBase->ja_ATFlag);
   printf("ja_ATOffset=$%lx\n  ", JanusBase->ja_ATOffset);
   printf("ja_ServiceBase=$%lx\n", JanusBase->ja_ServiceBase);
}



VOID PrintFunctions()
{
   printf(
"* b=print ServiceBase   d=print ServiceDatas   j=print JanusBase\n");
   printf(
"* p=print ServiceParam   q=quit   s=signal ServiceTask\n");
   printf(
"* z=Amiga/PCAdds/DeletesService[] to zero\n");
   printf(
"* enter function code and any args, then hit return\n");
}



BOOL ServicesPeek()
{
   BOOL retvalue, printfuncs;
   UBYTE c;

   retvalue = printfuncs = TRUE;

   c = getchar();
   switch (c)
      {
      case 'b':
         PrintServiceBase();
         break;
      case 'd':
         PrintServiceDatas();
         break;
      case 'j':
         PrintJanusBase();
         break;
      case 'p':
         PrintServiceParam();
         break;
      case 'q':
         do c = getchar(); while (c != '\n');
         printfuncs = FALSE;
         retvalue = FALSE;
         break;
      case 's':
         Signal(ServiceBase->ServiceTask, 
               1 << ServiceBase->TaskSigNum);
         break;
      case 'z':
#ifdef   CHANNELING
         baServiceParam->AmigaAddsService = 0;
#endif   /* of ifdef CHANNELING */
#ifndef   CHANNELING
         ServiceParam->AmigaAddsService[0] = 0xFFFF;
         ServiceParam->AmigaAddsService[1] = 0xFFFF;
         ServiceParam->AmigaDeletesService[0] = 0xFFFF;
         ServiceParam->AmigaDeletesService[1] = 0xFFFF;
         ServiceParam->PCAddsService[0] = 0xFFFF;
         ServiceParam->PCAddsService[1] = 0xFFFF;
         ServiceParam->PCDeletesService[0] = 0xFFFF;
         ServiceParam->PCDeletesService[1] = 0xFFFF;
#endif   /* of ifndef CHANNELING */
         break;
      case '\n':
         printfuncs = FALSE;
         break;
      default:
         break;
      }

   if (printfuncs) PrintFunctions();
   return(retvalue);
}



VOID main(argc, argv)
int argc;
char **argv;
{
   JanusBase = (struct JanusAmiga *) OpenLibrary("janus.library", 0);
   printf("janus.library = 0x%lx\n", JanusBase);
   if (!JanusBase) exit(0);

   ServiceBase = JanusBase->ja_ServiceBase;
   printf("ServiceBase = 0x%lx\n", ServiceBase);
   if (!ServiceBase) exit(0);

   ServiceParam = ServiceBase->ServiceParam;
   printf("ServiceParam = 0x%lx\n", ServiceParam);
   if (!ServiceParam) exit(0);
   ServiceParam = (struct ServiceParam *)MakeWordPtr(ServiceParam);
   baServiceParam = (struct ServiceParam *)MakeBytePtr(ServiceParam);

   PrintFunctions();
   while (ServicesPeek()) ;

   CloseLibrary(JanusBase);
   exit(0);
}


