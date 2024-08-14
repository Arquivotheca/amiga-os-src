#include    "exec/types.h"
#include    "exec/ports.h"
#include    "exec/io.h"
#include    "libraries/dos.h"

#define     CHUNKSIZE	    (512*17) 
#define     TOTALCHUNKS     (101*4)
#define     CHUNKNAME	    "Track"
#define     MAXERRLINE	    19
#define     RETRYMAX	    3

extern LONG stdin;

struct Port *ioPort = 0;
struct IOStdReq *ioReq = 0;
ULONG errFile = 0;

UBYTE buffer[CHUNKSIZE];
int detected, reported, totalChunks;
int iteration;
int error;
int errLine;  
	      
EndGame(code)
int code;
{
    if (errFile != 0) Close(errFile);
    if (ioPort != 0) DeletePort(ioPort);
    if (ioReq != 0) {
	if (ioReq->io_Device != 0) CloseDevice(ioReq);
	FreeMem(ioReq, sizeof(*ioReq)); 	  
    }
    exit(code);
}
    

ReportError(chunk, retry, message, parm1, parm2, parm3, parm4)
int chunk;
char *message;
int parm1, parm2, parm3, parm4;
{					  
    if (retry == 0)
	if (++errLine > MAXERRLINE) errLine = 3;
    printf("\233%ld;HI %4ld, %s %4ld: ", errLine, iteration, CHUNKNAME, chunk);
    printf(message, parm1, parm2, parm3, parm4);
    if (errFile != 0) {
	if (retry == 0) {
	    fprintf(errFile, "\n I %4ld, %s %4ld: ",
		    iteration, CHUNKNAME, chunk);
	    fprintf(errFile, message, parm1, parm2, parm3, parm4);
	}
	else
	    fprintf(errFile, " %ld", retry+1);
    }
    printf("\233%ld;H\233K", ((errLine+1)>MAXERRLINE)?3:(errLine+1));
    if (retry == 0) {
	printf("\233%ld;H", MAXERRLINE+2);
	printf("Totals: %ld reported, %ld detected errors in %ld %ss", 
		reported, detected, totalChunks, CHUNKNAME);
    }
    error = TRUE; 
}

main(argc, argv)
int argc;
char *argv[];
{
    UBYTE *b, c;
    int retry;
    int i, j;

    if (argc > 1) {
	errFile = Open(argv[1], MODE_NEWFILE);
	if (errFile == 0) {
	    printf("error %ld opening \"%s\" for output.\n", IoErr(), argv[1]);
	    EndGame(20);
	}
    }
    ioPort = (struct Port *) CreatePort("test.jdisk.port", 0);
    if (ioPort == 0) {
	printf("error in ioPort creation.\n");
	EndGame(20);
    }
    ioReq = (struct IOStdReq *) CreateStdIO(ioPort);
    if (ioReq == 0) {
	printf("error in ioReq creation.\n");
	EndGame(20);
    }
    ioReq->io_Device = 0;
    error = OpenDevice("jdisk.device",0,ioReq,0);
    if (error != 0) {
	printf("error %ld in jdisk.device open.\n", error);
	EndGame(20);
    }

    detected = reported = totalChunks = 0;
    printf("\14jdisk.device test");
    if (errFile != 0) 
	printf(" -> \"%s\"", argv[1]);
    printf(": hit RETURN to terminate test...\n");
    printf("Testing %s xxxx,  Iteration xxxx", CHUNKNAME);
    errLine = MAXERRLINE;
    i = TOTALCHUNKS;
    iteration = 0;
    retry = RETRYMAX;
    while (!WaitForChar(stdin, 0)) { 
	if ((!error) || (++retry > RETRYMAX)) retry = 0;
	error = FALSE;
	if (retry == 0) {
	    totalChunks++;
	    if (++i >= TOTALCHUNKS) {
		printf("\2332;%ldH%4ld", sizeof(CHUNKNAME)+26, ++iteration);  
		i = 0;
	    }
	    printf("\2332;%ldH%4ld", sizeof(CHUNKNAME)+9, i);	   
	    b = buffer;
	    for (j = 0; j < CHUNKSIZE; j++) {
		c = 0xff & (i + j);
		if (iteration & 1) c = 0xff & (~c);
		*b++ = c;
	    }
	    ioReq->io_Command = CMD_WRITE;
	    ioReq->io_Data = buffer;
	    ioReq->io_Length = CHUNKSIZE;
	    ioReq->io_Offset = ULMult(i,CHUNKSIZE);
	    if (error = DoIO(ioReq)) {
		reported++;
		ReportError(i, retry, "Write Error reported 0x%lx", error);
	    }
	    else 
		if (ioReq->io_Actual != CHUNKSIZE) {
		    detected++;
		    ReportError(i, retry,
			    "Write io_Actual was %ld, wanted %ld",
			    ioReq->io_Actual, CHUNKSIZE);
		}
	}
	/* read the data */
	if (!error) {
	    b = buffer;
	    for (j = 0; j < CHUNKSIZE; j++)
		*b++ = 0;
	    ioReq->io_Command = CMD_READ;
	    ioReq->io_Data = buffer;
	    ioReq->io_Length = CHUNKSIZE;
	    ioReq->io_Offset = ULMult(i,CHUNKSIZE);
	    if (error = DoIO(ioReq)) {
		if (retry == 0) reported++;
		ReportError(i, retry, "Read Error reported 0x%lx", error);
	    }
	    else 
		if (ioReq->io_Actual != CHUNKSIZE) {
		    if (retry == 0) detected++;
		    ReportError(i, retry,
			    "Read io_Actual was %ld, wanted %ld",
			    ioReq->io_Actual, CHUNKSIZE);
		}
		else {
		    b = buffer;
		    for (j = 0; j < CHUNKSIZE; j++) {
			c = 0xff & (i + j);
			if (iteration & 1) c = 0xff & (~c);
			if (*b++ != c) {
			    if (retry == 0) detected++;
			    ReportError(i, retry,
		    "index %5ld got $%02lx, not $%02lx, read trial %ld",
				    ((ULONG) b) - ((ULONG) buffer) - 1,       
				    b[-1], c, retry+1);
			    error = TRUE; 
			}
		    }
		}
	}
    }
    printf("\233%ld;H", MAXERRLINE+2);
    printf("Totals: %ld reported, %ld detected errors in %ld %ss\n", 
	    reported, detected, totalChunks, CHUNKNAME);
    printf("End of jdisk.device test.\n");
    if (errFile) fprintf(errFile, "\n");
    EndGame(0);
}