/****************************************************************************
 * Ex5.c - Ex5 C Binding & services example
 *
 * 9-12-89 - Bill Koester - New Code
 ***************************************************************************/
#define LINT_ARGS                /* Enable type checking in Janus incliudes*/

#include <janus/janus.h>
#include "Ex5.h"

#define DBG_GETBASE        1     /* Debug message control                  */
#define DBG_ADDSERVICE     1     /* 0 = off, 1 = on                        */
#define DBG_SERVICEDATA    1
#define DBG_EXRPTR         1
#define DBG_DELETESERVICE  1

main()
{
	struct Ex5Req *exr;
   UBYTE  error;
   struct ServiceData *SData;
   int    times = 0;

   /************************************************************************/
   /* To use ANY of the Janus C functions or assembly int 0x0B calls you   */
   /* MUST first make sure that the int 0x0B vector has been redirected.   */
   /* To do this, simply call one of the int 0x0B functions as a dummy call*/
   /* BEFORE expecting valid results. The first call will redirect the     */
   /* vector so that subsequent calls will return valid data. This is      */
   /* called 'Waking up services'.                                         */
   /************************************************************************/

   /************************************************************************/
   /* Make sure INT JFUNC_JINT is redirected (Wake up services) before use */
   /************************************************************************/

   { UWORD i;
      error=GetBase(JSERV_PCDISK,&i,&i,&i);
      if(DBG_GETBASE) printf("Error from  GetBase = "); PError(error);
   }

   error=AddService(&SData,(ULONG)EX5_APPLICATION_ID,EX5_LOCAL_ID,
                    sizeof(struct Ex5Req),MEMF_BUFFER,
                    0L,
          ADDS_LOCKDATA|ADDS_EXCLUSIVE|ADDS_TOAMIGA_ONLY|ADDS_FROMAMIGA_ONLY);
   if(DBG_ADDSERVICE) printf("Error from  AddService = "); PError(error);
   if(error!=JSERV_OK) {
      printf("Error! Ex5 service not added\n");	
      return(1);
   }

   printf("ServiceData at %lx\n",SData);
   if(DBG_SERVICEDATA) printSD(SData);

   exr=(struct Ex5Req *)(SData->sd_PCMemPtr );
   if(DBG_EXRPTR) printf("EXR pointer = %lx\n",exr);

	JanusInitLock(&exr->exr_Lock);

	/* ADDS_LOCKDATA was specified in AddService() to automatically lock */
   /* the ServiceData lock so that no one could	CallService() us before */
   /* our initialization. So after our initialization we must 				*/
   /* UnLockServiceData() to let others begin to call us. 					*/
	UnLockServiceData(SData);
more:
	JanusLock(&exr->exr_Lock);
	times++;
	if(times>1000)
	{
		times=0;
		printf(". ");
	}
	JanusUnlock(&exr->exr_Lock);
   goto more;

   error=DeleteService(SData);
   if(DBG_DELETESERVICE) printf("Error from DeleteService = "); PError(error);
}
