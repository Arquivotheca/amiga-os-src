
/*
 * RCP.C
 *
 * $Id: rcp.c,v 1.5 93/12/13 17:22:09 kcd Exp $
 *
 */

#include    <stdio.h>

#include    <exec/types.h>
#include    <exec/memory.h>

#include    <utility/tagitem.h>

#include    <dos/dosasl.h>

#include    <pragmas/exec_pragmas.h>
#include    <clib/exec_protos.h>
#include    <pragmas/dos_pragmas.h>
#include    <clib/dos_protos.h>

#include    <ss/socket.h>
#include    <netdb.h>
#include    <pwd.h>
#include    <string.h>

#include    "rcp_protos.h"
#include    "asyncio.h"

//
// Defines that relate to RCP
//
// MAXSOCKS is the value passed to setup_sockets()
#define MAXSOCKS    5

// A define used for converting Unix protection bits into AmigaDos bits.
// Used after creating a file or directory.
#define xfer(x) ( (~( ((x & 7) << 1) | ((x & 2) >> 1) )) & 0xf)


// BUFFERSIZE is the blocksize that RCP breaks transmissions up into
#define BUFFERSIZE  8192

//
// ReadArgs stuff
//
#define TEMPLATE "-R/S,-D/S,-F/S,-T/S,/M"
#define PUBLICTEMPLATE "-R/S,FROM/A,TO/A: "
#define OPT_RECURSIVE       0
#define OPT_DIRECTORY       1
#define OPT_REMOTEFROM      2
#define OPT_REMOTETO        3
#define OPT_STRINGS         4
#define OPT_TO              5
#define OPT_SOCKET          6
// #define OPT_COUNT           7

//
// Externals ...
//
extern struct Library *AbsExecBase;

//
// Greg's Nifty Global structure that contains all of the global variables
// we could -possibly- want for the whole darned program.
//
#if 0
struct GlobalData
{
    struct Library  *EBase;           // ExecBase (SysBase)
    struct Library  *DBase;           // DOSBase
    struct Library  *SBase;           // SockBase
    ULONG            errno;           // The lovely Unixish errno!
    ULONG            Socket;          // Socket this activity is happening on.
    UBYTE            Flags;           // Flags that relate to our operation (see below)
    UBYTE            Filler;          // To word-align things
    UBYTE            SourceUser[80];  // Usernames for both sides of the transfer
    UBYTE            DestUser[80];    // Ditto.
    UBYTE            SourceHost[80];  // For user-started rcp's, the name of the source file host
    UBYTE            DestHost[80];    // Same as above, but the name of the destination file host
    ULONG            Opts[OPT_COUNT]; // ReadArgs options
    UBYTE            RAIn[256];       // Huge buffers for ReadArgs.  :'(  Also used as general
    UBYTE            RAToken[256];    // Same as above.                   purpose buffers.
    ULONG	     rc;
};
#endif
// defines for the bit positions in the Flags field above
// Set if this operation is recursive.
#define FLG_RECURSIVE       1
// Set if this operation requires that the destination be a directory.
#define FLG_DESTDIR         2
// Set if this incarnation of rcp was started by a local user, and not by rshd.
#define FLG_LOCALUSAGE      4

//
// Some #defines to coerce SAS into redirecting the #pragmas through our
// oh-so-nifty global structure.
//
//#define SysBase     (g->EBase)
// The above statement can't be set until the global data area is set up
// and initialized.  Thus, the -real- #define for this is a few lines into
// the main() code.  Until then, SysBase will be defined to AbsExecBase.
#define SysBase AbsExecBase
#define DOSBase     (g->DBase)
#define SockBase    (g->SBase)
#define ASYNC_BUF_SIZE 65536


//
// Main entry point
//

