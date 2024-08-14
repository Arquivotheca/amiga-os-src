
#include    "exec/types.h"
#include    "devices/serial.h"


#define unless(x) if(!(x))


struct IOExtSer *SerialIO;


main(int argc,char * argv[])
{
ULONG	temp;
ULONG	temp2=0;
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
		SerialIO->IOSer.io_Command=SDCMD_QUERY;
		temp=DoIO(SerialIO);
		printf("SDCMD_QUERY %ld, Actual-%x\n",temp,SerialIO->IOSer.io_Actual);

		SerialIO->IOSer.io_Command=SDCMD_SETPARAMS;
		SerialIO->io_Baud=9600;
		temp=DoIO(SerialIO);
		printf("SDCMD_SETPARAMS %ld\n",temp);

		SerialIO->IOSer.io_Command= CMD_WRITE;
		SerialIO->IOSer.io_Length = strlen("fish are frogs\n");
		SerialIO->IOSer.io_Data   = (APTR) "fish are frogs\n";
		temp=DoIO(SerialIO);
		printf("SDCMD_WRITE %ld-io_Actual %lx\n",temp,SerialIO->IOSer.io_Actual);

		SerialIO->IOSer.io_Command= CMD_WRITE;
		SerialIO->IOSer.io_Length = -1;
		SerialIO->IOSer.io_Data   = (APTR)"...the mice will play\n";
		temp=DoIO(SerialIO);
		printf("SDCMD_WRITE %ld-io_Actual %lx\n",temp,SerialIO->IOSer.io_Actual);

		CloseDevice(SerialIO);
		}
	DeletePort(SerialIO->IOSer.io_Message.mn_ReplyPort);
	DeleteExtIO(SerialIO);
	}
}
