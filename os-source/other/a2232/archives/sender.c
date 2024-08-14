
#include    "exec/types.h"
#include    "devices/serial.h"
#include    "libraries/dos.h"


#define unless(x) if(!(x))


struct IOExtSer *SerialIO;


static char array[4]={0x01,0x02,0x00,0x04};



int CXBRK()
{
    return(-1); /* -1 means exit, 0 means stay */
}


main(int argc,char * argv[])
{
ULONG	temp;
ULONG	unit;

    if(argc!=2)
	{
	printf("Give a unit number\n");
	exit(10);
	}

    unit=atol(argv[1]);

    if( SerialIO=(struct IOExtSer *)CreateExtIO( CreatePort(0L,0L) ,sizeof(struct IOExtSer)) )
	{
	    temp=OpenDevice("serial.device",unit,SerialIO,0);
	    printf("OpenDevice returns %lx\n",temp);
	    unless( temp )
		{
		while(!( SIGBREAKF_CTRL_C & SetSignal(SIGBREAKF_CTRL_C,0) ))
		    {
		  /* SerialIO->IOSer.io_Command= CMD_WRITE;
		    SerialIO->IOSer.io_Length = 20;
		    SerialIO->IOSer.io_Data   = (APTR)"-1234567890123456789";
		    temp=DoIO(SerialIO);
		   */

		    SerialIO->IOSer.io_Command= CMD_WRITE;
		    SerialIO->IOSer.io_Length = 4;
		    SerialIO->IOSer.io_Data   = &array[0];
		    temp=DoIO(SerialIO);
		    }

		/* Delay(2*50); */
		CloseDevice(SerialIO);
		}
	DeletePort(SerialIO->IOSer.io_Message.mn_ReplyPort);
	DeleteExtIO(SerialIO);
	}
}