ULONG rcp(void)
{

    struct GlobalData *g;

    g = (struct GlobalData *) AllocMem(sizeof(struct GlobalData),MEMF_CLEAR);
    if (g)
    {
        g->EBase = AbsExecBase;
        #undef SysBase
        #define SysBase (g->EBase)
        DOSBase = OpenLibrary("dos.library",37L);
        if (DOSBase)
        {
            struct RDArgs *rdargs;
            struct RDArgs *mra;
            ULONG notags[2]={TAG_DONE,0};

            // Since we're using 'MyReadArgs()', we need a RdArgs structure
            // ahead of time:
            mra = (struct RDArgs *) AllocDosObject(DOS_RDARGS,(struct TagItem *) &notags);
            if (mra)
            {
                // Do a ReadArgs(), deal with special cases
                if (rdargs = MyReadArgs(TEMPLATE, &g->Opts[0], mra,g))
                {
                    // Demand the most recent socket library, and open it
                    SockBase = OpenLibrary("inet:libs/socket.library",4);
                    if (SockBase)
                    {
                        int entries=0;
                        ULONG *n;
                        ULONG sockval;
                        BOOL remote;
                        struct servent *sp;
                        struct passwd *pwd;

                        // Let socket library know how many sockets we might want
                        // and let it init things
                        setup_sockets(MAXSOCKS,&g->errno);

                        // Look up the port # we should rcmd() on for 'rcp' (same as 'rsh')
                        sp = getservbyname("shell","tcp");
                        if (sp)
                        {
                            // Find information about this user (his username, particularly)
                            pwd = getpwuid(getuid());
                            if (pwd)
                            {
                                // Count # of entries in the /M field
                                // And second-guess a shitload of ReadArgs() stuff
                                // What I really want is:
                                // <multiple FROM strings>/M <exactly one TO string>/A <optional #>/N
                                // Unfortunately, since we can't qualify the 'TO' with
                                // something like "rcp FROM x y z TO a", ReadArgs can't
                                // find the end of the /M field and the start of the /A
                                // field, much less the optional /N field.  So, I:
                                // (1) Just grab FROM, TO, and the # as a big /M.
                                // (2) Check to see if I'm expecting that #.  If so, I use
                                //     last /M entry for it, and convert to a ULONG, and now
                                //     say that there's one less /M entry.
                                // (3) Grab the now-last /M entry for TO, and say that there's
                                //     now one less /M entry.
                                // (4) Whatever's left in the /M becomes 'FROM'.
                                // (5) I check to make sure that the right fields have
                                //     arguments, because in this case, ReadARgs() can't
                                //     do this for me.
                                //
                                if (g->Opts[OPT_STRINGS])
                                {
                                    n = (ULONG *) g->Opts[OPT_STRINGS];
                                    while (*n)
                                    {
                                        n++;
                                        entries++;
                                    }
                                    n = (ULONG *) g->Opts[OPT_STRINGS];
                                    if (g->Opts[OPT_REMOTEFROM] || g->Opts[OPT_REMOTETO])
                                    {
                                        sockval = myatol((char *) n[entries-1]);
                                        g->Opts[OPT_SOCKET] = (ULONG) &sockval;
                                        entries--;
                                    }
                                    if (entries)
                                    {
                                        g->Opts[OPT_TO] = n[entries-1];
                                        entries--;
                                    }
                                }

                                remote = ( g->Opts[OPT_REMOTEFROM] || g->Opts[OPT_REMOTETO] );

                                if (!remote && !entries)
                                    PrintFault(ERROR_REQUIRED_ARG_MISSING,"rcp");

                                // Okay, some quickie tests to ensure that the required arguments have
                                // been given.  Note that since ReadArgs() isn't straightforward
                                // for this program, this kind of pain is necessary.
                                // If this is a remote operation, we need a socket # and a TO.
                                // If local, we need at least one FROM, and a TO.
                                if (    (remote && g->Opts[OPT_SOCKET] && g->Opts[OPT_TO])    ||
                                        (!remote && g->Opts[OPT_TO] && entries)    )
                                {

                                    if (g->Opts[OPT_DIRECTORY])
                                    {
                                        g->Flags |= FLG_DESTDIR;        // Destination must be a directory
                                    }

                                    if (g->Opts[OPT_RECURSIVE])
                                    {
                                        g->Flags |= FLG_RECURSIVE|FLG_DESTDIR;      // Copy files recursively, thankyouverymuch
                                    }

                                    if (g->Opts[OPT_REMOTEFROM])        // They request that we send files
                                    {
                                        ULONG *m;
                                        // Own the socket that rshd is passing us.
                                        g->Socket = s_inherit( (void *) *((ULONG *)g->Opts[OPT_SOCKET]));
                                        // As long as there's a FROM ...
                                        if (g->Opts[OPT_STRINGS])
                                        {
                                            // For each FROM, do a transfer
                                            m = (ULONG *) g->Opts[OPT_STRINGS];
                                            while (entries)
                                            {
                                                FromUsToThem(g,(STRPTR)*m);
                                                m++;
                                                entries--;
                                            }
                                            // In this case, the entry I copied into 'TO' is actually
                                            // a 'FROM'.  I did this to avoid a bunch of special-casing.
                                            // In 99% of the cases, this is the only FromUsToThem()
                                            // done.
                                            FromUsToThem(g,(STRPTR)g->Opts[OPT_TO]);
                                        }
                                        s_close(g->Socket);
                                    }

                                    if (g->Opts[OPT_REMOTETO])          // They request that we receive files
                                    {
                                        // Own the socket that rshd is passing us
                                        g->Socket = s_inherit( (void *) *((ULONG *)g->Opts[OPT_SOCKET]));
                                        // Get into a mode where we're at their beck and call
                                        ToUsFromThem(g,(STRPTR)g->Opts[OPT_TO]);
                                        s_close(g->Socket);
                                    }

                                    // This is when WE request something.  A local request.
                                    if (!remote)
                                    {
                                        int ctr;
                                        STRPTR equals, topath;

                                        g->Flags |= FLG_LOCALUSAGE;     // Mark this as local usage, not remote

                                        // There's two code segments here, one for TO and one inside
                                        // a loop that handles the same operation for every FROM.
                                        //
                                        // It tries to split up the string into (optionally)
                                        // three fields -- user, host, and path.  The
                                        // given format is user@host=path.  Only 'path' is
                                        // required; user cannot exist without host.
                                        //

                                        // Sort out 'To' path vs. host
                                        equals = (STRPTR) strchr((char *)g->Opts[OPT_TO],'='+128);
                                        if (equals)
                                        {
                                            STRPTR atsym;
                                            int r;
                                            topath = &equals[1];
                                            r = ((ULONG) equals - (ULONG) g->Opts[OPT_TO]);
                                            strdncpy(g->DestHost,(char *)g->Opts[OPT_TO],MIN(r,79),g);

                                            // Is there an '@' symbol in it?
                                            atsym = (STRPTR) strchr(g->DestHost,'@');
                                            if (atsym)
                                            {
                                                int r;
                                                r = (ULONG) atsym - (ULONG) g->DestHost;
                                                strdncpy(g->DestUser,g->DestHost,MIN(r,79),g);
                                                strcpy(g->DestHost,&atsym[1]);
                                            }
                                        }
                                        else
                                            topath = (STRPTR) g->Opts[OPT_TO];

                                        // For each 'from' entry, try to split it up into host & path portions
                                        for (ctr=0; ctr < entries; ctr++)
                                        {
                                            STRPTR froms, frompath;

                                            froms = (STRPTR) ((ULONG *)g->Opts[OPT_STRINGS])[ctr];
                                            equals = (STRPTR) strchr(froms,'='+128);
                                            g->SourceHost[0]=0;
                                            if (equals)
                                            {
                                                STRPTR atsym;
                                                int cl;

                                                frompath = &equals[1];
                                                cl = ( (ULONG) equals - (ULONG) froms );
                                                strdncpy(g->SourceHost,froms,MIN(cl,79),g);

                                                // Is there an '@' symbol in it?
                                                atsym = (STRPTR) strchr(g->SourceHost,'@');
                                                if (atsym)
                                                {
                                                    int r;
                                                    r = (ULONG) atsym - (ULONG) g->SourceHost;
                                                    strdncpy(g->SourceUser,g->SourceHost,MIN(r,79),g);
                                                    strcpy(g->SourceHost,&atsym[1]);
                                                }
                                            }
                                            else
                                                frompath = froms;

                                            // We need to determine whether they're trying to do wildcards
                                            // or not.  If so, the destination MUST BE A DIRECTORY.

                                            if (IsWild(frompath,g))
                                                g->Flags |= FLG_DESTDIR;

                                            // If both are remote or both are local, announce that
                                            // we can't handle their request.  Not coded yet.
                                            // May never be.  Two ways to do a remote-remote:
                                            // (1) Use rcmd() to get the source machine to
                                            //     do it; that way it's a local->remote operation
                                            //     for it.
                                            // (2) Do a source->us  us->destination operation;
                                            //     essentially do it twice, using us as the
                                            //     middleman.
                                            //
                                            //     Local-local can probably be implemented
                                            //     easiest by implying a hostname and some path
                                            //     extension.  We'll have a source and a dest
                                            //     rcp running, but it'll work.  :')
                                            //


                                            if ( ( g->Opts[OPT_REMOTEFROM] ^ g->Opts[OPT_REMOTETO] ) ||
                                                 (g->SourceHost[0] && g->DestHost[0]) ||
                                                 (!g->SourceHost[0] && !g->DestHost[0]) )

                                            {
                                                PutStr("rcp: Local-Local or Remote-Remote copies not supported\n");
                                                g->rc = 20;
                                            }
                                            else
                                            {
                                                STRPTR targethost;
                                                STRPTR suser, duser;

                                                if (g->SourceHost[0])
                                                {
                                                    targethost = g->SourceHost;
                                                    suser = g->SourceUser;
                                                    duser = g->DestUser;
                                                    sprintf(g->RAIn,"rcp%s%s -f %s",
                                                            (g->Flags & FLG_RECURSIVE) ? " -r" : "",
                                                            (g->Flags & FLG_DESTDIR) ? " -d" : "",
                                                            frompath);
                                                    if (!suser[0])
                                                        suser = pwd->pw_name;
                                                    if (!duser[0])
                                                        duser = pwd->pw_name;
                                                    g->Socket = rcmd(&targethost,sp->s_port, suser,
                                                                     duser, g->RAIn, 0);
                                                    if (g->Socket != -1L)
                                                    {
                                                        ToUsFromThem(g,(STRPTR)topath);
                                                        s_close(g->Socket);
                                                    }
                                                    else
                                                    {
                                                        PutStr("rcp: ");
                                                        PutStr(strerror(g->errno));
                                                        PutStr("\n");
                                                        g->rc = 20;
                                                    }
                                                }
                                                else
                                                {
                                                    targethost = g->DestHost;
                                                    suser = g->SourceUser;
                                                    duser = g->DestUser;
                                                    sprintf(g->RAIn,"rcp%s%s -t %s",
                                                            (g->Flags & FLG_RECURSIVE) ? " -r" : "",
                                                            (g->Flags & FLG_DESTDIR) ? " -d" : "",
                                                            topath);
                                                    if (!suser[0])
                                                        suser = pwd->pw_name;
                                                    if (!duser[0])
                                                        duser = pwd->pw_name;
                                                    g->Socket = rcmd(&targethost,sp->s_port, suser,
                                                                     duser, g->RAIn, 0);
                                                    if (g->Socket != -1L)
                                                    {
                                                        FromUsToThem(g,(STRPTR)frompath);
                                                        s_close(g->Socket);
                                                    }
                                                    else
                                                    {
                                                        PutStr("rcp: ");
                                                        PutStr(strerror(g->errno));
                                                        PutStr("\n");
                                                        g->rc = 20;
                                                    }
                                                }

                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                PutStr("rcp: local user id not set -- check inet:db/passwd file\n");
                                g->rc = 20;
                            }
                        }
                        else
                        {
                            PrintFault(ERROR_REQUIRED_ARG_MISSING,"rcp");
			    g->rc = 20;
			}
                        cleanup_sockets();

                        CloseLibrary(SockBase);
                    }
                    else
                    {
                        PutStr("rcp: Couldn't open socket.library\n");
                        g->rc = 20;
                    }
                    FreeArgs(rdargs);
                }
                else
                {
                    PrintFault(IoErr(),"rcp");
                    g->rc=20;
                }
            }
            else
                FreeDosObject(DOS_RDARGS,mra);
            CloseLibrary(DOSBase);
        }
        FreeMem(g,sizeof(struct GlobalData));
        #undef SysBase
        #define SysBase AbsExecBase
    }
    return(g->rc);
}

#undef SysBase
#define SysBase (g->EBase)

#define RESP_OK         0       /* Success! */
#define RESP_FAIL       -1      /* Failure of this operation */
#define RESP_FAILEXIT   -2      /* Failure of the entire command */

//
// GetResponse() -- gets a one byte response code and a possible error string.
// If the response code is RESP_OK, the routine exits.  If not, it attempts to
// fetch all of the ascii data up to a linefeed character, and print that
// string out as a remote error code.
//
// Entry:
//          g - global data ptr
//
// Exit:
//          RESP_OK, RESP_FAIL, RESP_FAILEXIT.  See above.
//
int GetResponse(struct GlobalData *g)
{
    char abyte;                 // Storage area for one-byte reads

    int s;                      // Socket this is happening on

    s = g->Socket;

    if (recv(s,&abyte,1,0) < 0) // Get a response byte or exit
        return(RESP_FAILEXIT);

    switch (abyte)              // Deal with this response code
    {

        case 0:
            return(RESP_OK);

        case 1:
        case 2:
        {
            char msgbuf[128];
            int x=0;
            while ( (x < 128) & (recv(s,&msgbuf[x],1,0) == 1) ) // Continue as long as we still have buffer space and the connection hasn't closed
            {
                if (msgbuf[x] == '\n')
                {
                    msgbuf[x+1]=0;
                    Write(Output(),msgbuf,strlen(msgbuf));
                    break;
                }
                x++;
            }
            if (abyte == 1)
                return (RESP_FAIL);
            return(RESP_FAILEXIT);
        }
    }
}

//
// SendError() -- Sends and prints an error string
// If the current operation is local, the given error is printed
// to this process' stdout.  Regardless, the error code is
// sent to the destination machine.
//
// Entry:
//      g      - pointer to the global data area
//      errmsg - a string, null terminated, with a leading character
//               in the RCP format, describing the seriousness of the
//               error:
//                      00 - no error
//                      01 - failure
//                      02 - complete failure
//
// Exit:
//      none
//
void SendError(struct GlobalData *g, STRPTR errmsg)
{

    if (g->Flags & FLG_LOCALUSAGE)
        Write(Output(),&errmsg[1],strlen(&errmsg[1]));

    send(g->Socket,errmsg,strlen(errmsg),0);
}

//
// GetUnixData() -- Find out the file 'mode' (protection bits) and return them
//                  in the Unixy octal format.  Also find and return the
//                  file size.
//
// Entry:
//          globalptr -- ptr to global data area
//          filename -- file to find data on
//          mode     -- ptr to a ulong to store the mode bits in
//          size     -- ptr to a ulong to store the file size in
//
// Exit:
//          status   -- success/failure boolean.  TRUE/FALSE.
//
BOOL GetUnixData(struct GlobalData *g, STRPTR filename, ULONG *mode, ULONG *size)
{

    BPTR l;
    BOOL ret=FALSE;

    l = Lock(filename,ACCESS_READ);
    if (l)
    {
        struct FileInfoBlock *f;
        ULONG notags[2]={TAG_DONE,0};
        f = (struct FileInfoBlock *) AllocDosObject(DOS_FIB,(struct TagItem *) &notags);
        if (f)
        {
            if (Examine(l,f))
            {
                ULONG p;
                *size = f->fib_Size;
                p = ((~(f->fib_Protection)) & 0xf);     // Grab rwed
                p = (p >> 1);                           // Make rwx
                p = (p | (p << 3) | (p << 6));          // Make rwxrwxrwx
                *mode =p;
                ret = TRUE;
            }
            FreeDosObject(DOS_FIB,f);
        }
        UnLock(l);
    }

    return(ret);
}


//
// SendFile() -- send a given file or directory to a remote machine.
//
// Entry:
//          g - global data area ptr
//          filename - name of the file to send
//
//
// Exit:
//          Returns status.  RESP_OK, RESP_FAIL, RESP_FAILEXIT
//

int SendFile(struct GlobalData *g, STRPTR filename)
{

    STRPTR justname;        // Ptr to the file portion of the path
    UBYTE cmd[128];
    ULONG size, mode;
    int r=RESP_FAIL;
    int s;                  // Socket ptr

    s = g->Socket;          // Get this from the globdata struct

    // Create a pointer into the filename string that represents
    // the filename only -- and not the full path.
    justname = (STRPTR) strrchr(filename,'/');
    if (!justname)
    {
        justname = (STRPTR) strrchr(filename,':');
        if (!justname)
            justname = cmd;
        else
            justname++;
    }
    else
        justname++;

    if (GetUnixData(g,filename,&mode,&size))
    {
        sprintf(cmd,"C%04o %ld %s\n",mode,size,justname);
        send(s,cmd,strlen(cmd),0);
        r = GetResponse(g);
        if (!r)
        {
            APTR buff;

            buff = (APTR) AllocMem(BUFFERSIZE,0);
            if (buff)
            {
                struct ASyncFile *fh;
                fh = OpenAsync(g, filename,MODE_READ,ASYNC_BUF_SIZE);
                if (fh)
                {
                    // Send them the data itself; broken up into BUFFERSIZE chunks
                    ULONG left,thisblock;
                    left = size;
                    while (left)
                    {
                        thisblock = MIN(BUFFERSIZE,left);
                        if (ReadAsync(g,fh,buff,thisblock) == thisblock)
                        {
                            send(s,buff,thisblock,0);
                        }
                        else
                            break;
                        left -= thisblock;
                    }
                    send(s,"\x00",1,0);
                    r = GetResponse(g);
                    CloseAsync(g,fh);
                }
                FreeMem(buff,BUFFERSIZE);
            }
        }
    }
    return(r);
}

//
// ChangeDirectory -- Force server to step into a directory
//
// Entry:
//      g       - global data area ptr
//      dirname - directory name to step into (or create -- their problem)
//
// Exit:
//      status  - success codes (RESP_OK, RESP_FAIL, RESP_FAILEXIT)
//
int ChangeDirectory(struct GlobalData *g, STRPTR path)
{
    STRPTR justname;
    UBYTE cmd[128];
    ULONG mode, size;
    int r;
    int s;

    s = g->Socket;  // Set up socket ptr

    // Create a pointer into the dirname string that represents
    // the direname only -- and not the full path.
    justname = (STRPTR) strrchr(path,'/');
    if (!justname)
    {
        justname = (STRPTR) strrchr(path,':');
        if (!justname)
            justname = cmd;
        else
            justname++;
    }
    else
        justname++;

    if (!GetUnixData(g,justname,&mode,&size))
    {
        mode = 0x1ff;
    }

    sprintf(cmd,"D%04o %ld %s\n",mode,0,justname);
    send(s,cmd,strlen(cmd),0);
    r = GetResponse(g);

    return(r);
}

//
// ExitDirectory -- Force server to back out one directory
//
// Entry:
//      g       - global data area ptr
//
// Exit:
//      status  - response codes
//
int ExitDirectory(struct GlobalData *g)
{
    int s;
    s = g->Socket;
    send(s,"E\n",2,0);
    return(GetResponse(g));
}

//
// FromUsToThem -- send a specific file (or files) across the net
//
// The routine is one of two that RCP requires.  This transfers
// files from the Amiga on which it's running to another machine.
//
// Entry:
//      g       - global data area ptr
//      path    - full path to the file to transfer (wildcards accepted)
//
// Exit:
//      Status  - One of the RESP_ codes.
//
int FromUsToThem(struct GlobalData *g, STRPTR path)
{
    struct AnchorPath *ap;
    int r=RESP_FAILEXIT;

    GetResponse(g);

    ap = (struct AnchorPath *) AllocMem(sizeof(struct AnchorPath) + 255, MEMF_CLEAR);
    if (ap)
    {
        int err;

        r = RESP_OK;
        ap->ap_Flags = APF_DOWILD;
        ap->ap_BreakBits = SIGBREAKF_CTRL_C;
        ap->ap_Strlen = 255;
        err = MatchFirst(path,ap);
        if (!err)
        {
            // Okay, do the recursive tree search
            // The MatchFirst/MatchNext pair simply step through the
            // directory tree, and when they encounter a subdirectory,
            // they immediately step into it.  When a directory
            // is found, a ChangeDirectory() is done, to convince the
            // client to step into (or create) the next destination.
            // When a file is found, a SendFile() is done.  A ExitDirectory()
            // is done when exiting a subdirectory.
            do
            {
                STRPTR fn=(STRPTR) (&(ap->ap_Buf[0]));

                // If done with a directory, step out of it
                if (ap->ap_Flags & APF_DIDDIR)
                {
                    ap->ap_Flags &= ~APF_DIDDIR;
                    r = ExitDirectory(g);
                    if (r)
                        break;
                    continue;
                }

                // If a new dir is found, step into it.
                if (ap->ap_Info.fib_DirEntryType >= 0)
                {
                    if (g->Flags & FLG_RECURSIVE)
                    {
                        ap->ap_Flags |= APF_DODIR;
                        r = ChangeDirectory(g,fn);
                        if (r)
                            break;
                    }
                }

                // If a file, send it.
                if (ap->ap_Info.fib_DirEntryType < 0)
                {
                    r = SendFile(g,fn);
                    if (r == RESP_FAILEXIT)
                        break;
                }

            }
            while (!MatchNext(ap));

        }
        else
        {
            err = IoErr();
            if (err != ERROR_NO_MORE_ENTRIES)
            {
                Fault(err,"",g->RAIn,254);
                // Force the Fault() output into an appropriate RCP error msg
                sprintf(g->RAToken,"\002rcp%s\n",g->RAIn);
                SendError(g,g->RAToken);
                GetResponse(g);
            }
        }
        FreeMem(ap,sizeof(struct AnchorPath) + 255);
    }
    return(r);
}

//
// ToUsFromThem -- receive a file (or files) from a remote host
//
// This routine is the second of two required modes that RCP uses.
// Given an operative socket, it will wait for commands from the
// remote machine, telling it to move into certain directories,
// receive files, etc.
//
// Entry:
//      g       - global data area ptr
//      f       - path to receive data to
//
// Exit:
//      Response code -- RESP_OK, RESP_FAIL, RESP_FAILEXIT
//
int ToUsFromThem(struct GlobalData *g, STRPTR f)
{
    int r=RESP_FAILEXIT;
    int s=g->Socket;
    UBYTE cmd[128];
    UBYTE forcename[128];
    STRPTR errstring;
    BOOL force=FALSE;
    int x;
    BOOL cont=TRUE;
    UBYTE code=0;

    BPTR oldcd, newcd;

    // First, we need to CurrentDir() into the destination directory.
    // This is a bit of a problem, as we have a complicated situation.
    // We're given a flag that identifies wildcard & recursive situations.
    // (which is provided by the host); we also don't know if we're being
    // given a path to a file (which might not even exist yet!) or the name
    // of a destination directory.
    //
    // To deal with this insanity (which is made worse by the fact that
    // everybody has their own ideas on what the computer should do, given
    // the extremely vague commands they enter ...), I generate a 3-bit
    // code that identifies the situations:
    //
    // DIRNEEDED is true if the destination MUST be a directory.
    // FOUND is true if we were able to get some sort of lock on the
    // destination path the host gave us.
    // FILE? is true if the lock created above shows that the path
    // identifies a FILE, and not a directory.  (Only valid, of course,
    // when FOUND is true..)
    //
    // Several results are possible:
    //
    // #1: Each file each transmitted with the filename as it exists on
    //     the source.  If someone wishes to transfer a file and create it
    //     on the destination as a different name than it existed on the
    //     source, we have to know this and deal with it.  Essentially,
    //     We just sense the condition, and override the filename passed
    //     with the file later.
    //
    // #2  If someone does something like:  rcp myfile itchy:ram:myfile,
    //     the path "ram:myfile" may represent something that doesn't exist.
    //     If it doesn't, we have to assume that they didn't intend it to
    //     be a directory path, but instead, the destination filename.
    //     (Which hasn't been created yet.)  To find the destination
    //     DIRECTORY, we must strip the filename from the path, and use
    //     the result as the directory path.
    //
    // #3  Nothing special.  Use the path given as a directory path,
    //     and use the name(s) passed with the file(s) to create the
    //     files in this directory.
    //
    // Several errors are possible, too.
    //
    // DIRNEEDED    FOUND       FILE?       Operation?
    // -----------------------------------------------------------------
    //  F           F           F           (1)(2)
    //  F           F           T           (1)(2)
    //  F           T           F           (3)
    //  F           T           T           (1)(2)
    //  T           F           F           Dest Dir not found.  Create it.
    //  T           F           T           Dest Dir not found.  Create it.
    //  T           T           F           (3)
    //  T           T           T           ERROR -- Destination must be a directory
    //

    // If either flag is set, the destination must be a directory.  Mark DIRNEEDED.
    if (g->Flags & (FLG_RECURSIVE | FLG_DESTDIR))
        code |= 4;

    newcd = Lock(f,ACCESS_READ);
    if (newcd)
    {
        struct FileInfoBlock *fb;
        ULONG notags[2]={TAG_DONE,0};
        // Since found, we mark FOUND.
        code |= 2;
        fb = (struct FileInfoBlock *) AllocDosObject(DOS_FIB,(struct TagItem *) &notags);
        if (fb)
        {
            if (Examine(newcd,fb))
            {
                // If a file, we need to mark FILE?.
                if (fb->fib_DirEntryType < 0)
                    code |= 1;
            }
            FreeDosObject(DOS_FIB,fb);
        }
    }

    // We now have our 3-bit code.
    switch (code)
    {
        case 0:
        case 1:
        case 3:
        {
            STRPTR y;
            // Unlock the old lock
            UnLock(newcd);

            y = (STRPTR) strchr(f,'/');
            if (!y)
            {
                y = (STRPTR) strchr(f,':');
                if (!y)
                {
                    f[0]=0;
                }
            }
            if (y)
            {
                strcpy(forcename,&y[1]);
                y[1]=0;
            }

            // Okay, 'f' points to the directory path, and we copied the filename to force
            // to into 'forcename'.

            // Try to get a dir lock
            newcd = Lock(f,ACCESS_READ);
            force = TRUE;
            if (!newcd)
                errstring = "\002rcp: Destination directory not found.\n";
            break;
        }
        case 4:
        case 5:
        {
            BPTR tlock;
            // Create the directory
            tlock = CreateDir(f);
            if (tlock)
            {
                newcd = ParentDir(tlock);           // for compatibility w/ old rcp
                UnLock(tlock);
//                newcd = Lock(f,ACCESS_READ);
                errstring = "\002rcp: Destination directory not found.\n";
            }
            else
                errstring = "\002rcp: Can't create directory.\n";
            break;
        }
        case 7:
        {
            UnLock(newcd);
            newcd=0;
            errstring = "\002rcp: Destination must be a directory.\n";
            break;
        }
        // Cases 2 and 6 don't require anything special.
    }


    // Things start to make sense again from here on down ... :')

    if (newcd)
    {
        // Since we have a lock on the directory, change our cd into it ...

        oldcd = CurrentDir(newcd);

        // We're required to send an "ack" of sorts, to verify that we did
        // indeed find a destination directory.
        send(s,"\000",1,0);

        // Okay, now that we're in the right directory, sit in a loop and receive
        // commands from the host.  Pretty simple, actually -- just sit here and grab
        // line after line until we either receive errors or the other side closes
        // the connection.
        while (cont)
        {
            // Read in the command string -- note that it's terminated by a linefeed
            x = 0;
            while (TRUE)
            {
                if (recv(s,&cmd[x],1,0) == 1)
                {
                    if (cmd[x++] == '\n')
                        break;
                }
                else
                {
                    cont=FALSE;     // recv failed -- break all the way out.
                    r=RESP_OK;      // Socket probably closed; natural ending -- exit "a-okay".
                    break;
                }
            }
            cmd[x]=0;               // Null terminate

            // If not trying to exit all of the way out, try to process
            // the command line we just received.
            if (cont)
            {

                // Check to see if we've been given an error code
                if ((cmd[0] == 1) || (cmd[0] == 2))
                {
                    Write(Output(),"rcp: ",5);
                    Write(Output(),&cmd[1],strlen(&cmd[1]));
                    if (cmd[0] == 2)        // If a major error, exit entirely
                        break;
                }
                else
                {
                    // Actually try to interpret the command

                    switch(cmd[0])
                    {
                        // Directory command.  Format is 'D#### 0 dirname'.  ####=mode.
                        // We need to CD to that directory, or make it if it doesn't exist.
                        case 'D':
                        {
                            STRPTR fname;
                            ULONG mode, tmode;

                            // The following is an octal string->ULONG conversion.
                            // I've had VERY flakey operation out of SAS's atol() routine
                            // so I trust none of them.
                            cmd[1] -= '0';
                            cmd[2] -= '0';
                            cmd[3] -= '0';
                            cmd[4] -= '0';
                            mode = cmd[1]*256+cmd[2]*64+cmd[3]*8+cmd[4];
                            // Although AS225 doesn't understand it, Envoy DOES
                            // understand the other protection bits.  So, I'll do a little
                            // reorganization just for that.

                            //                  rwx become rwe; w becomes d too.
                            //                  ~ flips the bits to AmigaDos reverse-logic
                            //                  & 0xf keeps the bottom portion only.  :')
                            tmode = xfer(mode >> 6) | (xfer(mode >> 3) << 8) | (xfer(mode) << 12);
                            //           owner               group                   other

                            // Find the filename by skipping past the space past the filesize.
                            fname = &cmd[5];
                            if (TRUE)
                            {
                                fname = (STRPTR) strchr(&fname[1],' ');
                                if (fname)
                                {
                                    int x=0;
                                    fname++;

                                    // Null out the linefeed.
                                    while ( (fname[x]) && (fname[x] != '\n') )
                                        x++;
                                    fname[x]=0;

                                    // Get a lock on the next dir; change to it; tell host "a-ok!"
                                    newcd = Lock(fname,ACCESS_READ);
                                    if (newcd)
                                    {
                                        BPTR xcd;
                                        xcd = CurrentDir(newcd);
                                        UnLock(xcd);
                                        send(s,"\x00",1,0);
                                    }
                                    else
                                    {
                                        // Must not exist already.  Create it!
                                        if (newcd = CreateDir(fname))
                                        {
                                            BPTR xcd;
                                            UnLock(newcd);
                                            SetProtection(fname,tmode);
                                            newcd = Lock(fname,ACCESS_READ);
                                            if (newcd)
                                            {
                                                xcd = CurrentDir(newcd);
                                                UnLock(xcd);
                                                send(s,"\x00",1,0);
                                            }
                                            else
                                            {
                                                SendError(g,"\002rcp: Can't access directory!\n");
                                                cont = FALSE;
                                            }
                                        }
                                        else
                                        {
                                            SendError(g,"\002rcp: Can't create directory!\n");
                                            cont = FALSE;
                                        }
                                    }
                                }
                            }
                            break;
                        }

                        // Exit Directory command.  CD to the parent of where we are.
                        // Format is 'E'.
                        case 'E':
                        {
                            BPTR xcd;
                            xcd = newcd;
                            newcd = ParentDir(xcd);
                            CurrentDir(newcd);
                            UnLock(xcd);
                            send(s,"\x00",1,0);
                            break;
                        }

                        // Receive file command.  Format is 'Cmode filesize filename'
                        // Where mode is the 4 digit Unix filemode (protections),
                        // filesize is the size of the file in bytes, and filename is
                        // the name of the file to receive to -- IN THIS DIRECTORY.
                        case 'C':
                        {
                            STRPTR fname;
                            STRPTR tmp;
                            ULONG size;
                            ULONG mode, tmode;

                            // The following is an octal string->ULONG conversion.
                            // I've had VERY flakey operation out of SAS's atol() routine
                            // so I trust none of them.
                            cmd[1] -= '0';
                            cmd[2] -= '0';
                            cmd[3] -= '0';
                            cmd[4] -= '0';
                            mode = cmd[1]*256+cmd[2]*64+cmd[3]*8+cmd[4];
                            // Although AS225 doesn't understand it, Envoy DOES
                            // understand the other protection bits.  So, I'll do a little
                            // reorganization just for that.

                            //                  rwx become rwe; w becomes d too.
                            //                  ~ flips the bits to AmigaDos reverse-logic
                            //                  & 0xf keeps the bottom portion only.  :')
                            tmode = xfer(mode >> 6) | (xfer(mode >> 3) << 8) | (xfer(mode) << 12);
                            //      owner               group                   other

                            // Find the filename by skipping past the space past the filesize
                            fname = &cmd[5];
                            if (TRUE)
                            {
                                fname[0]=0;
                                tmp = &fname[1];
                                fname = (STRPTR) strchr(&fname[1],' ');
                                if (fname)
                                {
                                    int x=0;
                                    fname[0]=0;
                                    size=myatol(tmp);
                                    if (TRUE)
                                    {
                                        APTR buffmem;
                                        fname++;

                                        // Null out the linefeed.
                                        while ( (fname[x]) && (fname[x] != '\n') )
                                            x++;
                                        fname[x]=0;
                                        // Try to get some buffer memory ...
                                        buffmem = (APTR) AllocMem(BUFFERSIZE,0);
                                        if (buffmem)
                                        {
                                            struct ASyncFile *ofh;
                                            STRPTR xname;

                                            // Open the destination file itself ...
                                            if (!force)
                                                xname = fname;
                                            else
                                                xname = forcename;
                                            if (!strcmp(xname,"*"))
                                                xname="starfile";
                                            ofh = OpenAsync(g,xname,MODE_WRITE,ASYNC_BUF_SIZE);
                                            if (ofh)
                                            {
                                                // We are supposed to ack them here -- admit that we did indeed receive the 'C' command.
                                                send(s,"\000",1,0);
                                                // Read it from the network to the file in BUFFERSIZE blocks.
                                                while (size)
                                                {
                                                    LONG this, got;
                                                    this = MIN(BUFFERSIZE,size);
                                                    got = recv(s,buffmem,this,0);
                                                    if (got < 0)
                                                        break;
                                                    WriteAsync(g,ofh,buffmem,got);
                                                    size -= got;
                                                }
                                                CloseAsync(g,ofh);
                                                SetProtection(xname,tmode);
                                                // If everything went okay, tell them so.
                                                GetResponse(g);     // eat their code ..
                                                if (!size)
                                                    send(s,"\000",1,0);
                                                else
                                                {
                                                    SendError(g,"\002rcp: Failed to write to destination file.\n");
                                                    cont = FALSE;
                                                }
                                            }
                                            else
                                            {
                                                SendError(g,"\002rcp: Can't create destination file.\n");
                                                cont = FALSE;
                                                break;
                                            }
                                            FreeMem(buffmem,BUFFERSIZE);
                                        }
                                        else
                                        {
                                            SendError(g,"\002rcp: Out of memory.\n");
                                            cont = FALSE;
                                            break;
                                        }
                                    }
                                }
                            }
                            break;
                        }

                    }
                }
            }
        }

        // Change back to old cd.
        newcd = CurrentDir(oldcd);
        UnLock(newcd);

    }
    else
    {
        SendError(g,errstring);
    }
    return(r);
}

// IsWild -- Decide if a path is wildcarded or not
//
// This function decides whether a path contains wildcards
// or not.  The problem with doing this is that we need to
// support both AmigaDOS '#' '?' characters as well as
// Unix '*' wildcards.  Thus, we use ParsePattern() to
// detect AmigaDOS wildcards, and a simple string search
// to find any '*'s.
//
// Entry:
//      x - ptr to string
//      g - ptr to global data area
//
// Exit:
//      TRUE  -- path is wildcarded
//      FALSE -- path is not wildcarded
//
BOOL IsWild(STRPTR x, struct GlobalData *g)
{
    STRPTR y;
    int z;
    BOOL result=FALSE;;
    z = strlen(x)*2+2;
    y = (STRPTR) AllocMem(z,0);
    if (y)
    {
        result = ParsePattern(x,y,z);
        FreeMem(y,z);
    }
    if (strchr(x,'*'))          // If there's a '*' in it, go Unixy and say "yes, it's wild!"
        result = TRUE;
    return(result);

}

// A variation on strncpy -- copy n characters,
// but make sure the destination gets null-terminated.
// The catch is that the 'length' value must be 1 less than
// the total possible space pointed to by 'to', in case of
// this situation.
void strdncpy(char *to, char *from, int length, struct GlobalData *g)
{
    strncpy(to,from,length);
    to[length]=0;
}

//
// MyReadArgs -- Do ReadARgs, but do some extra parsing.
// Unfortunately, Unix compatibility is important to AS225's users.
// There are places where ReadArgs and Unix are at odds.  Because of the
// fact that we need to provide some level of ReadArgs() compatibility,
// we do a bit of preprocessing before passing the command string to
// ReadArgs.  The '=' character is a specific problem.  We need this to
// be available, where ReadArgs uses it for aliasing.  because of this,
// We preparse all '='s out, convert them to something else, and then
// look for that something else in the main code instead of '='.  :'(

struct RDArgs *MyReadArgs(UBYTE *template, ULONG *table, struct RDArgs *ra, struct GlobalData *g)
{

    struct RDArgs *ret;
    BOOL once=FALSE;
    UBYTE *lptr;

    UBYTE *q;

    lptr = GetArgStr();
    strdncpy(g->RAIn,lptr,256,g);

    while (TRUE)
    {
        int y, z;
        y = strlen(g->RAIn);
        if (y == 1)
            break;
        for (z = 0; z < y; z++)
        {
            if (!(g->RAIn[z] == '?'))
            {
                if ( (g->RAIn[z] == ' ') ||
                     (g->RAIn[z] == '\n') )
                    continue;
                else
                    break;
            }
        }
        if (z != y)
            break;
        Write(Output(),PUBLICTEMPLATE,strlen(PUBLICTEMPLATE));
        if (!once)
        {
            once = TRUE;
            FGets(Input(),g->RAIn,255);
        }
        FGets(Input(),g->RAIn,255);

    }

    // Because ReadArgs has it's own meanings for the '=' character, Brian and
    // I decided to preparse the string before sending it to ReadArgs().
    // Yes -- we're aware that this nukes use of the '=' sign in normal
    // ReadArgs() behavior.  But crummy Unix compatibility is more
    // important to our users than Amiga consistency.  :'(
    while (q = (UBYTE *) strchr(g->RAIn,'='))
        *q='='+128;

    // okay, this version has the command name in it, too.  copy over that.

    ra->RDA_Buffer = (UBYTE *) g->RAToken;
    ra->RDA_BufSiz = 255;
    ra->RDA_Source.CS_Buffer = (UBYTE *) g->RAIn;
    ra->RDA_Source.CS_Length = 255;
    ra->RDA_Source.CS_CurChr = 0;
    ret = ReadArgs(template,table,ra);

    return(ret);
}

