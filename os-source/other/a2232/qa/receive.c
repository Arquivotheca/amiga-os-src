/*
    Commodore Amiga Test Suite
    Serial.device Performance Test Receiver
    Written by: John J. Szucs
                Systems Evaluation Section
                Product Assurance Department
                Commodore Technology, Inc.
    Copyright © 1990 by Commodore Technology, Inc.

Receive [UNIT <Unit Number>] [BAUD <Baud Rate>] [DATABITS <Data Bits>] [STOPBITS <Stop Bits>] [PARITY <Parity>] [BUFFER <Buffer>] [RAD_BOOGIE|NO_RAD_BOOGIE] [BYTES <Byte Count>] [SHARED|EXCLUSIVE] [SECS <Secs>] [MICROS <Micros>] [POLLS <Polls>] [XDISABLED] [7WIRE] [QUIET]

Description
===========
Open serial.device
Set baud rate, data bits, stop bits, and parity from command line
Receive for byte count and buffer size from command line with count
    of overrun errors
Close serial.device
Report parameters, bytes transferred, errors, and receive checksum
*/

/* System include section */
 #include "exec/types.h"
 #include "exec/memory.h"
 #include "exec/io.h"

 #include "devices/serial.h"
 #include "devices/timer.h"

 #include "dos/dos.h"

 #include "proto/exec.h"

/* ANSI C include section */
 #include "stdio.h"
 #include "ctype.h"
 #include "math.h"

/* Local macro section */
 #define StrToW(x) ((WORD) strtol(x,NULL,0))
 #define StrToL(x) (strtol(x,NULL,0))

/* Constant definition section */
 #define BYTES_PER_K 1024

 #define DEFAULT_UNIT_NUMBER 0 /* default: motherboard serial port */
 #define DEFAULT_BAUD_RATE 9600 /* default: 9600 baud */
 #define DEFAULT_DATA_BITS 8 /* default: 8 data bits */
 #define DEFAULT_STOP_BITS 1 /* default: 1 stop bit */
 #define DEFAULT_PARITY NULL /* default: no parity */
 #define DEFAULT_BYTE_COUNT 32*BYTES_PER_K /* default: 32K transfer */
 #define DEFAULT_BUFFER_SIZE 64 /* default: 64 bytes (minimum allowed by serial.device) */
 #define DEFAULT_RAD_BOOGIE NULL /* default: SERF_RAD_BOOGIE false */
 #define DEFAULT_QUICK_IO NULL /* default: IOF_QUICK false */
 #define DEFAULT_SHARED NULL /* default: exclusive access to serial.device unit */
 #define DEFAULT_TIMEOUT_SECS 1L /* default timeout: one-second receive timeout */
 #define DEFAULT_TIMEOUT_MICROS 0L
 #define DEFAULT_TIMEOUT_POLLS 60L /* default timeout: 60 polls (of 1 second each) */
 #define DEFAULT_XDISABLED NULL /* default: XOn/XOff enabled */
 #define DEFAULT_7WIRE NULL /* default: 7-wire protocol disabled */
 #define DEFAULT_QUIET FALSE /* default: full error reporting */

/* Global variable section */

struct MsgPort *SerMsgPort=NULL;
struct IOExtSer *IOExtSer=NULL;
BOOL SerialDeviceOpen=FALSE;

struct MsgPort *TimeMsgPort=NULL;
struct timerequest *TimeRequest=NULL;
BOOL TimerDeviceOpen=FALSE;

ULONG UnitNumber;
ULONG BaudRate;
UBYTE DataBits;
UBYTE StopBits;
ULONG Parity;
ULONG ByteCount;
UBYTE *Buffer=NULL;
ULONG BufferSize=0L;
ULONG RadBoogie;
ULONG QuickIO;
UBYTE Shared;
UBYTE XDisabled;
UBYTE Handshake7Wire;
UBYTE *Pattern=NULL;
ULONG PatternSize=0L;
ULONG TimeoutSecs;
ULONG TimeoutMicros;
ULONG TimeoutPolls;
BOOL Quiet;

/* Parser keyword setup */
UBYTE *KeywordStrings[] ={
    "UNIT",
    "BAUD",
    "DATABITS",
    "STOPBITS",
    "PARITY",
    "BYTES",
    "BUFFER",
    "RAD_BOOGIE",
    "NO_RAD_BOOGIE",
    "QUICK_IO",
    "NO_QUICK_IO",
    "STARTPATTERN",
    "ENDPATTERN",
    "SHARED",
    "EXCLUSIVE",
    "SECS",
    "MICROS",
    "POLLS",
    "XDISABLED",
    "7WIRE",
    "QUIET",
    NULL
};

