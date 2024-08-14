#define LINT_ARGS
#define PRAGMAS

#include <janus/janus.h>
#include "//jlib/serviceint.h"
#include "hardware/custom.h"
#include <stdio.h>
 
extern struct Custom custom;
 
 
struct JanusBase    *JanusBase;
struct ServiceBase  *ServiceBase;
struct ServiceParam *ServiceParam;
struct ServiceParam *baServiceParam;

#if 0
SHORT strlen();
UBYTE getchar();
#endif
 
void ExpungeService(ULONG AppID,USHORT LocalID);
 
UBYTE buffer[256];
 
VOID PrintCustomer(struct ServiceCustomer *customer)
{
   printf("    Customer=$%lx ", customer);
   printf("Next=$%lx ", customer->scs_NextCustomer);
   printf("Flags=$%lx ", customer->scs_Flags);
   printf("Task=$%lx ", customer->scs_Task);
   printf("SigBit=$%lx\n", customer->scs_SignalBit);
}
 
VOID PrintServiceData(struct ServiceData *data)
{
   struct ServiceData *bdata;
   struct ServiceCustomer *customer;
 
   Forbid();
 
   bdata = (struct ServiceData *)MakeBytePtr((APTR)data);

   printf("###############\n");
   printf("# ServiceData #\n");
   printf("###############\n");
   printf("ServiceData=0x%08lx ", bdata);
   printf("ApplicationID=%lu ",    data->sd_ApplicationID);
   printf("LocalID=%lu\n",         data->sd_LocalID);
   printf("\n");
   printf("Flags=",          data->sd_Flags);
   if(data->sd_Flags & SERVICE_DELETED)
      printf("SERVICE_DELETED ");
   if(data->sd_Flags & EXPUNGE_SERVICE)
      printf("EXPUNGE_SERVICE ");
   if(data->sd_Flags & SERVICE_AMIGASIDE)
      printf("SERVICE_AMIGASIDE ");
   if(data->sd_Flags & SERVICE_PCWAIT)
      printf("SERVICE_PCWAIT ");
   if(data->sd_Flags & AMIGA_EXCLUSIVE)
      printf("AMIGA_EXCLUSIVE ");
   if(data->sd_Flags & PC_EXCLUSIVE)
      printf("PC_EXCLUSIVE ");
   if(data->sd_Flags & SERVICE_ADDED)
      printf("SERVICE_ADDED ");
   if(data->sd_Flags & 0xffffff80)
      printf("GARBAGE");
   printf("\n");
   printf("\n");

   printf("ServiceDataLock=0x%02lx ",bdata->sd_ServiceDataLock);
   printf("      AmigaUserCount    =%3lu ",   bdata->sd_AmigaUserCount);
   printf("       PCUserCount=%3lu\n"   ,   bdata->sd_PCUserCount);
   printf("ReservedByte   =0x%02lu "   ,  bdata->sd_ReservedByte);
   printf("\n");

   printf("MemSize        =0x%04lx ",        data->sd_MemSize);
   printf("    MemType           =0x%lx ",        data->sd_MemType);
   printf("    MemOffset=0x%04lx ",      data->sd_MemOffset);
   printf("\n");

   printf("AmigaMemPtr    =0x%08lx ",    data->sd_AmigaMemPtr);
   printf("PCMemPtr          =0x%08lx\n",    data->sd_PCMemPtr);
   printf("JRememberKey   =0x%04lx ",data->sd_JRememberKey);
   printf("    NextServiceData   =0x%04lx ",data->sd_NextServiceData);
   printf("\n");

   printf("FirstPCCustomer=0x%08lx ",  data->sd_FirstPCCustomer);
   printf("FirstAmigaCustomer=0x%08lx\n", data->sd_FirstAmigaCustomer);
   printf("\n");
 
   customer = (struct ServiceCustomer *)data->sd_FirstAmigaCustomer;
   while (customer)
   {
      PrintCustomer(customer);
      customer = (struct ServiceCustomer *)customer->scs_NextCustomer;
   }
   printf("\n");

   printf("Semaphore        =0x%08lx\n",         data->sd_Semaphore);
   printf("ZaphodReserved[0]=0x%08lx ", data->sd_ZaphodReserved[0]);
   printf("ZaphodReserved[1]=0x%08lx\n", data->sd_ZaphodReserved[1]);
   printf("ZaphodReserved[2]=0x%08lx ", data->sd_ZaphodReserved[2]);
   printf("ZaphodReserved[3]=0x%08lx\n", data->sd_ZaphodReserved[3]);
 
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
        ptr = (UBYTE *)(&ServiceBase->sb_ServiceSemaphore);
        PrintBytes(ptr, sizeof(struct SignalSemaphore), "    ");
 
        printf("  ServiceParam=$%lx ", ServiceBase->sb_ServiceParam);
        printf("TaskSigNum=%ld ", ServiceBase->sb_TaskSigNum);
        printf("SetupSig=$%lx ", ServiceBase->sb_SetupSig);
        printf("ServiceTask=$%lx\n", ServiceBase->sb_ServiceTask);
}
 
 
 
