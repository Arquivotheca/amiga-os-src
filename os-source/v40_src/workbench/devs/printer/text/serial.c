#include <exec/types.h>
#include <exec/exec.h>
#include <devices/serial.h>

struct MsgPort serialPort;
struct IOExtSer serialIOB0;
struct IOExtSer serialIOB1;
int signal;

main()
{
    ULONG err;

    struct IOExtSer *ior;
    int i, j, flag;
    char data0[80], data1[80];

    if ((signal=AllocSignal(-1))<0) {
	printf("can't get a signal\n");
	Panic(0);
    }
    printf("signal=%ld\n",signal);
    /* open the device */
    if ((err=OpenDevice("serial.device",0,&serialIOB0,0))!=0) {
	printf("open device failed\n");
	Panic(1);
    }
    printf("opendevice err=%ld\n",err);
    Key();
    /* setup the msg port in the I/O request */
    serialPort.mp_Node.ln_Type = NT_MSGPORT;
    serialPort.mp_Flags = 0;
    serialPort.mp_SigBit = signal;
    serialPort.mp_SigTask = (struct Task *) FindTask((char *) NULL);
    AddPort(&serialPort);
    serialIOB0.IOSer.io_Message.mn_ReplyPort = &serialPort;
    serialIOB1 = serialIOB0;

    for (i = 1; i <= 26; i++) {
	for (j = 0; j < i*3; j++) {
	    data0[j] = 'A'+i-1;
	    data1[j] = 'a'+i-1;
	}
	data0[i*3] = 0;
	data1[i*3] = 0;
	serialIOB0.IOSer.io_Command = CMD_WRITE;
	serialIOB0.IOSer.io_Length = i*3;
	serialIOB0.IOSer.io_Data = data0;
	serialIOB1.IOSer.io_Command = CMD_WRITE;
	serialIOB1.IOSer.io_Length = i*3;
	serialIOB1.IOSer.io_Data = data1;
	printf("calling SendIO for 0 & 1\n");
	printf(data0);
	printf("\n");
	printf(data1);
	printf("\n");
	SendIO(&serialIOB0);
	SendIO(&serialIOB1);
	printf("wait for ...");
	flag = 3;
	while (flag) {
	    Wait(1<<signal);
	    while (ior = (struct IOExtSer *) GetMsg(&serialPort)) {
		if (ior == &serialIOB0) {
		    printf(" 0 done");
		    flag &= 2;
		}
		if (ior == &serialIOB1) {
		    printf(" 1 done");
		    flag &= 1;
		}
	    }
	    if (flag) printf(" ... ");
	}
	printf("\n");
    }

    /* check for an error */
    if ((err=serialIOB0.IOSer.io_Error)!=0) {
	printf("IOB0 error # %ld\n",err);
    }
    if ((err=serialIOB1.IOSer.io_Error)!=0) {
	printf("IOB1 error # %ld\n",err);
    }
    /* shut down */
    printf("shutting down\n");
    Panic(3);
}
Panic(code)
int code;
{
    if (code>=2) {
	CloseDevice(&serialIOB0);
	CloseDevice(&serialIOB1);
	RemPort(&serialPort);
    }
    if (code>=1) FreeSignal(signal);
    exit();
}
Key()
{
    int c;
    printf("hit return...");
    c = getchar();
}
