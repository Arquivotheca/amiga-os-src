#undef  BADDR
#define BADDR( bptr )   (((long)bptr) << 2)
#define BTOCSTR(bstr)   ((TEXT *)(BADDR(bstr) + 1))

#define ACTION_FIND_INPUT       1005
#define ACTION_FIND_OUTPUT      1006

#define DOS_TRUE	-1

#define ID_RAWCON ((('R' << 24L) | ('A' << 16L)) | ('W' << 8L))
#define ID_CON ((('C' << 24L) | ('O' << 16L)) | ('N' << 8L))

extern struct MsgPort  *CreatePort();
extern struct Process *GetProcessID();
extern struct Process  *GetProcess();
extern int returnpkt();
extern int qpkt();
extern struct DosPacket *taskwait();
