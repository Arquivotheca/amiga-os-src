/****** Librarian.module ************************************************************************
    Librarian -- Opens and Closes Resident Libraries

    Mitchell/Ware           Version 3.02            3/19/88

    Three calls:

        SetLibraryVersion(ver)	Sets version number to pass to OpenLibrary.

        Librarian (string)      Opens specified libraries, returns false if
                                one or more failed. Attempts to open all
                                libraries specified, so that user can be
                                aprised of all the missing libraries
                                in one execute attempt.

        CloseLibrarian (string) close specified libraries, or all if NULL.
                                no errors. WARNING: CloseLibrarian(NULL)
                                must be the LAST thing called just before
                                a program exit, or CLOSE attention must be
                                paid to calls made after the close, making
                                sure the calls aren't library dependent.

    Codes are as follows:

        asl.library          a          math.library                 0
        battclock.library    b          mathieeedoubbas.library      1
        battmem.library      B          mathieeedoubtrans.library    2
        clist.library        C          mathieeesingbas.library      3
        console.device       c          mathieeesingtrans.library    4
                                        mathtrans.library            5
        cx.library           X
        dos.library          D          misc.library                 m
        diskfont.library     d          potgo.library                p
        expansion.library    e          ramdrive.device              ^
        gadtools.library     G          rexxsyslib.library           x
                                        sys.library                  |
        graphics.library     g
        iffparse.library     I          timer.library                T
        icon.library         O          translator.library           t
        input.library        #          utility.library              u
        intuition.library    i          workbench.library            w

        keymap.library       k
        layers.library       l

*******************************************************************************/

#include <exec/types.h>
#include <exec/io.h>

struct Lib {
    long *base; /* pointer to base pointer */
    char *name; /* library/device name */
    char k;     /* key letter */
    char dev;   /* 1 = device, 0 = library */
    struct IORequest *io;   /* for Devices */
    BOOL DidWeOpenIt;   /* TRUE if we did */
    };

/* External refs to Base.c
*/
extern long AslBase                ;
extern long BattClockBase          ;
extern long BattMemBase            ;
extern long ClistBase              ;
extern long ConsoleDevice          ;
extern long CxBase                 ;
extern long DOSBase                ;
extern long DiskfontBase           ;
extern long ExpansionBase          ;
extern long GadToolsBase           ;
extern long GfxBase                ;
extern long IFFParseBase           ;
extern long IconBase               ;
extern long InputBase              ;
extern long IntuitionBase          ;
extern long KeymapBase             ;
extern long LayersBase             ;
extern long MathBase               ;
extern long MathIeeeDoubBasBase    ;
extern long MathIeeeDoubTransBase  ;
extern long MathIeeeSingBasBase    ;
extern long MathIeeeSingTransBase  ;
extern long MathTransBase          ;
extern long MiscBase               ;
extern long PotgoBase              ;
extern long RamdriveDevice         ;
extern long RexxSysBase            ;
extern long SysBase                ;
extern long TimerBase              ;
extern long TranslatorBase         ;
extern long UtilityBase            ;
extern long WorkbenchBase          ;

static struct Lib LibList[] = {
    {   &AslBase                , "asl.library",                'a', 0  },
    {   &BattClockBase          , "battclock.library",          'b', 0  },
    {   &BattMemBase            , "battmem.library",            'B', 0  },
    {   &ClistBase              , "clist.library",              'C', 0  },
    {   &ConsoleDevice          , "console.device",             'c', 1  },
    {   &CxBase                 , "cx.library",                 'X', 0  },
    {   &DOSBase                , "dos.library",                'D', 0  },
    {   &DiskfontBase           , "diskfont.library",           'd', 0  },
    {   &ExpansionBase          , "expansion.library",          'e', 0  },
    {   &GadToolsBase           , "gadtools.library",           'G', 0  },
    {   &GfxBase                , "graphics.library",           'g', 0  },
    {   &IFFParseBase           , "iffparse.library",           'I', 0  },
    {   &IconBase               , "icon.library",               'O', 0  },
    {   &InputBase              , "input.library",              '#', 0  },
    {   &IntuitionBase          , "intuition.library",          'i', 0  },
    {   &KeymapBase             , "keymap.library",             'k', 0  },
    {   &LayersBase             , "layers.library",             'l', 0  },
    {   &MathBase               , "math.library",               '0', 0  },
    {   &MathIeeeDoubBasBase    , "mathieeedoubbas.library",    '1', 0  },
    {   &MathIeeeDoubTransBase  , "mathieeedoubtrans.library",  '2', 0  },
    {   &MathIeeeSingBasBase    , "mathieeesingbas.library",    '3', 0  },
    {   &MathIeeeSingTransBase  , "mathieeesingtrans.library",  '4', 0  },
    {   &MathTransBase          , "mathtrans.library",          '5', 0  },
    {   &MiscBase               , "misc.library",               'm', 0  },
    {   &PotgoBase              , "potgo.library",              'p', 0  },
    {   &RamdriveDevice         , "ramdrive.device",            '^', 1  },
    {   &RexxSysBase            , "rexxsyslib.library",         'x', 0  },
    {   &SysBase                , "sys.library",                '|', 0  },
    {   &TimerBase              , "timer.library",              'T', 0  },
    {   &TranslatorBase         , "translator.library",         't', 0  },
    {   &UtilityBase            , "utility.library",            'u', 0  },
    {   &WorkbenchBase          , "workbench.library",          'w', 0  },
    {0}};

static ULONG version = 34L;	/* default version number */

void SetLibraryVersion(ver)
{
	version = ver;
}

BOOL Librarian (s)      /* you MUST check the return status. FALSE is failed. */
UBYTE *s;
{
    UBYTE c;
    struct Lib *lib;
    BOOL ret = TRUE;

    if (s)
    {
        while (c = *s++)    /* while there is string left */
            for (lib = LibList; lib->base; ++lib)
                if (c == lib->k)
                {
                    if (!*lib->base) /* if not already open */
                    {
                        if (lib->dev)
                        {
                            if ((lib->io = malloc(sizeof(*lib->io))) && !OpenDevice(lib->name, 0, lib->io, NULL))
                                *lib->base = lib->io->io_Device;
                            else
                            {
                                screwup2("Unable to open device:", lib->name);
                                ret = FALSE;
                            }
                        }
                        else if (!(*lib->base = OpenLibrary(lib->name, version)))
                        {
                            screwup2("?Unable to open library:", lib->name);
                            ret = FALSE;
                        }
                        lib->DidWeOpenIt = ret;
                    }
                    break;
                }
     }
     else
        ret = FALSE;

    return ret;
}

void _CloseLib(struct Lib *lib)
{
    if (*lib->base && lib->DidWeOpenIt)
    {
        if (lib->dev)
            CloseDevice(lib->io), free(lib->io);

        else
            CloseLibrary(*lib->base);

        *lib->base = lib->io = NULL;
        lib->DidWeOpenIt = FALSE;
    }
}

void CloseLibrarian (s)  /* if s is NULL, close all! Else close only selected */
UBYTE *s;
{
    UBYTE c;
    struct Lib *lib;

    if (s)
    {
        while (c = *s++)        /* while there is string left */
            for (lib = LibList; lib->base; ++lib)
	            if (c == lib->k)
	                _CloseLib(lib);
    }
    else /* close everything */
        for (lib = LibList; lib->base; ++lib)
            _CloseLib(lib);
}
