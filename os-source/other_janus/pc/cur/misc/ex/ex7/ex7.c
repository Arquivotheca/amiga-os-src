/****************************************************************************
 * Ex7.c - Ex7 C Binding & services example
 *
 * This program finds the Amiga Ex7 service if available and
 * calls the service once then exits.
 *
 * 4-13-89 - Bill Koester - New Code
 ***************************************************************************/
#define LINT_ARGS                   /* Enable Janus type checking          */

#include <janus/janus.h>
#include "Ex7.h"

#define DBG_GETBASE        1        /* Debug message control              */
#define DBG_GETSERVICE     1        /* 0 = off, 1 = on                    */
#define DBG_SERVICEDATA    1
#define DBG_EXRPTR         1
#define DBG_CALLSERVICE    1
#define DBG_EXRSTRUCT      1
#define DBG_RELEASESERVICE 1


main(argc,argv)
int argc;
char *argv[];
{
   UWORD  Flags = 0;                /* Flags for GetService()              */
   UBYTE  error;                    /* Generic error variable              */
   struct ServiceData *SData;       /* Our ServiceData pointer             */
	APTR	 ptr1,ptr2,ptr3;

   if(argc>2)                       /* print usage info                    */
   {
      printf("Usage: Ex7 [/w]\n");
      return(1);
   }
   if(argc==2)                      /* Check for /w wait option            */
   {
      if(strcmpi(argv[1],"/w")!=0)
      {
         printf("Invalid switch\n");
         return(1);
      }
      Flags=GETS_WAIT;              /* Set GETS_WAIT flag so that          */
   }                                /* GetService() will wait for          */
                                    /* Ex7 to become available             */

   /************************************************************************/
   /* To use ANY of the Janus C functions or assembly int 0x0B calls you   */
   /* MUST first make sure that the int 0x0B vector has been redirected    */
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

   /************************************************************************/
   /* Get the Amiga side service. If the service is available SData will   */
   /* contain a valid pointer to a ServiceData structure on exit. If the   */
   /* service is NOT available the call to GetService() will fail UNLESS   */
   /* the GETS_WAIT flag was set, in which case the GetService() call will */
   /* wait (possibly forever) until the service DOES become available.     */
   /************************************************************************/
   error=GetService(&SData,(ULONG)EX7_APPLICATION_ID,(UWORD)EX7_LOCAL_ID,
                    (ULONG)0,(UWORD)Flags);
   if(DBG_GETSERVICE) printf("Error from  GetService = "); PError(error);
   if(error!=JSERV_OK) {
      printf("Error! Ex7 service not available. Run Ex7 on Amiga\n");	
      return(1);
   }

   if(DBG_SERVICEDATA) printSD(SData);

   /**************************/
   /* Call the Amiga service */
   /**************************/

	printf("sd_JRememberKey = %lx\n",(ULONG)SData->sd_JRememberKey);

	ptr1=AllocServiceMem(SData,100,MEMF_BUFFER);
	printf("ptr1 = %lx\n",ptr1);

	ptr2=AllocServiceMem(SData,100,MEMF_PARAMETER);
	printf("ptr2 = %lx\n",ptr2);

	ptr3=AllocServiceMem(SData,100,MEMF_BUFFER);
	printf("ptr3 = %lx\n",ptr3);

	printf("sd_JRememberKey = %lx\n",(ULONG)SData->sd_JRememberKey);
	
	printf("Press return to FreeServiceMem(ALL)\n");
	{ char buf[255]; gets(buf); }

	FreeServiceMem(SData,0);
	printf("sd_JRememberKey = %lx\n",(ULONG)SData->sd_JRememberKey);

	printf("Press realloc 1-3\n");
	{ char buf[255]; gets(buf); }

	ptr1=AllocServiceMem(SData,100,MEMF_BUFFER);
	printf("ptr1 = %lx\n",ptr1);

	ptr2=AllocServiceMem(SData,100,MEMF_BUFFER);
	printf("ptr2 = %lx\n",ptr2);

	ptr3=AllocServiceMem(SData,100,MEMF_BUFFER);
	printf("ptr3 = %lx\n",ptr3);

	printf("sd_JRememberKey = %lx\n",(ULONG)SData->sd_JRememberKey);

	printf("Press return to FreeServiceMem(ptr1)\n");
	{ char buf[255]; gets(buf); }

	FreeServiceMem(SData,ptr1);
	printf("sd_JRememberKey = %lx\n",(ULONG)SData->sd_JRememberKey);

	printf("Press return to ReleaseService()\n");
	{ char buf[255]; gets(buf); }

	goto cleanup;

   error=CallService(SData);
   if(DBG_CALLSERVICE) printf("Error from CallService = "); PError(error);

   /************************************************************************/
   /* Wait for Ex7 to respond                                              */
   /* Note: In the GetService call above we specified Handler as NULL.     */
   /*       There are two ways to know when a service has been called. The */
   /*       first way is to provide a pointer to an assembly language      */
   /*       routine that will be executed whenever the service is called   */
   /*       The second way, shown here, is to wait for the flag            */
   /*       SERVICE_PCWAIT in the ServiceData's Flags field. This flag will*/
   /*       be set every time YOU do a CallService() and will be cleared   */
   /*       every time someone else does a CallService()                   */
   /************************************************************************/
   while(SData->sd_Flags&SERVICE_PCWAIT);


   /***************************************************************/
   /* Be polite and ReleaseService() so the service can go away.  */
   /***************************************************************/
cleanup:
   error=ReleaseService(SData);
   if(DBG_RELEASESERVICE) printf("Error from ReleaseService = "); PError(error);
}