VOID PrintServiceDatas()
{
        RPTR rdata;
        struct ServiceData *data;
 
        rdata = ServiceParam->spm_FirstServiceData;
        while (rdata != 0xFFFF)
                {
                data = (struct ServiceData *)JanusOffsetToMem(rdata, 
                                MEMF_PARAMETER | MEM_WORDACCESS);
                PrintServiceData(data);
                rdata = data->sd_NextServiceData;
                }
}
 
 
 
VOID PrintServiceParam()
{
        printf("ServiceParam=$%lx (Janus offset=$%04lx)\n", ServiceParam, 
                        JanusMemToOffset((APTR)ServiceParam));
 
        printf("  Lock=$%02lx\n", baServiceParam->spm_Lock);
 
#ifdef        CHANNELING
        printf("  ChannelMasks=\n");
        PrintBytes(&baServiceParam->spm_ChannelMasks[0], 32, "    ");
 
        printf("  PCToAmiga=\n");
        PrintBytes(&baServiceParam->PCToAmiga[0], SERVICE_IO_FIELDS, "    ");
 
        printf("  AmigaToPC=\n");
        PrintBytes(&baServiceParam->AmigaToPC[0], SERVICE_IO_FIELDS, "    ");
 
        printf("  PCAddsService=$%lx ", baServiceParam->PCAddsService);
        printf("AmigaAddsService=$%lx ", baServiceParam->AmigaAddsService);
#endif        /* of ifdef CHANNELING */
#ifndef        CHANNELING
        printf("  PCToAmiga=            ");
        PrintShorts(&baServiceParam->spm_PCToAmiga[0], SERVICE_IO_FIELDS, "");
 
        printf("  AmigaToPC=            ");
        PrintShorts(&baServiceParam->spm_AmigaToPC[0], SERVICE_IO_FIELDS, "");
 
        printf("  PCAddsService=        ");
        PrintShorts(&ServiceParam->spm_PCAddsService[0], 2, "");
 
        printf("  PCDeletesService=     ");
        PrintShorts(&ServiceParam->spm_PCDeletesService[0], 2, "");
 
        printf("  AmigaAddsService=     ");
        PrintShorts(&ServiceParam->spm_AmigaAddsService[0], 2, "");
 
        printf("  AmigaDeletesService=  ");
        PrintShorts(&ServiceParam->spm_AmigaDeletesService[0], 2, "");
#endif        /* of ifndef CHANNELING */
 
        printf("  FirstServiceData=$%lx\n", ServiceParam->spm_FirstServiceData);
}
 
 
 
VOID PrintJanusBase()
{
        printf("JanusBase=$%lx\n", JanusBase);
 
        printf("  jb_LibNode=\n");
        PrintBytes(&JanusBase->jb_LibNode, sizeof(struct Library), "    ");
 
        printf("  jb_IntReq=$%lx ", JanusBase->jb_IntReq);
        printf("jb_IntEna=$%lx ", JanusBase->jb_IntEna);
        printf("jb_ParamMem=$%lx ", JanusBase->jb_ParamMem);
        printf("jb_IoBase=$%lx\n  ", JanusBase->jb_IoBase);
        printf("jb_ExpanBase=$%lx ", JanusBase->jb_ExpanBase);
        printf("jb_ExecBase=$%lx ", JanusBase->jb_ExecBase);
        printf("jb_DOSBase=$%lx\n  ", JanusBase->jb_DOSBase);
        printf("jb_SegList=$%lx ", JanusBase->jb_SegList);
        printf("jb_IntHandlers=$%lx\n", JanusBase->jb_IntHandlers);
 
        printf("  jb_IntServer=\n");
        PrintBytes(&JanusBase->jb_IntServer, sizeof(struct Interrupt), "    ");
 
        printf("  jb_ReadHandler=\n");
        PrintBytes(&JanusBase->jb_ReadHandler, sizeof(struct Interrupt), "    ");
 
        printf("  jb_KeyboardRegisterOffset=$%lx ", JanusBase->jb_KeyboardRegisterOffset);
        printf("jb_ATFlag=$%lx ", JanusBase->jb_ATFlag);
        printf("jb_ATOffset=$%lx\n  ", JanusBase->jb_ATOffset);
        printf("jb_ServiceBase=$%lx\n", JanusBase->jb_ServiceBase);
}
 
 
 
