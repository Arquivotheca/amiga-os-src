/* Defines ------------------------------------------------------------------- */
#define DEFAULT_REPS 30                /* default times to repeat a test */
#define MAX_PORTNAME_RETRIES 999       /* max number of Port_NNN's to try */
#define LOG_BUF_SIZE 128               /* max size of log message         */

/* You need to define PORT_TEMPLATE, PORT_TEMP_LEN, and PROGNAME.
   Port template should be "Progname_", template length should
   be length of PORT_TEMPLATE + number of digits in retries + 1,
   and PROGNAME is the name of your client program.
*/

#ifdef DEBUG
#define D(x) (x);
#else
#define D(x) ;
#endif


/* Globals.  This Code Shall Be Residentable --------------------------------- */
/* YOU NEED TO HAVE THIS DEFINED SOMEWHERE IN YOUR CODE!!! */
/* struct Globals {
    / Things common to all clients /
    struct RDArgs *rdargs;
    struct MsgPort *mp;
    struct DateTime dt;
    UBYTE portName[PORT_TEMP_LEN];
    SHORT rc;

    / Things specific to this client /

};
*/

extern struct ExecBase *SysBase;


/* Function Protos ----------------------------------------------------------- */
extern VOID bsprintf(UBYTE *target, UBYTE *format, ULONG args, ...);  /* asm */
extern VOID bsprintf2(UBYTE *target, UBYTE *format, VOID *args, ...);  /* asm */
VOID GoodBye(struct Globals *g);
BOOL sendMsg(struct memRequest *mr, struct Globals *g);
struct MsgPort *createClientPort(STRPTR portName);
struct MsgPort *findServerPort(VOID);
VOID dumpMemReq(struct memRequest *mr);
VOID aprintf(STRPTR fmt, LONG argv, ...);
BOOL servLog(struct memRequest *mr, struct Globals *g, STRPTR fmt, LONG argv, ...);
BOOL servAccess(LONG new, struct memRequest *mr, struct Globals *g);
BOOL servAlloc(ULONG bytes, LONG mode, struct memRequest *mr, struct Globals *g);
BOOL servFree(struct memRequest *mr, struct Globals *g);
VOID showValidNames(VOID);
VOID setBoard(STRPTR name, struct memRequest *mr);

/* declarations useful for looking up valid board specs */
struct lookUp {
    STRPTR name;
    LONG val;
};

static struct lookUp vals[] = {
    { "MB",     LOC_MOTHERBOARD },
    { "CPU",    LOC_CPU_BOARD },
    { "1",      LOC_BUS_1 },
    { "2",      LOC_BUS_2 },
    { "3",      LOC_BUS_3 },
    { "4",      LOC_BUS_4 },
    { "5",      LOC_BUS_5 },
    { "6",      LOC_BUS_6 },
    { "CHIP",   LOC_CHIP },
    { "FAST",   LOC_FAST },
    { "ANY",    LOC_ANY },
    { "ROM",    LOC_ROM },
    { NULL,     -1},
};
