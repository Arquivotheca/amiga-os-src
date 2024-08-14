/****** ljanus.lib/PrintSD ******************************************
*
*   NAME   
*		PrintSD -- Print out a ServiceData structure.
*
*   SYNOPSIS
*		PrintSD(ServiceData);
*
*		VOID PrintSD(struct ServiceData *);
*
*   FUNCTION
*		Prints out all the fields of a ServiceData structure. Useful for
*		debugging service programs.
*
*   INPUTS
*		ServiceData - Pointer to the ServiceData structure to be printed.
*
*   RESULT
*		The ServiceData structure is printed to stdout.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*		PError()
*
*****************************************************************************
*
*/
#define LINT_ARGS
#include	<janus/janus.h>

unsigned long stswap(unsigned long);
PrintSD(SData)
struct ServiceData *SData;
{

	printf("Dump of ServiceData structure at 0x%lx\n",SData);
	printf("-------------------------------------------\n");
   printf("|sd_ApplicationID   = 0x%8.8lx  sd_LocalID            = 0x%4.4x\n",stswap(SData->sd_ApplicationID),SData->sd_LocalID);	
   printf("|sd_Flags           = 0x%4.4x      sd_ServiceDataLock    = 0x%2.2x\n",SData->sd_Flags,SData->sd_ServiceDataLock);	
   printf("|sd_PCUserCount     = %3.3d         sd_AmigaUserCount     = %3.3d\n",SData->sd_PCUserCount,SData->sd_AmigaUserCount);	
   printf("|sd_MemSize = 0x%4.4x  sd_MemType = 0x%4.4x  sd_MemOffset = 0x%4.4x\n",SData->sd_MemSize,SData->sd_MemType,SData->sd_MemOffset);	
   printf("|sd_AmigaMemPtr     = 0x%8.8lx  sd_PCMemPtr           = 0x%8.8lx\n",(SData->sd_AmigaMemPtr),SData->sd_PCMemPtr);	
   printf("|sd_RememberKey     = 0x%4.4x\n",SData->sd_JRememberKey);	
   printf("|sd_NextServiceData = 0x%4.4x\n",SData->sd_NextServiceData);	
   printf("|sd_FirstPCCustomer = 0x%8.8lx  sd_FirstAmigaCustomer = 0x%8.8lx\n",SData->sd_FirstPCCustomer,(SData->sd_FirstAmigaCustomer));	
   printf("|sd_Semaphore       = 0x%8.8lx\n",(SData->sd_Semaphore));
}
unsigned long stswap(num)
unsigned long num;
{
	short hi,low;
	unsigned long  result =0;

	low = num & 0xffff;
	hi = num >>16;

	result= low;
	result = result <<16;
	result |= hi;
	return(result);

	
}
