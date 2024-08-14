/* -----------------------------------------------------------------------
 * finger.c
 *
 * $Locker:  $
 *
 * $Id: finger.c,v 1.4 93/01/14 16:34:11 gregm Exp $
 *
 * $Revision: 1.4 $
 *
 * $Header: Hog:Other/inet/src/c/finger/RCS/finger.c,v 1.4 93/01/14 16:34:11 gregm Exp $
 *
 *------------------------------------------------------------------------
 */



#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <ss/socket.h>

#include "finger_rev.h"
static char *compiler = "LATTICE 5.10A" ;
static char *vers = VERSTAG ;
#define MAXSOCKS 10

/*
 * Prototypes to make Lattice happy
 */
int finger(VOID);
int netfinger(REGISTER char *name, REGISTER long large, REGISTER struct Library *SockBase, REGISTER struct DosLibrary *DOSBase);

/*
 * ReadArgs Template, option counts
 */
#define TEMPLATE "-L/S,REMOTEUSERS/M"
#define OPT_COUNT 2
#define OPT_USERNAME 1
#define OPT_L 0

/*
 * Main entry -- since we're trying to save space, we don't bother with main()
 * -- we do CLI only operation with no globals, and make no assumptions about
 * startup.
 */

int finger()
{
    struct DosLibrary *DOSBase;
    struct Library *SysBase;

    long opts[OPT_COUNT];
    char **x;
    struct RdArgs *rdargs;
    struct Library *SockBase;
    long errno;

    SysBase = (*(void **)4L);               /* Annoying.  lattice wants ExecBase */
    DOSBase = (struct DosLibrary *) OpenLibrary("dos.library",36L);
    if (DOSBase)
    {
        SockBase = (struct Library *) OpenLibrary("inet:libs/socket.library",1L);
        if (SockBase)
        {
            setup_sockets(MAXSOCKS, &errno);

            /* Read in the command line arguments through ReadArgs */
            memset ((char *) opts, 0, sizeof(opts));
            rdargs = (struct RdArgs *) ReadArgs(TEMPLATE,opts,NULL);
            if (rdargs)
            {
                if (!opts[OPT_USERNAME])
                    PutStr("finger: required argument missing\n");
                else
                {
                    for (x = (char **) opts[OPT_USERNAME]; *x; x++)
                        netfinger(*x,opts[OPT_L],SockBase,DOSBase);
                }
                FreeArgs(rdargs);
            }
            cleanup_sockets();
            CloseLibrary(SockBase);
        }
        CloseLibrary((struct Library *)DOSBase);
    }
    return(0);
}

int netfinger(REGISTER char *name, REGISTER long large, REGISTER struct Library *SockBase, REGISTER struct DosLibrary *DOSBase)

{
    char *host;
    struct hostent *hp;
    struct servent *sp;
    struct sockaddr_in sin;
    int s;
    char *strrchr();
    char c;
    register int lastc;

    /* If a null name altogether, skip this. */
    if (name == NULL)
        return (0);

    /* Search for the '@' char -- we only handle remote names
     * (amigas really have no true 'accounts')
     */
    host = strrchr(name, '@');
    if (host == NULL)
    {
        PutStr("finger: username must be remote\n");
        return (0);
    }
    *host++=0;                      /* bump past the '@' symbol, delimit name */
    if (!(*host))                  /* If name is now null, skip this name */
        return(0);

    hp = (struct hostent *) gethostbyname(host);
    if (hp == NULL)
    {
        struct hostent def;
        struct in_addr defaddr;
        char *alist[1];
        char namebuf[128];

        defaddr.s_addr = inet_addr(host);
        if (defaddr.s_addr == -1)
        {
            PutStr("unknown host: ");
            PutStr(host);
            PutStr("\n");
            return (1);
        }
        strcpy(namebuf, host);
        def.h_name = namebuf;
        def.h_addr_list = alist, def.h_addr = (char *)&defaddr;
        def.h_length = sizeof (struct in_addr);
        def.h_addrtype = AF_INET;
        def.h_aliases = 0;
        hp = &def;
    }
    sp = getservbyname("finger", "tcp");
    if (sp == 0)
    {
        PutStr("finger: unknown service\n");
        return (1);
    }
    sin.sin_family = hp->h_addrtype;
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    sin.sin_port = sp->s_port;
    s = socket(hp->h_addrtype, SOCK_STREAM, 0);
    if (s < 0)
    {
        PutStr(strerror(errno));
        return (1);
    }
    PutStr("[");
    PutStr(hp->h_name);
    PutStr("]\n");
    if (connect(s, (struct sockaddr *) &sin, sizeof (sin)) < 0)
    {
        PutStr(strerror(errno));
        return (1);
    }
    if (large) send(s, "/W ", 3, 0);
    send(s, name, strlen(name), 0);
    send(s, "\r\n", 2, 0);

    while(recv(s,&c,1,0)==1)
    {
        switch(c)
        {
            case 0210:
            case 0211:
            case 0212:
            case 0214:
                c -= 0200;
                break;
            case 0215:
                c = '\n';
                break;
        }
        lastc = c;
        FPutC(Output(),c);
    }
    if (lastc != '\n')
        FPutC(Output(),'\n');
    return (1);
}