enum Keywords {BadKW=-1, UnitNumberKW, BaudKW, DataBitsKW, StopBitsKW, ParityKW,
    BytesKW, BufferKW, RadBoogieKW, NoRadBoogieKW, QuickIOKW, NoQuickIOKW,
    StartPatternKW, EndPatternKW, SharedKW, ExclusiveKW, SecsKW, MicrosKW,
    PollsKW, Handshake7WireKW, XDisabledKW, QuietKW};

enum Keywords KeywordValues[] ={
    UnitNumberKW,
    BaudKW,
    DataBitsKW,
    StopBitsKW,
    ParityKW,
    BytesKW,
    BufferKW,
    RadBoogieKW,
    NoRadBoogieKW,
    QuickIOKW,
    NoQuickIOKW,
    StartPatternKW,
    EndPatternKW,
    SharedKW,
    ExclusiveKW,
    SecsKW,
    MicrosKW,
    PollsKW,
    XDisabledKW,
    Handshake7WireKW,
    QuietKW,
    BadKW
};

/* Parity flag section */
UBYTE *ParityStrings[] ={
    "NONE",
    "ODD",
    "EVEN",
    "N",
    "O",
    "E",
    NULL,
};

ULONG ParityFlags[] ={
    NULL,
    SERF_PARTY_ODD|SERF_PARTY_ON,
    SERF_PARTY_ON,
    NULL,
    SERF_PARTY_ODD|SERF_PARTY_ON,
    SERF_PARTY_ON,
    NULL,
};

/* General-purpose function section */

BOOL GetSingleFlag(ClientString,ClientFlag,Strings,Flags)
UBYTE *ClientString;
ULONG *ClientFlag;
UBYTE *Strings[];
ULONG Flags[];
{
    ULONG Index;

    strupr(ClientString);

    for (Index=0;Strings[Index]!=NULL;Index++)
        if (strcmp(ClientString,Strings[Index])==0) {
            *ClientFlag=Flags[Index];
            return(TRUE);
        }

    return(FALSE);
}

enum Keywords GetKeyword(String)
UBYTE *String;
{
    int Lookup;
    UBYTE *Temp;

    Temp=AllocMem(strlen(String)+1,MEMF_PUBLIC|MEMF_CLEAR);
    strcpy(Temp,String);

    strupr(Temp);

    for (Lookup=0;KeywordStrings[Lookup]!=NULL;Lookup++)
        if (strcmp(KeywordStrings[Lookup],Temp)==0) {
            FreeMem(Temp,strlen(Temp)+1);
            return(KeywordValues[Lookup]);
        }

    FreeMem(Temp,strlen(Temp)+1);

    return(BadKW);
}

int isnumber(string)
char *string;
{
    int Loop;

    if (strncmp(string,"0x",2)==0) {
        for (Loop=2;Loop<strlen(string);Loop++)
            if (!isxdigit(string[Loop]))
                return(0);
    } else {
        for (Loop=0;Loop<strlen(string);Loop++)
            if (!(isdigit(string[Loop])) && string[Loop]!='-')
                return(0);
    }

    return(-1);
}

/* Main program */

