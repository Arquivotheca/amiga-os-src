/****************************************************************************
 * ATime.c - ASetTime C Binding & services example
 *
 * 7-25-88 - Bill Koester - New Code
 * 03/07/91	rsd	wait for timeserv to unlock servicedata
 ***************************************************************************/
#define LINT_ARGS
#include <dos.h>
#include <janus/janus.h>

#define D(x) ;
#define PSD 0

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

	if (argc > 1)
	{
		printf("Usage: ATime\n");
		return(1);
	}

  	Flags=GETS_WAIT|GETS_ALOAD_A;

   /************************************************************************/
   /* Make sure INT JFUNC_JINT is redirected (Wake up services) before use */
   /************************************************************************/
   error=GetBase(JSERV_PCDISK,&i,&i,&i);
   D(printf("Error from  GetBase = %d\n",error);)

   error=GetService(&SData,(ULONG)TIMESERV_APPLICATION_ID,TIMESERV_LOCAL_ID,
			0,Flags);
   D(printf("Error from  GetService = %d\n",error);)
   if(error!=JSERV_OK) {
      printf("Amiga could not load TimeServ - ATIME failed.\n");	
      return(1);
   }

   D(printSD(SData);)

	/* wait for him to be *really* ready (when he unlocks his sd) */
	LockServiceData(SData);
	UnlockServiceData(SData);

   error=CallService(SData);
   D(printf("Error from CallService = %d\n",error);)

   /************************************************************************/
   /* Kludge to wait for timeserv to finish											*/
   /************************************************************************/
/*
	while(SData->sd_Flags&SERVICE_PCWAIT);
*/
	WaitService(SData);

   tsr=(struct TimeServReq *)(SData->sd_PCMemPtr );
   D(printf("TSR pointer = %lx\n",tsr);)

      D(printf("Day    = %d\n",tsr->tsr_Day);)
      D(printf("Month  = %d\n",tsr->tsr_Month);)
      D(printf("Year   = %d\n",tsr->tsr_Year);)
      D(printf("Hour   = %d\n",tsr->tsr_Hour);)
      D(printf("Minutes= %d\n",tsr->tsr_Minutes);)
      D(printf("Seconds= %d\n",tsr->tsr_Seconds);)

   dd.day 	= tsr->tsr_Day;
   dd.month	= tsr->tsr_Month;
   dd.year	= tsr->tsr_Year;
   dd.dayofweek = 0;
   if(_dos_setdate(&dd)!=0) printf("Amiga date not valid for MS-DOS.\n");

   dt.hour	= tsr->tsr_Hour;
   dt.minute= tsr->tsr_Minutes;
   dt.second= tsr->tsr_Seconds;
   dt.hsecond=0;
   if(_dos_settime(&dt)!=0) printf("Amiga time not valid for MS-DOS.\n");

   tsr->tsr_String[26]='\000';
   printf("%s",tsr->tsr_String);

   error=ReleaseService(SData);
   D(printf("Error from ReleaseService = %d\n",error);)

}

#if PSD
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
#endif
