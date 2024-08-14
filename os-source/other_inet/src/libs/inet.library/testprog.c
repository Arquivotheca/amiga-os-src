
#include <stdio.h>
#include <exec/types.h>
#include <netinet/inetconfig.h>
#include <pragmas/inet_pragmas.h>

struct Library *InetBase;

main()
{

    InetBase = (struct Library *) OpenLibrary("inet.library",0L);
    if (InetBase)
    {
        ULONG tags[4]={INET_Gateway,TRUE,TAG_DONE,TRUE};
        ConfigureInet(&tags[0]);
        CloseLibrary(InetBase);
    }

}

