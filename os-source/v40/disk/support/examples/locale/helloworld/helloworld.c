;/*
;Sample use of CatComp
;
CatComp helloworld.cd CFILE helloworld_strings.h OBJFILE helloworld_strings.o
CatComp helloworld.cd helloworld.ct CATALOG Catalogs/Fran�ais/helloworld.catalog
SC RESOPT PARM=REGISTERS UCHAR CONSTLIB STREQ ANSI NOSTKCHK NOICONS OPT OPTPEEP helloworld.c
Slink LIB:c.o helloworld.o helloworld_strings.o TO HelloWorld LIB LIB:sc.lib SC SD
Quit
*/

/*
COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1992-1993
Commodore-Amiga, Inc.  All rights reserved.

DISCLAIMER: This software is provided "as is".  No representations or
warranties are made with respect to the accuracy, reliability, performance,
currentness, or operation of this software, and all use is at your own risk.
Neither commodore nor the authors assume any responsibility or liability
whatsoever with respect to your use of this software.
*/

#include <exec/types.h>
#include <libraries/locale.h>
#include <stdio.h>
#include <dos.h>

#include <clib/exec_protos.h>
#include <clib/locale_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/locale_pragmas.h>

#define CATCOMP_NUMBERS
#include "helloworld_strings.h"


extern struct Library *SysBase;
struct Library *LocaleBase;
struct Library *DOSBase;


STRPTR __asm GetString(register __a0 struct LocaleInfo *li,
                       register __d0 LONG stringNum);


VOID main(VOID)
{
struct LocaleInfo li;

    li.li_Catalog = NULL;
    if (LocaleBase = OpenLibrary("locale.library",38))
    {
        li.li_LocaleBase = LocaleBase;
        li.li_Catalog    = OpenCatalogA(NULL,"helloworld.catalog",NULL);
    }

    printf("%s\n",GetString(&li,MSG_HELLO));
    printf("%s\n",GetString(&li,MSG_BYE));

    if (LocaleBase)
    {
        CloseCatalog(li.li_Catalog);
        CloseLibrary(LocaleBase);
    }
}
