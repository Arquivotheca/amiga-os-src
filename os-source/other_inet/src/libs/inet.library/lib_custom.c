/*
 * Library startup & init
 */

#include <exec/types.h>
#include <exec/alerts.h>
#include <dos.h>
#include <sys/types.h>
#include <netinet/inet.h>
#include <nlist.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include "lib_custom.h"
#include "proto.h"

struct ExecBase *SysBase = NULL;
struct InetNode *InetBase = NULL;
extern struct Library *UtilityBase;

#define NLSIZE  32
static struct nlist lib_nl[NLSIZE];


void
enter_nlist(name, value)
        char    *name;
        void    *value;
{
        register struct nlist *nl = lib_nl;
        register int i;

        for(i = 0; i < NLSIZE; i++, nl++){
                if(!nl->n_name){
                        nl->n_name = name;
                        nl->n_value = value;
                        nl->n_type = 1;
                        break;
                }
        }
}


BOOL __saveds __asm CustomLibOpen(register __a6 struct InetNode *lib)
{
        static short imp_softc, nfile, file, nimp;

        SysBase = (struct ExecBase *)(*((ULONG *)4));
        InetBase = lib;

        if(!(DOSBase = (void *)OpenLibrary("dos.library", 0L))){
                return (BOOL)0;
        }
        if (!(UtilityBase = (struct Library *)OpenLibrary("utility.library",37L)))
        {
            CloseLibrary(DOSBase);
            return ((BOOL) 0);
        }
        lib->names = lib_nl;
        lib->nlsize = NLSIZE;

        s2attach();

        mbinit();
        domaininit();
        startINET();

        loattach();

//        aeattach();
//        aaattach();
//        slattach();


        ifinit();
        raw_init();
        arp_nlist_init();
        route_nlist_init();
        icmp_nlist_init();

        enter_nlist("_imp_softc", &imp_softc);
        enter_nlist("_nfile", &nfile);
        enter_nlist("_file", &file);
        enter_nlist("_nimp", &nimp);
        return (BOOL)1;
}


void __saveds __asm CustomLibExpunge(register __a6 struct InetNode *lib)
{
    if (DOSBase)
        CloseLibrary((struct Library *)DOSBase);

    if (UtilityBase)
        CloseLibrary((struct Library *)UtilityBase);
}


