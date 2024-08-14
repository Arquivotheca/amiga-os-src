
#include    "exec/types.h"
#include    "exec/nodes.h"
#include    "devices/serial.h"
#include    "libraries/dos.h"


#define unless(x) if(!(x))


struct IOExtSer *SerialIO0;
struct IOExtSer *SerialIO1;


static char array[4]={0,0,0,0};



int CXBRK()
{
    return(-1); /* -1 means exit, 0 means stay */
}


main(int argc,char * argv[])
{
ULONG	temp;
ULONG	unit0;
ULONG	unit1;

    if(argc!=3)
	{
	printf("Give two unit numbers\n");
	exit(10);
	}

    unit0=atol(argv[1]);
    unit1=atol(argv[2]);

if( SerialIO0=(struct IOExtSer *)CreateExtIO( CreatePort(0L,0L) ,sizeof(struct IOExtSer)) )
    {

    if( SerialIO1=(struct IOExtSer *)CreateExtIO( CreatePort(0L,0L) ,sizeof(struct IOExtSer)) )
	{

	    temp =OpenDevice("serial.device",unit0,SerialIO0,0);
	    printf("OpenDevice %ld returns %lx\n",unit0,temp);
	    unless( temp )
		{

		temp|=OpenDevice("serial.device",unit1,SerialIO1,0);
		printf("OpenDevice %ld returns %lx\n",unit1,temp);

		/* Mark message as "done" (so CheckIO() is not fooled) */
		/* A false NULL will be send the first time */
		SerialIO0->IOSer.io_Message.mn_Node.ln_Type=NT_REPLYMSG;
		SerialIO1->IOSer.io_Message.mn_Node.ln_Type=NT_REPLYMSG;

		unless( temp )
		    {
		    while(!( SIGBREAKF_CTRL_C & SetSignal(SIGBREAKF_CTRL_C,0) ))
			{
			    SerialIO0->IOSer.io_Command= CMD_READ;
			    SerialIO0->IOSer.io_Length = 1;
			    SerialIO0->IOSer.io_Data   = &array[0];
			    temp=DoIO(SerialIO0);

			    SerialIO1->IOSer.io_Command= CMD_WRITE;
			    SerialIO1->IOSer.io_Length = 1;
			    SerialIO1->IOSer.io_Data   = &array[0];
			    temp=DoIO(SerialIO1);
			}
		    CloseDevice(SerialIO1);
		    }
		CloseDevice(SerialIO0);
		}
	DeletePort(SerialIO1->IOSer.io_Message.mn_ReplyPort);
	DeleteExtIO(SerialIO1);
	}
    DeletePort(SerialIO0->IOSer.io_Message.mn_ReplyPort);
    DeleteExtIO(SerialIO0);
    }
}
