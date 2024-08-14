/*
**  This test program can be used in place of services manger to call
**  StartService().  This is useful for testing new services.  In particular
**  you might wish to test the expungability of your service after a call
**  to StartService() which isn't folled by a connection from a client.
**
**  ONLY SERVICES MANAGER SHOULD DIRECTLY CALL StartService().  DO NOT
**  EVER DO THIS IN ANY OF YOUR OWN CODE.
**
**  Your service's preferences code or utilities may call your own service's
**  GetServiceAttrs() or SetServiceAttrs(), but should not call any other
**  service's.
*/
#include <exec/types.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <envoy/nipc.h>
#include <stdio.h>
#include <envoy/services.h>

ULONG StartService(STRPTR userName, STRPTR password, STRPTR name);
VOID GetServiceAttrsA(struct TagItem *tagList);
#pragma libcall SvcBase RexxReserved 1e 00
#pragma libcall SvcBase StartService 24 A9803
#pragma libcall SvcBase GetServiceAttrsA 2a 801

extern struct Library *SysBase;
struct Library *SvcBase;

main()
{
ULONG error;
char buffer[64];
char buffer2[64];
struct TagItem attrstags[] =
{
	{SVCAttrs_Name, },
	{TAG_DONE}
};

	SvcBase = OpenLibrary("utility.library", 0L);
	printf("tilitybase=%lx\n", SvcBase);
	CloseLibrary(SvcBase);
	SvcBase = OpenLibrary("erc.service", 0L);
	error = StartService(NULL, NULL, buffer);
	if(error)
	{
		printf("Service not started -- error %ld.\n", error);
	}else
	{
		printf("Service started and waiting for Transactions "
		       "on Entity '%s.'\n", buffer);
	}
	attrstags[0].ti_Data = (LONG)buffer2;
	printf("attrstags=%lx, buffer2=%lx\n", attrstags, buffer2);
	GetServiceAttrsA(attrstags);
	printf("SRCAttrs_Name is '%s'.\n", buffer2);
	CloseLibrary(SvcBase);
}