main(argc,argv)
int argc;
char *argv[];
{
    int Argument;
    enum Keywords Keyword;

    BYTE Error;

    ULONG BytesTransferred=0L, Errors=0L, Checksum=0L;
    BOOL TimeoutFlag;
    ULONG Loop;
    LONG StartTime[3], StopTime[3];
    ULONG ElapsedTime;
    BOOL StartTimeSet=FALSE;

    /* Sign-on message */
    printf("Commodore Amiga Test Suite\n");
    printf("serial.device Performance Test Receiver Module\n");
    printf("Copyright © 1990 by Commodore Technology, Inc.\n");

    /* Default set-up */
    UnitNumber=DEFAULT_UNIT_NUMBER;
    BaudRate=DEFAULT_BAUD_RATE;
    DataBits=DEFAULT_DATA_BITS;
    StopBits=DEFAULT_STOP_BITS;
    Parity=DEFAULT_PARITY;
    ByteCount=DEFAULT_BYTE_COUNT;
    BufferSize=DEFAULT_BUFFER_SIZE;
    RadBoogie=DEFAULT_RAD_BOOGIE;
    QuickIO=DEFAULT_QUICK_IO;
    XDisabled=DEFAULT_XDISABLED;
    Handshake7Wire=DEFAULT_7WIRE;
    TimeoutSecs=DEFAULT_TIMEOUT_SECS;
    TimeoutMicros=DEFAULT_TIMEOUT_MICROS;
    TimeoutPolls=DEFAULT_TIMEOUT_POLLS;
    Quiet=DEFAULT_QUIET;

    /* Command line parsing */
    for (Argument=1;Argument<argc;Argument++) {
        Keyword=GetKeyword(argv[Argument]);
        switch (Keyword) {
            case UnitNumberKW:
                if (Argument>argc-2)
                    UserError("Required argument missing\n");
                Argument++;
                if (!isnumber(argv[Argument]))
                    UserError("Invalid numeric argument\n");
                UnitNumber=StrToL(argv[Argument]);
                break;
            case BaudKW:
                if (Argument>argc-2)
                    UserError("Required argument missing\n");
                Argument++;
                if (!isnumber(argv[Argument]))
                    UserError("Invalid numeric argument\n");
                BaudRate=StrToL(argv[Argument]);
                break;
            case DataBitsKW:
                if (Argument>argc-2)
                    UserError("Required argument missing\n");
                Argument++;
                if (!isnumber(argv[Argument]))
                    UserError("Invalid numeric argument\n");
                DataBits=(UBYTE) StrToW(argv[Argument]);
                break;
            case StopBitsKW:
                if (Argument>argc-2)
                    UserError("Required argument missing\n");
                Argument++;
                if (!isnumber(argv[Argument]))
                    UserError("Invalid numeric argument\n");
                StopBits=(UBYTE) StrToW(argv[Argument]);
                break;
            case ParityKW:
                if (Argument>argc-2)
                    UserError("Required argument missing\n");
                Argument++;
                if (!GetSingleFlag(argv[Argument],&Parity,ParityStrings,ParityFlags))
                    UserError("Invalid parity mode\n");
                break;
            case BytesKW:
                if (Argument>argc-2)
                    UserError("Required argument missing\n");
                Argument++;
                if (!isnumber(argv[Argument]))
                    UserError("Invalid numeric argument\n");
                ByteCount=StrToL(argv[Argument]);
                break;
            case BufferKW:
                if (Argument>argc-2)
                    UserError("Required argument missing\n");
                Argument++;
                if (!isnumber(argv[Argument]))
                    UserError("Invalid numeric argument\n");
                BufferSize=StrToL(argv[Argument]);
                if (BufferSize%64)
                    UserError("Buffer size must be multiple of 64\n");
                break;
            case RadBoogieKW:
                RadBoogie=SERF_RAD_BOOGIE;
                break;
            case NoRadBoogieKW:
                RadBoogie=NULL;
                break;
            case QuickIOKW:
                QuickIO=IOF_QUICK;
                break;
            case NoQuickIOKW:
                QuickIO=NULL;
                break;
            case SharedKW:
                Shared=SERF_SHARED;
                break;
            case ExclusiveKW:
                Shared=NULL;
                break;
            case SecsKW:
                if (Argument>argc-2)
                    UserError("Required argument missing\n");
                Argument++;
                if (!isnumber(argv[Argument]))
                    UserError("Invalid numeric argument\n");
                TimeoutSecs=StrToL(argv[Argument]);
                break;
            case MicrosKW:
                if (Argument>argc-2)
                    UserError("Required argument missing\n");
                Argument++;
                if (!isnumber(argv[Argument]))
                    UserError("Invalid numeric argument\n");
                TimeoutMicros=StrToL(argv[Argument]);
                break;
            case PollsKW:
                if (Argument>argc-2)
                    UserError("Required argument missing\n");
                Argument++;
                if (!isnumber(argv[Argument]))
                    UserError("Invalid numeric argument\n");
                TimeoutPolls=StrToL(argv[Argument]);
                break;
            case XDisabledKW:
                XDisabled=SERF_XDISABLED;
                break;
            case Handshake7WireKW:
                Handshake7Wire=SERF_7WIRE;
                break;
            case QuietKW:
                Quiet=TRUE;
                break;
            default:
                printf("Keyword %s not recognized\n",argv[Argument]);
                Syntax();
                ShutDown();
                break;
        }
    }

    if (BufferSize<ByteCount) {
        BufferSize=ByteCount;
        if (BufferSize%64)
            BufferSize+=64-(BufferSize%64);
        printf("** Increasing buffer size to %lu ** \n",BufferSize);
    }

    /* Display parameters */
    printf("Unit number=%lu\n",UnitNumber);
    printf("Baud Rate=%lu\n",BaudRate);
    printf("Data Bits=%u\n",(UWORD) DataBits);
    printf("Stop Bits=%u\n",(UWORD) StopBits);
    printf("Parity=$%08lx\n",Parity);
    printf("Byte Count=%lu\n",ByteCount);
    printf("BufferSize=%lu\n",BufferSize);
    printf("RadBoogie=$%08lx\n",RadBoogie);
    printf("QuickIO=$%08lx\n",QuickIO);
    printf("Shared=$%02x\n",(UWORD) Shared);
    printf("XDisabled=$%02x\n",(UWORD) XDisabled);
    printf("Handshake 7-wire=$%02x\n",(UWORD) Handshake7Wire);
    printf("Timeout Secs=%lu\n",TimeoutSecs);
    printf("Timeout Micros=%lu\n",TimeoutMicros);
    printf("Timeout Polls=%lu\n",TimeoutPolls);
    printf("Quiet=$%02x\n",Quiet);

    /* Allocate buffer */
    Buffer=AllocMem(BufferSize,MEMF_PUBLIC|MEMF_CLEAR);
    if (Buffer==NULL)
        SystemError("Error in buffer allocation\n");

    /* Create serial.device reply port */
    SerMsgPort=CreatePort(NULL,0L);
    if (SerMsgPort==NULL)
        SystemError("Error in creation of serial.device reply port\n");

    /* Create IOExtSer I/O Request */
    IOExtSer=(struct IOExtSer *) CreateExtIO(SerMsgPort,sizeof(struct IOExtSer));
    if (IOExtSer==NULL)
        SystemError("Error in creation of serial.device IOExtSer\n");

    /* Initialize shared bit in IOExtSer */
    IOExtSer->io_SerFlags=Shared;

    /* Open serial.device */
    Error=OpenDevice("serial.device",UnitNumber,(struct IORequest *) IOExtSer,NULL);
    if (Error) {
        printf("Error %d in open of serial.device unit %ld\n",Error,UnitNumber);
        ShutDown();
    }
    SerialDeviceOpen=TRUE;

    /* Set-up serial.device parameters */
    IOExtSer->io_RBufLen=BufferSize;
    IOExtSer->io_Baud=BaudRate;
    IOExtSer->io_ReadLen=DataBits;
    IOExtSer->io_WriteLen=DataBits;
    IOExtSer->io_StopBits=StopBits;
    IOExtSer->io_SerFlags=Parity|RadBoogie|Shared|XDisabled|Handshake7Wire;
    IOExtSer->IOSer.io_Command=SDCMD_SETPARAMS;

    Error=DoIO((struct IORequest *) IOExtSer);
    if (Error) {
        printf("Error %d in serial.device parameter set-up\n",Error);
        ShutDown();
    }

    /* Create timer.device reply port */
    TimeMsgPort=CreatePort(NULL,0L);
    if (TimeMsgPort==NULL)
        SystemError("Error in creation of timer.device reply port\n");

    /* Create timerequest I/O request */
    TimeRequest=(struct timerequest *) CreateExtIO(TimeMsgPort,sizeof(struct timerequest));
    if (TimeRequest==NULL)
        SystemError("Error in creation of timer.device timerequest\n");

    /* Open timer.device */
    Error=OpenDevice("timer.device",UNIT_MICROHZ,(struct IORequest *) TimeRequest,NULL);
    if (Error) {
        printf("Error %d in open of timer.device unit UNIT_MICROHZ\n",Error);
        ShutDown();
    }

    /* Main loop */

    Errors=0L;
    BytesTransferred=0L;
    TimeoutFlag=FALSE;

    while (BytesTransferred<ByteCount && TimeoutFlag==FALSE) {
        Loop=0L;
        IOExtSer->IOSer.io_Actual=0L;
        while (Loop<TimeoutPolls && IOExtSer->IOSer.io_Actual==0L) {
            IOExtSer->IOSer.io_Command=SDCMD_QUERY;
            /* Quote for today:
                    "Query will always succeed" -- AutoDocs:Serial.doc */
            DoIO((struct IORequest *) IOExtSer);
            if (IOExtSer->IOSer.io_Actual==0L) {
                /* Delay for timeout interval */
                TimeRequest->tr_node.io_Command=TR_ADDREQUEST;
                TimeRequest->tr_time.tv_secs=TimeoutSecs;
                TimeRequest->tr_time.tv_micro=TimeoutMicros;
                Error=DoIO((struct IORequest *) TimeRequest);
                if (Error) {
                    printf("Error in DoIO of timer.device TR_ADDREQUEST\n");
                    printf("    with tv_secs=%lu and tv_micros=%lu (code=%d)\n",TimeoutSecs,TimeoutMicros,Error);
                    CleanUp(RETURN_FAIL);
                }
            }
            Loop++;
        }
        if (IOExtSer->IOSer.io_Actual==0)
            TimeoutFlag=TRUE;
        if (IOExtSer->IOSer.io_Actual) {
            if (StartTimeSet==FALSE) {
                DateStamp(StartTime);
                StartTimeSet=TRUE;
            }
            if (IOExtSer->IOSer.io_Actual>BufferSize)
                IOExtSer->IOSer.io_Length=BufferSize;
            else
                IOExtSer->IOSer.io_Length=IOExtSer->IOSer.io_Actual;
            IOExtSer->IOSer.io_Command=CMD_READ;
            IOExtSer->IOSer.io_Flags=QuickIO;
            IOExtSer->IOSer.io_Data=Buffer;
            Error=DoIO((struct IORequest *) IOExtSer);
            for (Loop=0;Loop<IOExtSer->IOSer.io_Actual;Loop++)
                Checksum+=Buffer[Loop];
            BytesTransferred+=IOExtSer->IOSer.io_Actual;
            if (Error) {
                Errors++;
                if (!Quiet)
                    printf("Error %d on read from serial.device on byte span %lu-%lu\n",Error,BytesTransferred-IOExtSer->IOSer.io_Actual,BytesTransferred-1);
            }
        }
    }

    DateStamp(StopTime);

    if (TimeoutFlag)
        printf("*** Receive timeout ***\n");

    printf("Bytes transferred=%lu\n",BytesTransferred);
    printf("Errors=%lu\n",Errors);
    printf("Receive checksum=$%08lx\n",Checksum);

    if (StartTimeSet)
        ElapsedTime=((StopTime[1]*60)+StopTime[2]/50)-((StartTime[1]*60)+StartTime[2]/50);
    else
        ElapsedTime=TimeoutSecs;

    printf("Elapsed time=%ld seconds\n",ElapsedTime);
    if (ElapsedTime==0L)
        printf("Throughput>=%ld bytes/second\n",BytesTransferred);
    else
        printf("Throughput=%ld bytes/second\n",BytesTransferred/ElapsedTime);

    CleanUp(RETURN_OK);
}

