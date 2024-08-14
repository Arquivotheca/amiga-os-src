
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1987 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .	| || the author.					   BBS:      */
/* | o	| ||   John Toebes    Dave Baker		     (919)-471-6436  */
/* |  . |//								     */
/* ======								     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "netcomm.h"
#include "proto.h"
#include <ctype.h>

#if PARANOID
int paranoid = 1;
#endif

#if DEBUG

BPTR debuglog = NULL;
extern char *dbgwind;
static void makehex U_ARGS((char *, char *));
static void termlog U_ARGS((void));
static void xclose U_ARGS((BPTR));
static BPTR xopen  U_ARGS((char *));

BPTR
initdebug(fh)
BPTR fh;
{
    BPTR ofh;
    ofh = debuglog;
    if (!(debuglog = fh))
        debuglog = xopen(dbgwind);

   BUG(("DEBUGGING INITIALIZED\n"));

   return(ofh);
}

#define NETDBPORT "NET DEBUG PORT"

void maybedebug(void)
{
   struct MsgPort *port;
   struct 
   {
      struct Message m;
      BPTR dbfh;
   } *dbmsg;

   if((port = FindPort(NETDBPORT)) && (dbmsg = GetMsg(port)))
   {
      debuglog = dbmsg->dbfh;
      FreeMem(dbmsg, sizeof(*dbmsg));
   }
}

void
termdebug()
{
    BUG(("Closing log\n"));

    if (debuglog)
    xclose(debuglog);
    debuglog = NULL;
}

BPTR
xopen(name)
char *name;
{
    return(Open(name, 1006));
}

void
xclose(log)
BPTR log;
{
    long args[1];
    struct FileHandle *fh;

    if (!log)
	return;

    fh = (struct FileHandle *)BADDR(log);
    args[0] = (long)fh->fh_Arg1;
    sendpkt(fh->fh_Type,ACTION_END,args,1);
}

void
xwrite(str,len)
char *str;
int len;
{
    long args[3];
    struct FileHandle *fh;

    if(!debuglog)
	return;

    fh = (struct FileHandle *)BADDR(debuglog);
    args[0] = (long)fh->fh_Arg1;
    args[1] = (long)str;
    args[2] = (long)len;
    sendpkt(fh->fh_Type,ACTION_WRITE,args,3);
}

#define CRMSG "Hit RETURN to continue: "

void
xgetcr()
{
    long args[3];
    struct FileHandle *fh;
    char stuff[10];

    if (!debuglog)
	return;

    xwrite(CRMSG, strlen(CRMSG));
    fh = (struct FileHandle *)BADDR(debuglog);
    args[0] = (long)fh->fh_Arg1;
    args[1] = (long)stuff;
    args[2] = 9L;
    sendpkt(fh->fh_Type,ACTION_READ,args,3);
}

void
myprintf(str,p1,p2,p3,p4,p5,p6,p7,p8,p9)
char *str;
char *p1,*p2,*p3,*p4,*p5,*p6,*p7,*p8,*p9;
{
    char buf[128];
    int len;

    if (!debuglog)
	return;

    len = sprintf(buf,str,p1,p2,p3,p4,p5,p6,p7,p8,p9);
    if (len > 128)
	len = 128;
    xwrite(buf,len);
}

void
myputbstr(str, name)
char *str;
char *name;
{
    int len;

    if (!debuglog)
	return;

    xwrite(str, strlen(str));
    len = *name++;
    xwrite(name, len);
    xwrite("\n", 1);
}

void
myputlstr(str, name, len)
char *str;
char *name;
int len;
{
    if (!debuglog)
	return;

    xwrite(str, strlen(str));
    xwrite(name, len);
    xwrite("\n", 1);
}

static void
makehex(s, h)
char *s;
char *h;
{
    int i;
    static char *digits = "0123456789ABCDEF";

    if (!debuglog)
	return;

    for(i = 0; i < 8; i += 2) {
	h[i]   = digits[(s[i/2] & 0xf0) >> 4];
	h[i+1] = digits[(s[i/2] & 0x0f)     ];
    }
}

void
mydump(s, l)
unsigned char *s;
int l;
{
    int i, j;
    char h[9];

    if (!debuglog)
	return;

    h[8] = 0;
    BUG(("Dumping %ld bytes starting at addr %08.8x\n", l, s));
    for( i = 0; i < l; i += 16) {
	BUG(("%08.8lx: ", i));

	for (j = 0; j < 16; j += 4) {
	    makehex(&s[i+j], h);
	    if (i+j<l){
		BUG(("%8.8s ", h));
	    } else {
		BUG(("         "));
	    }
	}

	for (j = 0; j < 16 && i + j < l; j++) {
	    if (isprint(s[i+j])) {
		BUG(("%c", s[i+j]));
	    } else {
		BUG(("."));
	    }
	}
	BUG(("\n"));
    }
}

void
cprwait(global)
GLOBAL global;
{
    int i, j, oldpri;

    if (!request(global, REQ_GENERAL, "Loop for debug?"))
	return;

    BUG(("********* DEBUG WAIT LOOP ***********\n"))
    BUG(("******* CATCH TASK WITH CPR *********\n"))
    oldpri = SetTaskPri(FindTask(NULL), -20);
    i = 1;

    /* This loop will go until you set i to 0 with CPR */
    while(i) {
	BUG(("."))
	for(j = 0; i && j < 100000; j++);
    }

    SetTaskPri(FindTask(NULL), oldpri);
    return;
}
#endif