#include <exec/types.h>
#include <clib/exec_protos.h>
#ifdef _USEOLDEXEC_
#include <pragmas/exec_old_pragmas.h>
#else
extern struct ExecBase *SysBase;
#include <pragmas/exec_pragmas.h>
#endif

/*------ Common support library functions ---------*/
void BeginIO(struct IORequest *);
struct IORequest *CreateExtIO(struct MsgPort *, long);
struct MsgPort *CreatePort(char *, long);
struct IOStdReq *CreateStdIO(struct MsgPort *);
struct Task *CreateTask( UBYTE *name, long pri, APTR initPC,
	unsigned long stackSize );
void DeleteExtIO(struct IORequest *);
void DeletePort(struct MsgPort *);
void DeleteStdIO(struct IOStdReq *);
void DeleteTask(struct Task *);
void NewList(struct List *);