UserError(String)
UBYTE *String;
{
    puts(String);
    Syntax();
    ShutDown();
}

SystemError(String)
UBYTE *String;
{
    puts(String);
    ShutDown();
}

Syntax()
{
    printf("Syntax:\n");
    printf("Receive [UNIT <Unit Number>] [BAUD <Baud Rate>] [DATABITS <Data Bits>]\n");
    printf("    [STOPBITS <Stop Bits>] [PARITY <Parity>] [BUFFER <Buffer>]\n");
    printf("    [SERF_RAD_BOOGIE|NO_SERF_RAD_BOOGIE] [BYTES <Byte Count>]\n");
    printf("    [SHARED|EXCLUSIVE] [XDISABLED] [7WIRE]\n");
    printf("    [SECS <Secs>] [MICROS <Micros>] [POLLS <Polls>]\n");
}

ShutDown()
{
    CleanUp(RETURN_FAIL);
}

CleanUp(ReturnCode)
int ReturnCode;
{
    if (SerialDeviceOpen)
        CloseDevice((struct IORequest *) IOExtSer);

    if (SerMsgPort)
        DeletePort(SerMsgPort);

    if (IOExtSer)
        DeleteExtIO((struct IORequest *) IOExtSer);

    if (TimerDeviceOpen)
        CloseDevice((struct IORequest *) TimeRequest);

    if (TimeMsgPort)
        DeletePort(TimeMsgPort);

    if (TimeRequest)
        DeleteExtIO((struct IORequest *) TimeRequest);

    if (Buffer)
        FreeMem(Buffer,BufferSize);

    exit(ReturnCode);
}
