#include "exec/types.h"
#include "exec/io.h"
#include "libraries/dosextens.h"
#ifdef junk
#include "intuition/intuition.h"
#endif

strlen(string)
UBYTE *string;
{
    LONG count;
    count = 0;
    while(string[count] != 0) {
        count++;
    }
    return(count);
}

copystr(dest, source)
    char    *dest;
    char    *source;
{
    while (*dest++ = *source++);
}

copydata(source, dest, length)
        char *source;
        char  *dest;
        int  length;
{
    int     i;

    for (i = 0; i < length; i++)
        *dest++ = *source++;
}

cmpstr(astr, bstr)
        char *astr;
        char *bstr;
{
    while (*astr && *bstr && (*astr == *bstr))
        astr++, bstr++;
    return ((*astr == *bstr));
}

#ifdef junk
sendpkt( link, id, type, res1, res2, arg1 )
long link,id,type,res1,res2,arg1;
{
    struct Message message;
    int i;

    link = (LONG)&message;  
    

    if ( qpkt(&link) == 0 ) {
        return( 0 );
    }


    if ( (i=taskwait()) != (LONG)&link ) {
        return( 0 );
    }


    return( res1 );
}

qpkt( data )
LONG *data;
{
    struct MsgPort *mp;
    struct Process *FindTask();
    struct Message *msg;

    msg = (struct Message *) data[0];
    mp =  (struct MsgPort *) data[1];

    data[1] = (LONG) &(FindTask( 0 )->pr_MsgPort);

    msg->mn_Node.ln_Name = data;

    PutMsg( mp, msg );

    return( 1 );
}

taskwait()
{
    struct Process *proc = FindTask(0);
    struct Message *msg;

    while( (msg = (struct Message *)GetMsg( &proc->pr_MsgPort )) == 0 ) {
        Wait( ~0 );
    }
 
    return( (int)(msg->mn_Node.ln_Name) );
}
#endif

ActionInhibit(Toggle,Drive)
LONG Toggle;
BYTE Drive;
{
LONG Task;
static UBYTE *CharDrive[4];

    CharDrive[0] = "DHx:";
    CharDrive[1] = "DH0:";
    CharDrive[2] = "DH1:";
    CharDrive[3] = "DH2:";

    Task = DeviceProc(CharDrive[Drive]);
/*    sendpkt(0,Task,ACTION_INHIBIT,0,0,Toggle); */
    return;
}
