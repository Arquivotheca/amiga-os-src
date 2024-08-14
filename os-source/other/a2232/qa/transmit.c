/*
    Commodore Amiga Test Suite
    Serial.device Test Transmitter
    Written by: John J. Szucs
                Systems Evaluation Group
                Product Assurance Department
                Commodore Technology, Inc.
    Copyright © 1990 by Commodore Technology, Inc.

Transmit [UNIT <Unit Number>] [BAUD <Baud Rate>] [DATABITS <Data Bits>] [STOPBITS <Stop Bits>] [PARITY <Parity>] [BUFFER <Buffer>] [RAD_BOOGIE|NO_RAD_BOOGIE] [BYTES <Byte Count>] [STARTPATTERN <Byte>[ <Byte>...] ENDPATTERN] [SHARED|EXCLUSIVE] [MICROS <Micros>] [SECS <Secs>] [NOTIMEOUT] [XDISABLED] [7WIRE] [PATTERNSIZE <Bytes>] [QUIET] [PRE_DELAY <Seconds>] [POST_DELAY <Seconds>]

Description
===========
Open serial.device
Set baud rate, data bits, stop bits, and parity from command line
Transmit pattern for <Byte Count> bytes
Close serial.device
Report parameters, bytes transferred, errors, and transmit checksum
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
 #define BITS_PER_BYTE 8
 #define MAX_UBYTE ((1<<BITS_PER_BYTE)-1)

 #define MILLION 1000000

 #define DEFAULT_UNIT_NUMBER 0 /* default: motherboard serial port */
 #define DEFAULT_BAUD_RATE 9600 /* default: 9600 baud */
 #define DEFAULT_DATA_BITS 8 /* default: 8 data bits */
 #define DEFAULT_STOP_BITS 1 /* default: 1 stop bit */
 #define DEFAULT_PARITY NULL /* default: no parity */
 #define DEFAULT_BYTE_COUNT 32*BYTES_PER_K /* default: 32K transfer */
 #define DEFAULT_BUFFER_SIZE 64 /* default: 64 bytes (minimum allowed by serial.device) */
 #define DEFAULT_RAD_BOOGIE NULL /* default: SERF_RAD_BOOGIE false */
 #define DEFAULT_QUICK_IO NULL /* default: IOF_QUICK false */
 #define DEFAULT_SHARED NULL /* default: exclusive access to unit */
 #define DEFAULT_TIMEOUT_CHECK TRUE /* default: timeout checking on */
 #define DEFAULT_XDISABLED NULL /* default: XOn/XOff enabled */
 #define DEFAULT_7WIRE NULL /* default: 7-wire protocol disabled */
 #define DEFAULT_QUIET FALSE /* default: full error reporting */
 #define DEFAULT_PRE_DELAY 0 /* default: no delay before start of transmission */
 #define DEFAULT_POST_DELAY 0 /* default: no delay before closing serial.device at end of transmission */

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
ULONG TimeoutMicros=0L;
ULONG TimeoutSecs=0L;
BOOL TimeoutCheck;
BOOL Quiet;
ULONG PreDelay;
ULONG PostDelay;

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
    "MICROS",
    "SECS",
    "NOTIMEOUT",
    "XDISABLED",
    "7WIRE",
    "PATTERNSIZE",
    "QUIET",
    "PRE_DELAY",
    "POST_DELAY",
    NULL
};

