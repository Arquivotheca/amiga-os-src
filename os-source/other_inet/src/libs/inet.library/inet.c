/*
** startINET() instantiates the inet supervisor process.  The process
** calls the network clocks and handles events coming in from
** the various network device handlers.
*/

#include <exec/types.h>
#include <exec/tasks.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <devices/timer.h>
#include <dos/dosextens.h>
#include <stdarg.h>
#include <string.h>

#include <netinet/inet.h>

#ifdef LATTICE
#include <proto/exec.h>
#include <proto/dos.h>
#endif

#ifdef AZTEC_C
#include <functions.h>
#endif

#include <sys/types.h>
#include <sys/kernel.h>

#include "proto.h"

extern struct ExecBase *SysBase;

static char *inetname = "inet.supervisor";
static struct FileHandle *debugfh=NULL;
static struct timerequest tr;
struct Task *netpid;
static int netbit;
//static int slipbit, ifbit, aabit;

static long timerevent;
extern long s2event;
static void do_timeouts(void);

short  hz = 10;

#define STACKSIZE       (4*1024L)

int startINET()
{
        struct Process *pid=NULL;
        extern void NetSegment();

        Forbid();
        if(!FindTask(inetname))
                pid = (struct Process *)CreateProc(inetname, 10L, dtob(NetSegment), STACKSIZE);
        Permit();

        return pid ? 0:-1;
}

void killINET()
{
        if(netpid){
                Signal(netpid, SIGBREAKF_CTRL_C);
        }
}

void __saveds netPROCESS(void)
{
        register long event;
        long waitmask, netevent;
//        long ifevent, aaevent, slipevent;

        netbit = AllocSignal(-1L);
        if(netbit == -1){
                return;
        }
/*
        ifbit = AllocSignal(-1L);
        if(ifbit == -1){
                return;
        }
        aabit = AllocSignal(-1L);
        if(aabit == -1){
                return;
        }
        slipbit = AllocSignal(-1L);
        if(slipbit == -1){
                return;
        }
*/
        s2init(NULL);
        bzero(&tr, sizeof(tr));
        tr.tr_node.io_Message.mn_ReplyPort = CreateMsgPort();
        timerevent = 1L << tr.tr_node.io_Message.mn_ReplyPort->mp_SigBit;
        if (OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest *)&tr, 0L)) {
                return ;
        }

        tr.tr_node.io_Command = TR_ADDREQUEST;
        tr.tr_time.tv_secs = 0L;
        tr.tr_time.tv_micro = 1000000/hz;
        SendIO((struct IORequest *)&tr);

        netevent = 1L << netbit;
//        ifevent  = 1L << ifbit;
//        aaevent = 1L << aabit;
//        slipevent = 1L << slipbit;
//        waitmask = netevent | timerevent | ifevent | aaevent | slipevent;
        waitmask = netevent | timerevent | s2event;

        if(!netpid){
                netpid = FindTask(0L);
        }

        while(1){

                event = Wait(waitmask);

                Forbid();
                if(event & timerevent){
                        do_timeouts();
                }
/*
                if(event & ifevent){
                        aepoll();
                }

                if(event & aaevent)
                        aapoll();

                if(event & slipevent){
                        slpoll();
                }
*/
                if(event & s2event)
                        s2poll();

                if((event & netevent) || (SetSignal(0L, netevent) & netevent)){
                        rawintr();
                        ipintr();
                }

/*              if(event & SIGBREAKF_CTRL_C){
                        break;
                }
*/

                Permit();
        }

/*
        if(debugfh){
                Close((BPTR)debugfh);
                debugfh = 0;
        }

        netpid = 0;

        if(!CheckIO((struct IORequest *)&tr)){
                AbortIO((struct IORequest *)&tr);
                WaitIO((struct IORequest *)&tr);
        }
        DeletePort(tr.tr_node.io_Message.mn_ReplyPort);
        CloseDevice((struct IORequest *)&tr);

        return;
*/
}

void setsoftnet()
{
        if(netpid){
                Signal(netpid, 1L << netbit);
        }
}

/*
void setsoftif()
{
        if(netpid){
                Signal(netpid, 1L << ifbit);
        }
}

void setsoftifaa()
{
        if(netpid){
                Signal(netpid, 1L << aabit);
        }
}
*/

/*
 * Call the given function after <tick> timer ticks.
*/
#define MAX_TIMEOUTS 8
static struct to {
        void    (*func)(caddr_t);
        caddr_t arg;
        short   ticks;
} timeouts[MAX_TIMEOUTS];

int timeout(func, arg, ticks)
        void    (*func)(caddr_t);
        caddr_t arg;
        int     ticks;
{
        register struct to *to;

        for(to = &timeouts[MAX_TIMEOUTS]; --to >= timeouts;){
                if(func == to->func){
                        break;
                }
        }

        if(to < timeouts){
                for(to = &timeouts[MAX_TIMEOUTS]; --to >= timeouts;){
                        if(!to->func){
                                break;
                        }
                }
                if(to < timeouts){
                        return -1;
                }
        }

        to->ticks = ticks;
        to->arg = arg;
        to->func = ticks? func:0;

        return 0;
}

static void
do_timeouts()
{
        register struct to *to;

        tr.tr_node.io_Command = TR_ADDREQUEST;
        tr.tr_time.tv_secs = 0L;
        tr.tr_time.tv_micro = 1000000/hz;
        SendIO((struct IORequest *)&tr);

        for(to = &timeouts[MAX_TIMEOUTS]; --to >= timeouts;){
                if(to->func && (to->ticks-- == 0)){
                        (*to->func)(to->arg);
                        if(to->ticks == 0){ /* may be != 0 if func reset it */
                                to->func = 0;
                        }
                }
        }
}


void free()
{
        printf("someone called free!!\n");
}

/*
 * This here is catch anyone who does a kernel printf
 */


/* 2.0 ONLY! */
void printf(char *fmt, ...)
{
    va_list args;

    if (debugfh==NULL) {
        debugfh = Open("con:0/0/640/100/INET.LIBRARY", MODE_NEWFILE);
    }
    va_start(args,fmt);
    VFPrintf((BPTR) debugfh,(STRPTR)fmt,(LONG *)args);
    va_end(args);
}


