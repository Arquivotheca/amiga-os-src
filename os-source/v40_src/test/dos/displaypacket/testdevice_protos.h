/* Prototypes for functions defined in
testdevice.c
 */

extern struct Library * SysBase;

extern struct DosLibrary * DOSBase;

extern struct Process * DosProc;

extern struct DeviceNode * DosNode;

extern struct DeviceList * DevList;

extern struct MsgPort * pid;

extern struct DosPacket * packet;

extern struct Message * msg;

extern struct MsgPort * HandReplyPort;

extern struct MsgPort * HandDataPort;

extern struct HAND_MESSAGE * h_message;

extern struct MinList LCBase;

extern int monitoring_packets;

extern ULONG generated_error;

extern ULONG real_Task;

extern ULONG real_Volume;

extern UWORD noLock;

void mane(void);

void FreeTestLock(struct FileLock * );

void GetTestLock(void);

LONG getOLDlock(LONG );

int restoreLock(struct DosPacket * );

void returnpacket(struct DosPacket * );

BOOL packetsqueued(void);

void * dosalloc(ULONG );

void dosfree(void * );

void btos(UBYTE * , UBYTE * );

struct MsgPort * setup_port(void);

void shutdown_port(struct MsgPort * );

void execute_command(struct HAND_MESSAGE * );

STRPTR typetostr(int );

void strCpy(STRPTR , STRPTR );

LONG strLen(STRPTR );

struct MsgPort * createPort(STRPTR , LONG );

void deletePort(struct MsgPort * );

void newList(struct List * );

