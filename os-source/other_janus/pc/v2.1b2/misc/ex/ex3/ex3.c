/****************************************************************************
 * Ex3.c - Ex3 C Binding & services example
 *
 * 4-13-89 - Bill Koester - New Code
 ***************************************************************************/
#define LINT_ARGS                /* Enable type checking in Janus incliudes*/

#include <janus/janus.h>
#include "Ex3.h"

#define DBG_GETBASE        1     /* Debug message control                  */
#define DBG_ADDSERVICE     1     /* 0 = off, 1 = on                        */
#define DBG_SERVICEDATA    1
#define DBG_DELETESERVICE  1

main()
{
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

   error=AddService(&SData,(ULONG)EX3_APPLICATION_ID,EX3_LOCAL_ID,0,0,
                    0L,ADDS_EXCLUSIVE|ADDS_TOAMIGA_ONLY|ADDS_FROMAMIGA_ONLY);
   if(DBG_ADDSERVICE) printf("Error from  AddService = "); PError(error);
   if(error!=JSERV_OK) {
      printf("Error! Ex3 service not added\n");	
      return(1);
   }

   printf("ServiceData at %lx\n",SData);
   if(DBG_SERVICEDATA) printSD(SData);

   /************************************************************************/
   /* Wait to be called.                                                   */
   /* Note: In the AddService call above we specified Handler as NULL.     */
   /* There are two ways to know when a service has been called. The first */
   /* way is to provide a pointer to an assembly language routine that will*/
   /* be executed whenever the service is called. The second way, shown    */
   /* here is to wait for the flag SERVICE_PCWAIT in the ServiceData's     */
   /* Flags field. This flag will be set every time YOU do a CallService() */
   /* and will be cleared everytime someone else does a CallService().     */
   /************************************************************************/

   SData->sd_Flags |= SERVICE_PCWAIT;
more:
   while(SData->sd_Flags&SERVICE_PCWAIT);

	if(SData->sd_Flags&EXPUNGE_SERVICE)	/* Someone wants us to expunge	*/
		goto cleanup;
	
   printf("Ex3 gets called!\n");

   /***************
    * Ex3 main loop
    ***************/

   /* Reply */
   CallService(SData);

   if(times++ < 4) 
      goto more;

cleanup:
   error=DeleteService(SData);
   if(DBG_DELETESERVICE) printf("Error from DeleteService = "); PError(error);
}
