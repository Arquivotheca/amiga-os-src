/****** ljanus.lib/WaitService ******************************************
*
*   NAME   
*		WaitService -- Wait for a service request to complete.
*
*   SYNOPSIS
*		WaitService(ServiceData);
*
*		VOID WaitService(struct ServiceData *);
*
*   FUNCTION
*		Waits for someone else to CallService() this service.
*
*   INPUTS
*		ServiceData - Pointer to the ServieData structure of the service you
*						  wish to wait for.
*
*   RESULT
*		Returns when this service has been called.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*		AddService(), CallService(), DeleteService(), GetService()
*		ReleaseService()
*
*****************************************************************************
*
*/
#define LINT_ARGS
#include <dos.h>
#include <janus\janus.h>

VOID WaitService(ServiceData)
struct ServiceData *ServiceData;
{
	while(ServiceData->sd_Flags&SERVICE_PCWAIT);
}
