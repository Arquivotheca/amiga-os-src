/*
**  $Id: configure.c,v 1.3 92/01/02 14:22:15 dlarson Exp $
**
**  SANA-II driver configuration example.
**
**  Copyright 1991 Commodore-Amiga, Inc.
**
**  This code may be modified and used freely on Amiga computers.
**
**
**  This example is pretty ugly and should be cleaned up.  Sorry.
**  Still, you should be able to see the proper steps to take
**  in configuring a SANA-II device.
*/

#define NOBUFFS
#include "/skeleton/skeleton.h"

struct MsgPort   *DevPort = NULL;
struct IOSana2Req *IOB1    = NULL;

int	DeviceOpen = 0;
long	DevBits;
int	NumBytes;

void DoConfig(UBYTE *Address);
void DoGetAddr(void);
void DoQuery(struct Sana2DeviceQuery *q);
void DoPrintAddr(void);


main(int argc, char *argv[])
{
int i;
long temp;
struct Sana2DeviceQuery Query;

        if(!initdev())
        {
        	printf("Open Failed.\n");
        	goto bottom;
        }
        printf("About to Query the device.\n");
	Query.SizeAvailable = sizeof(Query);
	DoQuery(&Query);
	if(IOB1->ios2_Req.io_Error)
	{
            	printf("ERROR RETURNED: %ld\n", (long)IOB1->ios2_Req.io_Error);
		goto bottom;
     	}
    	if(Query.AddrFieldSize%8 != 0 || Query.AddrFieldSize == 0)
	{
		printf("Oh, no!  There really is funny hardware in the world!\n"
		       "Address field is %d bits.  This example needs to be\n"
		       "updated to understand address fields with a number of\n"
		       "bits not divisible by eight.\n", Query.AddrFieldSize);
		goto bottom;
	}
	NumBytes = Query.AddrFieldSize/8;
	if(argc>1)
	{
		for(i=0; i<NumBytes; i++)
		{
			sscanf(argv[1]+(i*3), "%lx", &temp);
			IOB1->ios2_SrcAddr[i] = (char)temp;
		}
	}else
	{
		printf("\nNo user address, getting ROM address.\n");
		DoGetAddr();
		if (IOB1->ios2_Req.io_Error)
		{
			printf("ERROR RETURNED: 0x%lx\n",
               			(long)IOB1->ios2_Req.io_Error);
			goto bottom;
     		}
     	}
     	printf("SrcAddr is one we're going to attempt to configure with:\n");
	DoPrintAddr();
	printf("Attempting to set address\n");
	DoConfig(IOB1->ios2_SrcAddr);
	if(IOB1->ios2_Req.io_Error)
	{
		if(IOB1->ios2_Req.io_Error == S2ERR_BAD_STATE &&
		   IOB1->ios2_WireError == S2WERR_IS_CONFIGURED)
			printf("Pardon Me. I'm Already Configured!\n");
		else
			printf("ERROR CODE:%ld  WERR:%ld\n",
				(long)IOB1->ios2_Req.io_Error,
				(long)IOB1->ios2_WireError);
	} else
	{
		printf("Configured.  Results (SrcAddr is the address actually config'd to):\n");
		DoPrintAddr();
		printf("S2_GETSTATIONADDRESS and results (Src=Current  Dst=Default):\n");
		DoGetAddr();  /* really should check for an error, device could be offline, for example  */
		DoPrintAddr();
	}
bottom: closedev();
}


void DoConfig(UBYTE *Address)
{
register UBYTE *p = IOB1->ios2_SrcAddr;
register i;

        IOB1->ios2_Req.io_Error   = 0;
        IOB1->ios2_Req.io_Command = S2_CONFIGINTERFACE;
        for(i = 0; i <  NumBytes; i++)
        	*(p++) = *(Address++);
        DoIO((struct IORequest *)IOB1);
}


void DoQuery(struct Sana2DeviceQuery *q)
{

        IOB1->ios2_Req.io_Error   = 0;
        IOB1->ios2_Req.io_Command = S2_DEVICEQUERY;
        IOB1->ios2_StatData = q;
        DoIO((struct IORequest *)IOB1);
        IOB1->ios2_StatData = NULL;
}


void DoGetAddr(void)
{

        IOB1->ios2_Req.io_Error   = 0;
        IOB1->ios2_Req.io_Command = S2_GETSTATIONADDRESS;
        DoIO((struct IORequest *)IOB1);
}

void DoPrintAddr(void)
{
register i;

	printf("SrcAddr: ");
        for (i = 0; i <  NumBytes; i++)
		printf("%02x ", (long)(IOB1->ios2_SrcAddr[i] & 0xFF)) ;
	printf("\nDstAddr: ");
        for (i = 0; i <  NumBytes; i++)
		printf("%02x ", (long)(IOB1->ios2_DstAddr[i] & 0xFF)) ;
	printf("\n");
}

