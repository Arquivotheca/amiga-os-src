#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <netdb.h>

#include <stdlib.h>
#include <string.h>
/* make a standard IPAddress (ie, ULONG) out of a string of the
   form xxx.yyy.zzz.rrr
*/


/* protos for functions defined here. */
ULONG makeIPAddr(STRPTR);
STRPTR addrFromULong(ULONG);
STRPTR hostNameFromIPAddr(ULONG);

/* socket.library protos, Amiga-ized. */
ULONG inet_addr(STRPTR);
struct hostent *gethostbyaddr(UBYTE *, int, int);
STRPTR inet_ntoa(ULONG);


/* so caller need not open the library himself */
#pragma libcall mySockBase inet_addr 96 901
#pragma libcall mySockBase gethostbyaddr 90 10803
#pragma libcall mySockBase inet_ntoa B4 101


/* FOR TESTING

VOID main(int argc, UBYTE *argv[]) {
    ULONG foo;
    STRPTR bar;

    if (argc >= 2) {
        foo = makeIPAddr(argv[1]);
        Printf("makeIPAddr() returned %lx (%lu)\n", foo, foo);
        if (bar = addrFromULong(foo)) {
            Printf("addrFromULong() returned '%s'\n", bar);
            FreeVec(bar);
        }
    }
}
 ..... end test */


ULONG makeIPAddr(STRPTR addr_string) {
    ULONG result = 0UL;
    struct Library *mySockBase;

    if (mySockBase = OpenLibrary("socket.library",0)) {
        result = inet_addr(addr_string);
        CloseLibrary(mySockBase);
    }
    return(result);
}

/* The returned STRPTR *MUST* be FreeVec()'d by the caller! */
STRPTR addrFromULong(ULONG adul) {
    struct Library *mySockBase;
    STRPTR result=NULL, tmpStr;
    int len;

    if (mySockBase = OpenLibrary("socket.library",0)) {
        if (tmpStr = inet_ntoa(adul)) {
            len = strlen(tmpStr);
            if (result = AllocVec(len+1,MEMF_CLEAR)) {
                strcpy(result,tmpStr);
            }
        }
        CloseLibrary(mySockBase);
    }
    return(result);
}


/*  THIS FUNCTION IS UNDER CONSTRUCTION 
    The STRPTR returned from this function MUST be FreeVec()'d. 

STRPTR hostNameFromIPAddr(ULONG addr) {
    struct Library *mySockBase;
    STRPTR result=NULL;
    struct hostent *he;

    if (mySockBase = OpenLibrary("socket.library",0)) {
        if (he = gethostbyaddr(addr,4,AF_INET)) {
            
        }
        CloseLibrary(mySockBase);
    }
    return(result);
}
*/

