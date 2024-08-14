
#include    "exec/types.h"
#include    "devices/serial.h"
#include    "libraries/dos.h"


#define unless(x) if(!(x))


struct IOExtSer *SerialIO;


#define BUFFERSIZE 1


int CXBRK()
{
    return(0); /* -1 means exit, 0 means stay */
}


main(int argc,char * argv[])
{
ULONG	temp;
ULONG	unit;
APTR	buffer;

    if(argc!=2)
	{
	printf("Give a unit number\n");
	exit(10);
	}

    unit=atol(argv[1]);


    buffer=(void *)AllocMem(BUFFERSIZE,0);


    if( SerialIO=(struct IOExtSer *)CreateExtIO( CreatePort(0L,0L) ,sizeof(struct IOExtSer)) )
	{
	    temp=OpenDevice("serial.device",unit,SerialIO,0);
	    printf("OpenDevice returns %lx\n",temp);
	    unless( temp )
		{
		while(!( SIGBREAKF_CTRL_C & SetSignal(SIGBREAKF_CTRL_C,0) ))
		    {
		    SerialIO->IOSer.io_Command= CMD_READ;
		    SerialIO->IOSer.io_Length = BUFFERSIZE;
		    SerialIO->IOSer.io_Data   = buffer;
		    temp=DoIO(SerialIO);
		    printf("%02x ",*((unsigned char *)SerialIO->IOSer.io_Data));
		    }

		/* Delay(2*50); */
		CloseDevice(SerialIO);
		}
	DeletePort(SerialIO->IOSer.io_Message.mn_ReplyPort);
	DeleteExtIO(SerialIO);
	}


    FreeMem(buffer,BUFFERSIZE);
}
