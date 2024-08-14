; /*
sc NIPCTest.c nostackcheck idir=include: lib lib:amiga.lib lib:envoy.lib lib:debug.lib
slink to NIPCTest from lib:c.o  NIPCTest.o lib lib:sc.lib lib:amiga.lib lib:envoy.lib lib:debug.lib
*/


#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include <clib/nipc_protos.h>

#include <stdlib.h>


#define ASM     __asm
#define REG(x)  register __ ## x


extern void KPrintF (char* format, ...);
__saveds static ULONG ASM hookEntry(REG(a0) struct Hook *h, REG(a2) VOID *o, REG(a1) VOID *msg);
__saveds static ULONG DecodeNIPCInquiry (struct Hook *h, VOID *o, VOID *msg);



struct Hook      h;
unsigned long    retValue;

struct Library*  NIPCBase;
extern struct Library *SysBase;


void main (void)
{
    long scantime = 2;

    if (!(NIPCBase = OpenLibrary ("nipc.library", 37)))
    {
        KPrintF ("Can't open nipc.library V 37");
        exit (5);
    };

    h.h_Entry       = (ULONG (*)()) hookEntry;
    h.h_SubEntry    = DecodeNIPCInquiry;
    h.h_Data        = NULL;

    retValue = TRUE;

    NIPCInquiry (&h, scantime, 100,
        //  QUERY_IPADDR,       0,
        //  QUERY_REALMS,       0,
        QUERY_HOSTNAME,     0,
        //  QUERY_SERVICE,      0,
        QUERY_ENTITY,       0,
        QUERY_OWNER,        0,
        //  QUERY_ATTNFLAGS,    0,
        //  QUERY_CHIPREVBITS,  0,
        //  QUERY_MAXFASTMEM,   0,
        //  QUERY_AVAILFASTMEM, 0,
        //  QUERY_MAXCHIPMEM,   0,
        //  QUERY_AVAILCHIPMEM, 0,
        //  QUERY_KICKVERSION,  0,
        //  QUERY_WBVERSION,    0,
        //  QUERY_NIPCVERSION,  0,

        TAG_END);

    //  Wait for our packets
    Delay (scantime * 50);

    retValue = FALSE;

    //  Let's wait some more
    Delay (50);

    CloseLibrary (NIPCBase);
}


__saveds static ULONG ASM hookEntry(REG(a0) struct Hook *h, REG(a2) VOID *o, REG(a1) VOID *msg)
{
    return ((*(ULONG (*)(struct Hook *,VOID *,VOID *))(h->h_SubEntry))(h, o, msg));
}



__saveds static ULONG DecodeNIPCInquiry (struct Hook *h, VOID *o, VOID *msg)
{
    struct TagItem* tstate;
    struct TagItem* tag;



    KPrintF ("DecodeNIPCInquiry() entered\n");

    //
    //  The 'Object' is the address of the NetProbe Task
    //

    if (o)
    {
        //
        //  I've got no use for the calling Task
        //
    }


    //
    //  Now, we fill in our real data
    //

    tstate = (struct TagItem*) msg;

    while (tag = NextTagItem (&tstate))
    {
        ULONG tidata = tag->ti_Data;

        switch (tag->ti_Tag)
        {
            case QUERY_IPADDR:
                KPrintF ("DecodeNIPCInquiry::QUERY_IPADDR: '%ld.%ld.%ld.%ld'\n", ((tidata >> 24) & 0xff), ((tidata >> 16) & 0xff), ((tidata >> 8) & 0xff), (tidata & 0xff));
                break;

            case QUERY_REALMS:
                KPrintF ("DecodeNIPCInquiry::QUERY_REALMS: '%s'\n", (char*) tidata);
                break;

            case QUERY_HOSTNAME:
                KPrintF ("DecodeNIPCInquiry::QUERY_HOSTNAME: '%s'\n", (char*) tidata);
                break;

            case QUERY_SERVICE:
                KPrintF ("DecodeNIPCInquiry::QUERY_SERVICE: '%s'\n", (char*) tidata);
                break;

            case QUERY_ENTITY:
                KPrintF ("DecodeNIPCInquiry::QUERY_ENTITY: '%s'\n", (char*) tidata);
                break;

            case QUERY_OWNER:
                KPrintF ("DecodeNIPCInquiry::QUERY_OWNER: '%s'\n", (char*) tidata);
                break;

            case QUERY_MACHDESC:
                KPrintF ("DecodeNIPCInquiry::QUERY_MACHDESC: '%08lx'\n", tidata);
                break;

            case QUERY_ATTNFLAGS:
                KPrintF ("DecodeNIPCInquiry::QUERY_ATTNFLAGS: '%08lx'\n", tidata);
                break;

            case QUERY_LIBVERSION:
                KPrintF ("DecodeNIPCInquiry::QUERY_LIBVERSION: '%08lx'\n", tidata);
                break;

            case QUERY_CHIPREVBITS:
                KPrintF ("DecodeNIPCInquiry::QUERY_CHIPREVBITS: '%08lx'\n", tidata);
                break;

            case QUERY_MAXFASTMEM:
                KPrintF ("DecodeNIPCInquiry::QUERY_MAXFASTMEM: '%8ld'\n", tidata);
                break;

            case QUERY_AVAILFASTMEM:
                KPrintF ("DecodeNIPCInquiry::QUERY_AVAILFASTMEM: '%8ld'\n", tidata);
                break;

            case QUERY_MAXCHIPMEM:
                KPrintF ("DecodeNIPCInquiry::QUERY_MAXCHIPMEM: '%8ld'\n", tidata);
                break;

            case QUERY_AVAILCHIPMEM:
                KPrintF ("DecodeNIPCInquiry::QUERY_AVAILCHIPMEM: '%8ld'\n", tidata);
                break;

            case QUERY_KICKVERSION:
                KPrintF ("DecodeNIPCInquiry::QUERY_KICKVERSION: '%ld.%ld'\n", ((tidata >> 16) & 0xff), (tidata & 0xff));
                break;

            case QUERY_WBVERSION:
                KPrintF ("DecodeNIPCInquiry::QUERY_WBVERSION: '%ld.%ld'\n", ((tidata >> 16) & 0xff), (tidata & 0xff));
                break;

            case QUERY_NIPCVERSION:
                KPrintF ("DecodeNIPCInquiry::QUERY_NIPCVERSION: '%ld.%ld'\n", ((tidata >> 16) & 0xff), (tidata & 0xff));
                break;

            default:
                break;
        }
    }


    KPrintF ("DecodeNIPCInquiry() left\n");

    return retValue;
}
