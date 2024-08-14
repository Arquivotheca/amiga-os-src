/* Compile:
    lc -t -cs -HCommon_h.pre -j73 -oKillServer.o KillServer.c
    revit KillServer
    casm -a KillServer_rev.asm -o KillServer_rev.o
    blink from lib:c.o KillServer_rev.o KillServer.o to KillServer lib lib:lc.lib lib:amiga.lib sc sd nd define __main=__tinymain
    quit
*/
/* use precompiled headers for many things. */

/* KillServer:  25 Feb 92, J.W. Lockhart.
   Kills MemServer module of A2091 test code.
   Usage:  killserver
   Not tough :-)
 */

#define PROGNAME "KillServer"
#define MY_PORT "Dr_Death"

#ifdef DEBUG
#define D(x) (x);
#else
#define D(x) ;
#endif


VOID main(int argc, char *argv[]) {
    struct MsgPort *mp=NULL, *target=NULL;
    struct memRequest mr;
    struct DateTime dt;
    struct Message *msg=NULL;
    SHORT rc = RETURN_FAIL;

    Forbid();
        target = FindPort(SERVER_PORT);
        if (!target) {
            Permit();
            PutStr("Can't find server!\n");
            goto Bye;
        }
        /* check to see if some impatient soul has invoked us twice */
        if (!FindPort(MY_PORT))
            mp = CreatePort(MY_PORT, 0);
    Permit();
    if (!mp) {
        PutStr("Couldn't get message port!\n");
        goto Bye;
    }

    rc = RETURN_ERROR;
    memset(&mr, 0, sizeof(mr));
    memset(&dt, 0, sizeof(dt));
    dt.dat_Format = FORMAT_DOS;

    /* set up stuff the Server REQUIRES */
    mr.clientName = MY_PORT;
    mr.dt = &dt;

    /* Set up a command to write to the system log */
    mr.cmd = CMD_QUIT;

    /* make replying to us possible */
    mr.msg.mn_Node.ln_Type = NT_MESSAGE;
    mr.msg.mn_Length = sizeof(mr);
    mr.msg.mn_ReplyPort = mp;

    /* NB:  We don't use a "safe" PutMsg() here... we're the only
       program around which is expecting to find and kill off the
       server.  We're also a quick hack :-)
     */

    /* Send Message */
    PutStr("KillServer:  Sending termination command to Server...\n");
    DateStamp(&(dt.dat_Stamp));
    PutMsg(target, (struct Message *)&mr);
    D(PutStr("Client sleeping with WaitPort(my_port)\n"));

    /* Wait for reply */
    WaitPort(mp);
    D(PutStr("Client woke from WaitPort(mp)...\n"));
    while ((msg = GetMsg(mp)) != NULL) {
        /* Another assumption:  we're only going to see memRequest replies... */
        if (((struct memRequest *)msg)->rc2 = RC2_EXITING)
            PutStr("KillServer:  Server is shutting down.\n");
    }
    rc = RETURN_OK;

Bye:
    D(PutStr("Client in exit routine!\n"));
    if (mp) {
        while (msg = GetMsg(mp)) {
            D(PutStr("Got reply message!\n"));
        }
        DeletePort(mp);
        mp = NULL;
    }
    exit(rc);
}


