
#include    "exec/types.h"
#include    "proto/exec.h"
#include    "devices/serial.h"
#include    "libraries/dos.h"


#define unless(x) if(!(x))


/* one port for all, all ports for one */
struct MsgPort	*Port;
struct IOExtSer *SerialIO0;
struct IOExtSer *SerialIO1;
struct IOExtSer *SerialIO2;
struct IOExtSer *SerialIO3;
struct IOExtSer *SerialIO4;
struct IOExtSer *SerialIO5;
struct IOExtSer *SerialIO6;
struct IOExtSer *SerialIO7;
struct IOExtSer *SerialIO8;
struct IOExtSer *SerialIO9;


#define BUFSIZE 80*5
char	buffer[BUFSIZE];



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


    for(temp=0; temp<BUFSIZE; temp++)
	{
	buffer[temp]=temp%10+'0';
	}

    Port=CreatePort(0L,0L);
    SerialIO0=(struct IOExtSer *)CreateExtIO( Port ,sizeof(struct IOExtSer));
    SerialIO1=(struct IOExtSer *)CreateExtIO( Port ,sizeof(struct IOExtSer));
    SerialIO2=(struct IOExtSer *)CreateExtIO( Port ,sizeof(struct IOExtSer));
    SerialIO3=(struct IOExtSer *)CreateExtIO( Port ,sizeof(struct IOExtSer));
    SerialIO4=(struct IOExtSer *)CreateExtIO( Port ,sizeof(struct IOExtSer));
    SerialIO5=(struct IOExtSer *)CreateExtIO( Port ,sizeof(struct IOExtSer));
    SerialIO6=(struct IOExtSer *)CreateExtIO( Port ,sizeof(struct IOExtSer));
    SerialIO7=(struct IOExtSer *)CreateExtIO( Port ,sizeof(struct IOExtSer));
    SerialIO8=(struct IOExtSer *)CreateExtIO( Port ,sizeof(struct IOExtSer));
    SerialIO9=(struct IOExtSer *)CreateExtIO( Port ,sizeof(struct IOExtSer));




    temp=OpenDevice("serial.device",unit,SerialIO0,0);
    printf("OpenDevice returns %lx\n",temp);


	    unless( temp )
		{
		SerialIO0->IOSer.io_Command= CMD_WRITE;
		SerialIO0->IOSer.io_Length = BUFSIZE;
		SerialIO0->IOSer.io_Data   = buffer;

		CopyMem( SerialIO0, SerialIO1, sizeof(struct IOExtSer) );
		CopyMem( SerialIO0, SerialIO2, sizeof(struct IOExtSer) );
		CopyMem( SerialIO0, SerialIO3, sizeof(struct IOExtSer) );
		CopyMem( SerialIO0, SerialIO4, sizeof(struct IOExtSer) );
		CopyMem( SerialIO0, SerialIO5, sizeof(struct IOExtSer) );
		CopyMem( SerialIO0, SerialIO6, sizeof(struct IOExtSer) );
		CopyMem( SerialIO0, SerialIO7, sizeof(struct IOExtSer) );
		CopyMem( SerialIO0, SerialIO8, sizeof(struct IOExtSer) );
		CopyMem( SerialIO0, SerialIO9, sizeof(struct IOExtSer) );

		SendIO(SerialIO0);
		SendIO(SerialIO1);
		SendIO(SerialIO2);
		SendIO(SerialIO3);
		SendIO(SerialIO4);
		SendIO(SerialIO5);
		SendIO(SerialIO6);
		SendIO(SerialIO7);
		SendIO(SerialIO8);
		SendIO(SerialIO9);

		WaitIO(SerialIO0);
		WaitIO(SerialIO1);
		WaitIO(SerialIO2);
		WaitIO(SerialIO3);
		WaitIO(SerialIO4);
		WaitIO(SerialIO5);
		WaitIO(SerialIO6);
		WaitIO(SerialIO7);
		WaitIO(SerialIO8);
		WaitIO(SerialIO9);

		CloseDevice(SerialIO0);
		}
    DeletePort(SerialIO0->IOSer.io_Message.mn_ReplyPort);
    DeleteExtIO(SerialIO0);
    DeleteExtIO(SerialIO1);
    DeleteExtIO(SerialIO2);
    DeleteExtIO(SerialIO3);
    DeleteExtIO(SerialIO4);
    DeleteExtIO(SerialIO5);
    DeleteExtIO(SerialIO6);
    DeleteExtIO(SerialIO7);
    DeleteExtIO(SerialIO8);
    DeleteExtIO(SerialIO9);
}