VOID PrintFunctions()
{
   printf("\n");
   printf("********************************************\n");
   printf("* b=print ServiceBase d=print ServiceDatas *\n");
   printf("* j=print JanusBase   p=print ServiceParam *\n");
   printf("* q=quit              s=signal ServiceTask *\n");
   printf("*                                          *\n");
   printf("* z=Amiga/PCAdds/DeletesService[] to zero  *\n");
   printf("* e AppID LocalID = ExpungeService         *\n");
   printf("* enter function code and any args.        *\n");
   printf("********************************************\n");
   printf("COMMAND->");
}
 
 
 
BOOL ServicesPeek()
{
        BOOL retvalue, printfuncs;
        UBYTE c;
		char buf[512];
 
        retvalue = printfuncs = TRUE;

 		gets(buf);
/*         c = getchar(); */
		fprintf(stdout,"\014");
        switch (buf[0])
                {
                case 'b':
                        PrintServiceBase();
                        break;
                case 'e':
				        {
						   long AppID,LocalID;

						   sscanf(buf,"%*s %ld %ld\n",&AppID,&LocalID);
                           ExpungeService(AppID,LocalID);
				           break;
						}
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
                        /* do c = getchar(); while (c != '\n'); */
                        printfuncs = FALSE;
                        retvalue = FALSE;
                        break;
                case 's':
                        Signal(ServiceBase->sb_ServiceTask, 
                                        1 << ServiceBase->sb_TaskSigNum);
                        break;
                case 'z':
                        ServiceParam->spm_AmigaAddsService[0] = 0xFFFF;
                        ServiceParam->spm_AmigaAddsService[1] = 0xFFFF;
                        ServiceParam->spm_AmigaDeletesService[0] = 0xFFFF;
                        ServiceParam->spm_AmigaDeletesService[1] = 0xFFFF;
                        ServiceParam->spm_PCAddsService[0] = 0xFFFF;
                        ServiceParam->spm_PCAddsService[1] = 0xFFFF;
                        ServiceParam->spm_PCDeletesService[0] = 0xFFFF;
                        ServiceParam->spm_PCDeletesService[1] = 0xFFFF;
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
   JanusBase = (struct JanusBase *) OpenLibrary("janus.library", 0);
   printf("JanusBase @ 0x%lx\n", JanusBase);
   if (!JanusBase) exit(0);
 
   ServiceBase = JanusBase->jb_ServiceBase;
   printf("ServiceBase = 0x%lx\n", ServiceBase);
   if (!ServiceBase) goto cleanup;
 
   ServiceParam = ServiceBase->sb_ServiceParam;
   printf("ServiceParam = 0x%lx\n", ServiceParam);
   if (!ServiceParam) goto cleanup;
   baServiceParam = (struct ServiceParam *)MakeBytePtr((APTR)ServiceParam);
 
 
   PrintFunctions();
   while (ServicesPeek()) ;

cleanup:
   CloseLibrary(JanusBase);
}
void ExpungeService(ULONG AppID,USHORT LocalID)
{
   struct ServiceData *SDb;
   struct ServiceData *SDw;
   long   ServiceSignal;

   ServiceSignal=AllocSignal(-1);
   if(ServiceSignal==-1)
   {
      fprintf(stderr,"ExpungeService: Could not get signal!\n");
	  return;
   }

   if(GetService(&SDb,AppID,LocalID,ServiceSignal,0)!=JSERV_OK)
   {
      fprintf(stderr,"ExpungeService: Could not GetService(%ld,%ld)!\n",
	                 AppID,LocalID);
	  FreeSignal(ServiceSignal);
	  return;
   }
   SDw=MakeWordPtr(SDb);
   SDw->sd_Flags |= EXPUNGE_SERVICE;
   CallService(SDb);
   ReleaseService(SDb);
   FreeSignal(ServiceSignal);
}
