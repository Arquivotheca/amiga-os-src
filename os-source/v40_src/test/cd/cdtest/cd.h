/*
**  cd.device Test
**  Written by John J. Szucs
**  Copyright © 1993 Commodore-Amiga, Inc.
**  All Rights Reserved
*/

/*
**  Constants
*/

#define PROGRAM_NAME        "cd"            /* Program name */
#define KICKSTART_VERSION   40              /* Minimum Kickstart version */
#define WORKBENCH_VERSION   40              /* Minimum Workbench version */
#define REXX_VERSION        36              /* Minimum ARexx version */

#define REXXCOMMAND_LENGTH  32              /* Length of ARexx command buffer */
#define REXX_STEM_BUFFER 256                /* Length of buffer for ARexx stem variable names */
#define REXX_RESULT_BUFFER 256              /* Length of ARexx result string */
#define REXXSCRIPT_EXTENSION "cd"           /* Extension for ARexx scripts */

#define CDCONFIG_MAXTAGS    32              /* Maximum number of tags for CD_CONFIG */

#define COOKIE_CHANGEINT    0xC0EDBABE      /* Cookie for change interrupt data */
#define COOKIE_FRAMEINT     0xB16B00B5      /* Cookie for frame interrupt data */

/* Dummy cd.device calls */
/* #define DUMMY */

/*
**  Macros
*/

#ifndef FORVER
#define FOREVER for (;;)                    /* An -=*RJ*=-ism */
#endif /* FOREVER */

/*
**  Structure definitions
*/

/* Command table entry */
struct commandEntry {
    char    *command;                       /* Command */
    STRPTR  (*function)(struct RexxMsg *rxMsg,char *args,LONG *error);    /* Function */
};

/*
**  Global definitions
*/

#ifdef MAIN

struct RDArgs           *rdArgs=NULL;       /* dos.library/ReadArgs()
                                               control structure */

struct IOStdReq         *cdRequest=NULL;    /* cd.device I/O request */
struct MsgPort          *cdPort=NULL;       /* cd.device reply port */
BOOL                    cdOpen=FALSE;       /* cd.device open? */

AREXXCONTEXT            rexxContext;        /* ARexx context */
char                    rexxResult[REXX_RESULT_BUFFER];
                                            /* Buffer for ARexx result */

struct Interrupt        changeInterrupt;    /* Disk change interrupt */
ULONG                   changeIntCall=0L;   /* Call count for
                                               disk change interrupt */
ULONG                   changeIntBadData=0L;/* Bad data count for
                                               disk change interrupt */

struct Interrupt        frameInterrupt;     /* Frame interrupt */
ULONG                   frameIntCall=0L;    /* Call count for
                                               frame interrupt */
ULONG                   frameIntBadData=0L; /* Bad data count for
                                               frame interrupt */

BOOL                    debugMode=FALSE;    /* Debugging mode */
BOOL                    noCD=FALSE;         /* Dummy cd.device mode */

#else

extern struct RDArgs    *rdArgs;

extern struct IOStdReq  *cdRequest;
extern struct MsgPort   *cdPort;
extern BOOL             cdOpen;

extern AREXXCONTEXT     rexxContext;

extern char             rexxResult[];

extern struct Interrupt changeInterrupt;
extern ULONG            changeIntCall;
extern ULONG            changeIntBadData;

extern struct Interrupt frameInterrupt;
extern ULONG            frameIntCall;
extern ULONG            frameIntBadData;

extern BOOL             debugMode;
extern BOOL             noCD;

#endif /* MAIN */

/*
**  Prototypes
*/

/* from main.c */
void main(int argc,char *argv[]);
void goodbye(int returnCode);

/* from event.c */
void eventLoop(void);

/* from debug.c */
STRPTR testStemVarInt(struct RexxMsg *rxMsg,STRPTR args,LONG *error);

/* from args.c */
struct RDArgs *obtainArgs(STRPTR string,STRPTR template,void *options,ULONG optionsSize);
void releaseArgs(struct RDArgs *rdArgs);
ULONG strToMSF(STRPTR string);
LONG setStemVarInt(struct RexxMsg *rxMsg,STRPTR base,STRPTR stem,LONG value);
LONG setStemVarIntArray(struct RexxMsg *rxMsg,STRPTR base,LONG index,STRPTR stem,LONG value);

/* from bsprintf.a */
void bsprintf(char *buffer,char *format,...);

/* from cd.c */
STRPTR cdAddChangeInt(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
STRPTR cdAddFrameInt(struct RexxMsg *rxMsg,STRPTR  args,LONG *error);
STRPTR cdAttenuate(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
STRPTR cdChangeNum(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
STRPTR cdChangeState(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
STRPTR cdConfig(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
STRPTR cdEject(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
STRPTR cdGetGeometry(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
STRPTR cdInfo(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
STRPTR cdMotor(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
STRPTR cdPause(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
STRPTR cdPlayLSN(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
STRPTR cdPlayMSF(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
STRPTR cdPlayTrack(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
STRPTR cdProtStatus(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
STRPTR cdQCodeLSN(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
STRPTR cdQCodeMSF(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
STRPTR cdRead(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
STRPTR cdReadXL(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
STRPTR cdRemChangeInt(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
STRPTR cdRemFrameInt(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
STRPTR cdSearch(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
STRPTR cdSeek(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
STRPTR cdTOCLSN(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
STRPTR cdTOCMSF(struct RexxMsg *rxMsg,STRPTR args,LONG *error);

/* from verify.c */
BOOL byteVerify(UBYTE *buffer,ULONG length,UBYTE byte);
BOOL wordVerify(UBYTE *buffer,ULONG length,UWORD word);
BOOL offsetVerify(UBYTE *buffer,ULONG length,ULONG start);

/* from interrupt.c */
void __asm __interrupt __saveds changeInterruptCode(register __a1 APTR intData);
void __asm __interrupt __saveds frameInterruptCode(register __a1 APTR intData);
STRPTR getChangeInt(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
STRPTR getFrameInt(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
