#include <exec/types.h>
#include <exec/ports.h>

void kpf();

/* struct mysx
   {
   struct Message mysx_msgstruct;
   char *mysx_msg;
   long mysx_p1;
   long mysx_p2;
   long mysx_p3;
   long mysx_p4;
   long mysx_p5;
   };


void writetomm(msg,t1,t2,t3,t4,t5)
char *msg;
long t1,t2,t3,t4,t5;
{

struct MsgPort *mynewport, *godport;
struct mysx *mynewioreq;

mynewport = (struct MsgPort *) CreatePort("Someone writing to MM",0L);
mynewioreq = (struct mysx *) CreateExtIO(mynewport,sizeof(struct mysx));

mynewioreq->mysx_msg = msg;
mynewioreq->mysx_p1 = t1;
mynewioreq->mysx_p2 = t2;
mynewioreq->mysx_p3 = t3;
mynewioreq->mysx_p4 = t4;
mynewioreq->mysx_p5 = t5;

godport = (struct MsgPort *) FindPort("Message Monitor");
if (!godport) return;

PutMsg((struct MsgPort *)godport,(struct Message *)mynewioreq);
WaitPort((struct MsgPort *)mynewport);
GetMsg(mynewport);
DeleteExtIO((struct IORequest *)mynewioreq);
DeletePort(mynewport);
}
 */

void writetomm(string,arg1,arg2,arg3,arg4,arg5)
{
kprintf(string,arg1,arg2,arg3,arg4,arg5);
}
