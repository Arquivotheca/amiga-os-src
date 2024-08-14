/****************************************************************************
 * ATime.c - ASetTime C Binding & services example
 *
 * 7-25-88 - Bill Koester - New Code
 ***************************************************************************/
#include <dos.h>
#include <janus/janus.h>

#define DBG_GETBASE        0     /* Debug message control                  */
#define DBG_GETSERVICE     0     /* 0 = off, 1 = on                        */
#define DBG_SERVICEDATA    0
#define DBG_CALLSERVICE    0
#define DBG_TSRPTR         0
#define DBG_TSRSTRUCT      0
#define DBG_RELEASESERVICE	0

UBYTE GetService(ULONG,UWORD,UWORD,ULONG,struct ServiceData **);
UBYTE GetBase();

UWORD ServSignal;

main(argc,argv)
int argc;
char *argv[];
{
	UWORD  Flags = 0;
   struct dosdate_t dd;
   struct dostime_t dt;
   struct TimeServReq *tsr;
   int i;
   UBYTE error;
   struct ServiceData *SData;

	if(argc>2)
	{
		printf("Usage: ATime [/w]\n");
		return(1);
	}
	if(argc==2)
	{
		if(strcmpi(argv[1],"/w")!=0)
		{
			printf("Invalid switch\n");
			return(1);
		}
		Flags=GETS_WAIT;
	}

   /************************************************************************/
   /* Make sure INT JFUNC_JINT is redirected (Wake up services) before use */
   /************************************************************************/
   error=GetBase(JSERV_PCDISK,&i,&i,&i);
   if(DBG_GETBASE) printf("Error from  GetBase = %d\n",error);

   error=GetService((ULONG)TIMESERV_APPLICATION_ID,TIMESERV_LOCAL_ID,Flags,0,&SData);
   if(DBG_GETSERVICE) printf("Error from  GetService = %d\n",error);
   if(error!=JSERV_OK) {
      printf("Error! TimeServ service not available. Run TimeServ on Amiga\n");	
      return(1);
   }

   if(DBG_SERVICEDATA) printSD(SData);

   error=CallService(SData);
   if(DBG_CALLSERVICE) printf("Error from CallService = %d\n",error);

   /************************************************************************/
   /* Kludge to wait for timeserv to finish											*/
   /************************************************************************/
	while(SData->sd_Flags&SERVICE_PCWAIT);

   tsr=(struct TimeServReq *)(SData->sd_PCMemPtr );
   if(DBG_TSRPTR) printf("TSR pointer = %lx\n",tsr);

   if(DBG_TSRSTRUCT) {
      printf("Day    = %d\n",tsr->tsr_Day);
      printf("Month  = %d\n",tsr->tsr_Month);
      printf("Year   = %d\n",tsr->tsr_Year);
      printf("Hour   = %d\n",tsr->tsr_Hour);
      printf("Minutes= %d\n",tsr->tsr_Minutes);
      printf("Seconds= %d\n",tsr->tsr_Seconds);
   }

   dd.day 	= tsr->tsr_Day;
   dd.month	= tsr->tsr_Month;
   dd.year	= tsr->tsr_Year;
   dd.dayofweek = 0;
   if(_dos_setdate(&dd)!=0) printf("Error invalid date!\n");

   dt.hour	= tsr->tsr_Hour;
   dt.minute= tsr->tsr_Minutes;
   dt.second= tsr->tsr_Seconds;
   dt.hsecond=0;
   if(_dos_settime(&dt)!=0) printf("Error invalid time!\n");

   tsr->tsr_String[26]='\000';
   printf("%s",tsr->tsr_String);

   error=ReleaseService(SData);
   if(DBG_RELEASESERVICE) printf("Error from ReleaseService = %d\n",error);

}
PrintSD(SData)
struct ServiceData *SData;
{
   printf("ServiceData->ApplicationID = %8lx\n",SData->sd_ApplicationID);	
   printf("ServiceData->LocalID = %4x\n",SData->sd_LocalID);	
   printf("ServiceData->Flags = %4x\n",SData->sd_Flags);	
   printf("ServiceData->ServiceDataLock = %2x\n",SData->sd_ServiceDataLock);	
   printf("ServiceData->PCUserCount = %3d\n",SData->sd_PCUserCount);	
   printf("ServiceData->AmigaUserCount = %3d\n",SData->sd_AmigaUserCount);	
   printf("ServiceData->MemSize = %x\n",SData->sd_MemSize);	
   printf("ServiceData->MemType = %x\n",SData->sd_MemType);	
   printf("ServiceData->MemOffset = %x\n",SData->sd_MemOffset);	
   printf("ServiceData->AmigaMemPtr = %lx\n",SData->sd_AmigaMemPtr);	
   printf("ServiceData->PCMemPtr = %lx\n",SData->sd_PCMemPtr);	
   printf("ServiceData->RememberKey = %x\n",SData->sd_JRememberKey);	
   printf("ServiceData->NextServiceData = %x\n",SData->sd_NextServiceData);	
   printf("ServiceData->FirstPCCustomer = %lx\n",SData->sd_FirstPCCustomer);	
   printf("ServiceData->FirstAmigaCustomer = %lx\n",SData->sd_FirstAmigaCustomer);	
}
Handler()
{
   ServSignal=0;
}