enum Keywords {BadKW=-1, UnitNumberKW, BaudKW, DataBitsKW, StopBitsKW, ParityKW,
    BytesKW, BufferKW, RadBoogieKW, NoRadBoogieKW, QuickIOKW, NoQuickIOKW,
    StartPatternKW, EndPatternKW, SharedKW, ExclusiveKW, MicrosKW, SecsKW,
    NoTimeoutKW, XDisabledKW, Handshake7WireKW, PatternSizeKW, QuietKW,
    PreDelayKW, PostDelayKW};

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
    MicrosKW,
    SecsKW,
    NoTimeoutKW,
    XDisabledKW,
    Handshake7WireKW,
    PatternSizeKW,
    QuietKW,
    PreDelayKW,
    PostDelayKW,
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
    int Argument, PatternLoop;
    enum Keywords Keyword;
    UBYTE PatternByte;

    BYTE Error;

    ULONG BytesTransferred=0L, Errors=0L, PatternChecksum=0L, Checksum=0L;
    BOOL TimeoutFlag;
    ULONG WaitSignal, WaitMask, WriteLength;
    LONG StartTime[3], StopTime[3];
    ULONG ElapsedTime;

    BOOL SecsSet=FALSE, MicrosSet=FALSE, PatternSet=FALSE;

    /* Sign-on message */
    printf("Commodore Amiga Test Suite\n");
    printf("serial.device Performance Test Transmitter Module\n");
    printf("Copyright © 1989 by Commodore Technology, Inc.\n");

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
    TimeoutCheck=DEFAULT_TIMEOUT_CHECK;
    Quiet=DEFAULT_QUIET;
    PreDelay=DEFAULT_PRE_DELAY;
    PostDelay=DEFAULT_POST_DELAY;

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
            case StartPatternKW:
                if (Argument>argc-2)
                    UserError("STARTPATTERN encountered without matching ENDPATTERN\n");
                PatternSize=0L;
                Error=TRUE;
                for (PatternLoop=Argument+1;PatternLoop<argc;PatternLoop++) {
                    Keyword=GetKeyword(argv[PatternLoop]);
                    if (Keyword==EndPatternKW) {
                        Error=FALSE;
                        break;
                    } else
                        PatternSize++;
                }
                if (Error)
                    UserError("STARTPATTERN encountered without matching ENDPATTERN\n");
                if (PatternSize==0L)
                    SystemError("Empty pattern\n");
                Pattern=AllocMem(PatternSize,MEMF_PUBLIC);
                if (Pattern==NULL)
                    UserError("Unable to allocate pattern buffer\n");
                for (PatternLoop=0;PatternLoop<PatternSize;PatternLoop++) {
                    if (!isnumber(argv[Argument+1+PatternLoop]))
                        UserError("Invalid numeric argument\n");
                    PatternByte=StrToW(argv[Argument+1+PatternLoop]);
                    Pattern[PatternLoop]=PatternByte;
                    PatternChecksum+=PatternByte;
                }
                Argument+=PatternSize+1;
                PatternSet=TRUE;
                break;
            case EndPatternKW:
                UserError("ENDPATTERN encountered without matching STARTPATTERN\n");
                break;
            case SharedKW:
                Shared=SERF_SHARED;
                break;
            case ExclusiveKW:
                Shared=NULL;
                break;
            case MicrosKW:
                if (Argument>argc-2)
                    UserError("Required argument missing\n");
                Argument++;
                if (!isnumber(argv[Argument]))
                    UserError("Invalid numeric argument\n");
                TimeoutMicros=StrToL(argv[Argument]);
                MicrosSet=TRUE;
                if (SecsSet==FALSE)
                    TimeoutSecs=0L;
                break;
            case SecsKW:
                if (Argument>argc-2)
                    UserError("Required argument missing\n");
                Argument++;
                if (!isnumber(argv[Argument]))
                    UserError("Invalid numeric argument\n");
                TimeoutSecs=StrToL(argv[Argument]);
                SecsSet=TRUE;
                if (MicrosSet==FALSE)
                    TimeoutMicros=0L;
                break;
            case NoTimeoutKW:
                TimeoutCheck=FALSE;
                break;
            case XDisabledKW:
                XDisabled=SERF_XDISABLED;
                break;
            case Handshake7WireKW:
                Handshake7Wire=SERF_7WIRE;
                break;
            case PatternSizeKW:
                if (Argument>argc-2)
                    UserError("Required argument missing\n");
                Argument++;
                if (!isnumber(argv[Argument]))
                    UserError("Invalid numeric argument\n");
                PatternSize=StrToL(argv[Argument]);
                break;
            case QuietKW:
                Quiet=TRUE;
                break;
            case PreDelayKW:
                if (Argument>argc-2)
                    UserError("Required argument missing\n");
                Argument++;
                if (!isnumber(argv[Argument]))
                    UserError("Invalid numeric argument\n");
                PreDelay=StrToL(argv[Argument]);
                break;
            case PostDelayKW:
                if (Argument>argc-2)
                    UserError("Required argument missing\n");
                Argument++;
                if (!isnumber(argv[Argument]))
                    UserError("Invalid numeric argument\n");
                PostDelay=StrToL(argv[Argument]);
                break;
            default:
                printf("Keyword %s not recognized\n",argv[Argument]);
                Syntax();
                ShutDown();
                break;
        }
    }

    if (Pattern==NULL) {
        if (PatternSize==0)
            PatternSize=ByteCount;
        Pattern=AllocMem(PatternSize,MEMF_PUBLIC);
        if (Pattern==NULL)
            SystemError("Error in pattern buffer allocation\n");
        for (PatternLoop=0;PatternLoop<PatternSize;PatternLoop++) {
            Pattern[PatternLoop]=PatternLoop%(MAX_UBYTE+1);
            PatternChecksum+=Pattern[PatternLoop];
        }
    }

    if (MicrosSet==FALSE && SecsSet==FALSE) {
        /* Timeout secs = PatternSize/BaudRate/(DataBits+StopBits+StartBit)*5
            (minimum of 5 seconds) */
        TimeoutSecs=max(PatternSize/(BaudRate/(DataBits+StopBits+1))*5,5L);
        TimeoutMicros=0L;
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
    printf("Handshake 7-Write=$%02x\n",(UWORD) Handshake7Wire);
    printf("Pattern Size=%lu\n",PatternSize);
    if (PatternSet) {
        for (PatternLoop=0;PatternLoop<PatternSize;PatternLoop++)
            printf("Pattern[%lu]=$%02x\n",PatternLoop,(WORD) Pattern[PatternLoop]);
    }
    printf("Timeout Micros=%lu\n",TimeoutMicros);
    printf("Timeout Secs=%lu\n",TimeoutSecs);
    printf("Timeout Check=%d\n",TimeoutCheck);
    printf("Quiet=$%02x\n",Quiet);

    /* Create Serial MsgPort */
    SerMsgPort=CreatePort(NULL,0L);
    if (SerMsgPort==NULL)
        SystemError("Error in creation of serial.device I/O request reply port\n");

    /* Create IOExtSer I/O Request */
    IOExtSer=(struct IOExtSer *) CreateExtIO(SerMsgPort,sizeof(struct IOExtSer));
    if (IOExtSer==NULL)
        SystemError("Error in creation of serial.device IOExtSer\n");

    /* Initialize shared bit in IOExtSer */
    IOExtSer->io_SerFlags=Shared;

    /* Open serial.device */
    Error=OpenDevice("serial.device",UnitNumber,(struct IORequest *) IOExtSer,NULL);
    if (Error) {
        printf("Error %d in open of serial.device unit %lu\n",Error,UnitNumber);
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

    if (TimeoutCheck) {
        /* Create Timer MsgPort */
        TimeMsgPort=CreatePort(NULL,0L);
        if (TimeMsgPort==NULL)
            SystemError("Error in creation of serial.device I/O request reply port\n");
        /* Create timerequest I/O Request */
        TimeRequest=(struct timerequest *) CreateExtIO(TimeMsgPort,sizeof(struct timerequest));
        if (TimeRequest==NULL)
            SystemError("Error in creation of timer.device timerequest\n");
        /* Open timer.device */
        Error=OpenDevice("timer.device",UNIT_MICROHZ,(struct IORequest *) TimeRequest,NULL);
        if (Error) {
            printf("Error %d in open of timer.device unit UNIT_MICROHZ\n",Error);
            ShutDown();
        }
        TimerDeviceOpen=TRUE;
    }

    /* Delay before transmission (if requested) */
    if (PreDelay!=0L) {
        printf("Delaying for %lu before starting transmission\n",PreDelay);
        TimeRequest->tr_node.io_Command=TR_ADDREQUEST;
        TimeRequest->tr_node.io_Flags=NULL;
        TimeRequest->tr_time.tv_secs=PreDelay;
        TimeRequest->tr_time.tv_micro=0L;
        DoIO((struct IORequest *) TimeRequest);
    }

    printf("Starting transmission\n");

    DateStamp(StartTime);

    /* Main loop */

    Errors=0L;

    BytesTransferred=0L;
    TimeoutFlag=FALSE;

    if (TimeoutCheck)
        WaitMask=(1<<SerMsgPort->mp_SigBit)|(1<<TimeMsgPort->mp_SigBit);
    else
        WaitMask=1<<SerMsgPort->mp_SigBit;

    /* If TimeoutCheck is FALSE, TimeoutFlag will remain false throughout run */
    while (BytesTransferred<ByteCount && !TimeoutFlag) {
        /* Send serial.device command to write pattern */
        IOExtSer->IOSer.io_Command=CMD_WRITE;
        IOExtSer->IOSer.io_Flags=QuickIO;
        if (BytesTransferred+PatternSize>ByteCount)
            IOExtSer->IOSer.io_Length=ByteCount-BytesTransferred;
        else
            IOExtSer->IOSer.io_Length=PatternSize;
        IOExtSer->IOSer.io_Data=Pattern;
        WriteLength=IOExtSer->IOSer.io_Length;
        if (TimeoutCheck) {
            SendIO((struct IORequest *) IOExtSer);
            /* Send timer.device command to wait for timeout period */
            TimeRequest->tr_node.io_Command=TR_ADDREQUEST;
            TimeRequest->tr_node.io_Flags=NULL;
            TimeRequest->tr_time.tv_secs=TimeoutSecs;
            TimeRequest->tr_time.tv_micro=TimeoutMicros;
            SendIO((struct IORequest *) TimeRequest);
            /* Wait for return of transmission and/or timeout */
            WaitSignal=Wait(WaitMask);
            if (WaitSignal&(1<<SerMsgPort->mp_SigBit)) {
                AbortIO((struct IORequest *) TimeRequest);
                /* Wait for timer.device abort return */
                Error=WaitIO((struct IORequest *) TimeRequest);
                /* Wait for serial.device write return */
                Error=WaitIO((struct IORequest *) IOExtSer);
                /* Clear serial.device and timer.device reply port signal bits */
                SetSignal(0L,WaitMask);
                /* Checksum update */
                Checksum+=PatternChecksum;
                /* Bytes transferred update */
                BytesTransferred+=WriteLength;
                /* Error count update */
                if (Error) {
                    printf("Error %d on write to serial.device on byte span %lu-%lu\n",Error,BytesTransferred-WriteLength,BytesTransferred-1);
                    Errors++;
                }
            } else if (WaitSignal&(1<<TimeMsgPort->mp_SigBit)) {
                AbortIO((struct IORequest *) IOExtSer); /* Abort serial.device request */
                Error=WaitIO((struct IORequest *) IOExtSer); /* Wait for abort return */
                Error=WaitIO((struct IORequest *) TimeRequest); /* Clear reply message from TimeMsgPort */
                /* Issue: handling of error from serial.device AbortIO */
                TimeoutFlag=TRUE; /* Set timeout flag */
                Error=FALSE;
            }
/*
 #ifdef DEBUG
 printf("Timeout check enabled mode transmit of byte %lu-byte %lu good\n",BytesTransferred-WriteLength,BytesTransferred);
 #endif
*/
        } else {
            Error=DoIO((struct IORequest *) IOExtSer);
            Checksum+=PatternChecksum;
            BytesTransferred+=WriteLength;
            if (Error) {
                printf("Error %d on write to serial.device on byte span %lu-%lu\n",Error,BytesTransferred-WriteLength,BytesTransferred-1);
                Errors++;
            }
/*
 #ifdef DEBUG
 printf("Timeout check disabled mode transmit of byte %lu-byte %lu good\n",BytesTransferred-WriteLength,BytesTransferred);
 #endif
*/
        }
    }

    DateStamp(StopTime);

    /* Delay before closing channel (if requested) */
    if (PostDelay!=0L) {
        printf("Delaying for %lu before closing channel\n",PostDelay);
        TimeRequest->tr_node.io_Command=TR_ADDREQUEST;
        TimeRequest->tr_node.io_Flags=NULL;
        TimeRequest->tr_time.tv_secs=PostDelay;
        TimeRequest->tr_time.tv_micro=0L;
        DoIO((struct IORequest *) TimeRequest);
    }

    if (TimeoutFlag)
        printf("*** Program terminated with transmit timeout ***\n");

    printf("Bytes transferred=%lu\n",BytesTransferred);
    printf("Errors=%lu\n",Errors);
    printf("Transmit checksum=$%08lx\n",Checksum);

    ElapsedTime=((StopTime[1]*60)+StopTime[2]/50)-((StartTime[1]*60)+StartTime[2]/50);
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
    printf("Transmit [UNIT <Unit Number>] [BAUD <Baud Rate>] [DATABITS <Data Bits>]\n");
    printf("    [STOPBITS <Stop Bits>] [PARITY <Parity>] [BUFFER <Buffer>]\n");
    printf("    [RAD_BOOGIE|NO_RAD_BOOGIE] [BYTES <Byte Count>]\n");
    printf("    [STARTPATTERN <Byte>[ <Byte>...] ENDPATTERN] [SHARED|EXCLUSIVE]\n");
    printf("    [XDISABLED] [7WIRE] [SECS <Secs>] [MICROS <Micros>]\n");
    printf("    [NOTIMEOUT] [PATTERNSIZE <Bytes>] [QUIET]\n");
    printf("    [POST_DELAY <Seconds>] [PRE_DELAY <Seconds>]\n");
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

    if (IOExtSer)
        DeleteExtIO((struct IORequest *) IOExtSer);

    if (SerMsgPort)
        DeletePort(SerMsgPort);

    if (TimerDeviceOpen)
        CloseDevice((struct IORequest *) TimeRequest);

    if (TimeRequest)
        DeleteExtIO((struct IORequest *) TimeRequest);

    if (TimeMsgPort)
        DeletePort(TimeMsgPort);

    if (Buffer)
        FreeMem(Buffer,BufferSize);

    if (Pattern)
        FreeMem(Pattern,PatternSize);

    exit(ReturnCode);
}
