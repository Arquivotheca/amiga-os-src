
#include <exec/types.h>
#include <stdio.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/envoy_pragmas.h>

#include <clib/exec_protos.h>
#include <clib/envoy_protos.h>

#include <envoy/envoy.h>

struct Library *EnvoyBase;
extern struct Library *SysBase;

BOOL UserReq(ULONG tag1, ...)
{
    return(UserRequestA((struct TagItem *)&tag1));
}

void main(void)
{
	UBYTE UserBuff[32];
	UBYTE GroupBuff[32];

	if(EnvoyBase = OpenLibrary("envoy.library",0L))
	{
	    if(UserReq(UGREQ_UserBuff,UserBuff,
	    	       UGREQ_GroupBuff,GroupBuff,
	    	       UGREQ_UserBuffLen,32,
	    	       UGREQ_GroupBuffLen,32,
	    	       TAG_DONE))
	    {
	    	printf("UserName:  %s\n",UserBuff);
	    	printf("GroupName: %s\n",GroupBuff);
	    }
	    else
	    	printf("Failed!\n");

	    CloseLibrary(EnvoyBase);
	}
}

