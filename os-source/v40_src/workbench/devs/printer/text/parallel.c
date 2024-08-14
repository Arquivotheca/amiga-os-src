#include <exec/types.h>
#include <libraries/dos.h>
#include <devices/parallel.h>

int signal;
struct MsgPort parallelPort;

struct IOExtPar parallelIOB[16];
UWORD ioInUse;			/* mask of io elements in use */

UBYTE buffer[16][1024];
int length[16];

int file;

#define	NO_SIGNAL	1
#define	OPEN_FAIL	2
#define	FOPEN_FAIL	3
shutdown(code)
int code;
{
    switch (code) {
	case 0:
	    Close(file);
	case FOPEN_FAIL:
	    CloseDevice(&parallelIOB[0]);
	    RemPort(&parallelPort);
	case OPEN_FAIL:
	    FreeSignal(signal);
	case NO_SIGNAL:
    }
    exit(code);
}


main()
{
    int err, command;
    char fname[32], *fn;
    struct IOExtPar *ior;

    int iter, len, byte, req, i;

    /* allocate the signal */
    if ((signal=AllocSignal(-1))<0) {
	printf("can't get a signal\n");
	shutdown(NO_SIGNAL);
    }
    printf("signal = %ld\n",signal);

    /* open the device */
    if ((err = OpenDevice("parallel.device",0,&parallelIOB[0],0))!=0) {
	printf("open device failed:%ld\n", err);
	shutdown(OPEN_FAIL);
    }

    /* setup the msg port in the I/O request */
    parallelPort.mp_Node.ln_Type = NT_MSGPORT;
    parallelPort.mp_Flags = 0;
    parallelPort.mp_SigBit = signal;
    parallelPort.mp_SigTask = (struct Task *) FindTask(0);
    AddPort(&parallelPort);
    parallelIOB[0].IOPar.io_Message.mn_ReplyPort = &parallelPort;
    for (req = 1; req < 16; req++) {
	parallelIOB[req] = parallelIOB[0];
    }

    ioInUse = 0;
    command = 0;
    fn = fname;
    printf("enter error report file name: ");
    while ((*fn = getchar()) != '\n')
	if (*fn == '\b') fn--;
	else fn++;
    *fn = '\0';
    if ((file = Open(fname, MODE_NEWFILE)) == 0) {
	printf("open of file \"%s\" failed\n", fname);
	shutdown(FOPEN_FAIL);
    }

    while (command == 0) {
	printf("read or write [r,w]");
	byte = getchar();
	if (byte != '\n') while(getchar() != '\n');	/* flush line */
	if (byte == 'r') command = CMD_READ;
	if (byte == 'R') command = CMD_READ;
	if (byte == 'w') command = CMD_WRITE;
	if (byte == 'W') command = CMD_WRITE;
    }

    printf("Test initiated.  Hit any key to terminate it.\n");
    byte = 0;
    iter = 0;
    while (!WaitForChar(file, 0)) {
	for (len = 0; len < 1024; len++) {
	    while (!((~ioInUse) & (0xffff>>(16-(len&0xf)-1)))) {
		Wait(1<<signal);
		while (ior = (struct IOExtPar *) GetMsg(&parallelPort)) {
		    for (req = 0; req < 16; req++) {
			if (ior == &parallelIOB[req]) {
			    ioInUse &= ~(1<<req);
			    if (parallelIOB[req].IOPar.io_Error != 0) {
				if (command == CMD_READ) {
				    fprintf(file, "READ  iter %4ld, len %4ld: error %2ld\n", iter, len, parallelIOB[req].IOPar.io_Error);
				}
				else {
				    fprintf(file, "WRITE iter %4ld, len %4ld: error %2ld\n", iter, len, parallelIOB[req].IOPar.io_Error);
				}
			    }
			    if (parallelIOB[req].IOPar.io_Actual != length[req]) {
				if (command == CMD_READ) {
				    fprintf(file, "READ  iter %4ld, len %4ld: actual %4ld\n", iter, len, parallelIOB[req].IOPar.io_Actual);
				}
				else {
				    fprintf(file, "WRITE iter %4ld, len %4ld: actual %4ld\n", iter, len, parallelIOB[req].IOPar.io_Actual);
				}
			    }
			    if (command == CMD_READ) {
				i = 0;
				while (i < length[req]) {
				    if (buffer[req][i] != byte) {
					if (i+1 < length[req]) {
					    if (buffer[req][i] == 255) {
						if (buffer[req][i+1] == 0) {
						    fprintf(file, "READ  iter %4ld, len %4ld: lost %ld bytes starting with 0x%lx at %ld\n", iter, length[req], buffer[req][i]-byte, byte, i);
						    byte = buffer[req][i];
						}
						else {
						    fprintf(file, "READ  iter %4ld, len %4ld: expecting 0x%lx, got 0x%lx at %ld\n", iter, length[req], byte, buffer[req][i], i);
						}
					    }
					    else {
						if (buffer[req][i+1] == (buffer[req][i]+1)) {
						    fprintf(file, "READ  iter %4ld, len %4ld: lost %ld bytes starting with 0x%lx at %ld\n", iter, length[req], buffer[req][i]-byte, byte, i);
						    byte = buffer[req][i];
						}
						else {
						    fprintf(file, "READ  iter %4ld, len %4ld: expecting 0x%lx, got 0x%lx at %ld\n", iter, length[req], byte, buffer[req][i], i);
						}
					    }
					}
					else {
					    fprintf(file, "READ  iter %4ld, len %4ld: wrong last byte\n", iter, length[req]);
					}
				    }
				    i++;
				    if (++byte > 255) byte = 0;
				}
			    }
			}
		    }
		}
	    }
	    for (i = 0; i < (len&0xf)+1; i++) {
		if ((~ioInUse) & (1<<i)) {
		    req = i;
		}
	    }
	    ioInUse |= 1<<req;
	    parallelIOB[req].IOPar.io_Command = command;
	    parallelIOB[req].IOPar.io_Length = length[req] = len;
	    parallelIOB[req].IOPar.io_Data = buffer[req];
	    if (command == CMD_WRITE) {
		for (i = 0; i < len; i++) {
		    buffer[req][i] = byte;
		    if (++byte > 255) byte = 0;
		}
	    }
	    SendIO(&parallelIOB[req]);
	}
	iter++;
    }

    /* shut down */
    printf("shutting down\n");
    shutdown(0);
}
