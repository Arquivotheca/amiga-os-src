#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <libraries/dos.h>
#include <proto/dos.h>
#include <string.h>

long _stack = 4000;             /* a reasonable amount of stack space */
char *_procname = "Trap Handler";
long _priority = 0;             /* run at standard priority */
long _BackGroundIO = 1;         /* perform background I/O */
extern BPTR _Backstdout;        /* file handle we will write to with */

void NewBusError();
ULONG *vector, OldVector = NULL;
extern ULONG Base;


struct MsgPort *port, *FindPort(), *CreatePort();
void DeletePort();

#define MSG1 "Trap handler removed\n"
#define MSG2 "Trap handler installed\n"
#define MSG3 "Trouble allocating memory\n"

void CXBRK()
{
    Disable();
    vector = (ULONG *)(GetVBR()+8);
    vector[0] = (ULONG)OldVector;
    Enable();
}

void _main(argc, argv)
{
    if (argc  == 0)          /* called from Workbench, can't do anything */
	_exit(0L);

    if ((port=FindPort("Trap Handler")) != NULL)  /* I should put a Forbid() here */
	{
	    Signal(port->mp_SigTask,1<<(port->mp_SigBit));
	    Write(_Backstdout, MSG1, strlen(MSG1));
	    Close(_Backstdout);
	}
    else
	{
	    Disable();
		vector = (ULONG *)0x04;			/* Cache Execbase */
		Base = vector[0];
		vector = (ULONG *)(GetVBR()+8);
		OldVector = vector[0];
		vector[0] = (ULONG)&NewBusError;	/* Set up vector to new handler */
	    Enable();
	    if (!(port = CreatePort("Trap Handler",0)))
	    {
		CXBRK();
		Write(_Backstdout, MSG3, strlen(MSG3));
		Close(_Backstdout);
		_exit(10L);
	    }
	    Write(_Backstdout, MSG2, strlen(MSG2));
	    Close(_Backstdout);
	    Wait(1<<(port->mp_SigBit));
	    DeletePort(port);
	    CXBRK();
	}
}

