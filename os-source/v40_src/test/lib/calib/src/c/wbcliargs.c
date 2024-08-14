// check for existence of COMPLETED #ifdefs!!!!

/****** CALib.library/WBCliArgs ***********************************************

    NAME
        WBCliArgs -- Argument processing for WorkBench tooltypes and CLI

    VERSION
        1.00    04 Feb 1992 - Inception
        1.01    11 Feb 1992 - Fixed for case of '=' in ReadArgs(),
                              and 0x0nnn format for hex in tooltypes.
        1.02    10 Mar 1992 - Added WC_MToolTypes
        1.03    13 Mar 1992 - Added WC_Project
        1.04    03 Apr 1992 - Added floating-point template handling (/F)
        1.05    07 Apr 1992 - Fixed bugs with floating-point support,
                              changed it to /D
        1.06    21 Apr 1992 - Fixed bug in MToolTypes multiple - size
                              allocation too small

    AUTHOR
        Copyright © 1992 Mitchell/Ware Systems, Inc. All rights reserved.

    SYNOPSIS
        struct WBCli *WBCliArgs(struct TagItem *array)
        struct WBCli *WBCliArgsTags(ULONG tag1, (void *) data, ...)

    FUNCTION
        WBCliArgs processes arguments either from ToolTypes
        or ReadArgs(). It allows a uniformity and therefore
        transparency as how these arguments are to be processed.
        The same labels used for ReadArgs can also be used for
        Workbench ToolTypes without the programmer doing extra
        work. The ToolTypes, if exists, can be used as defaults
        for CLI ReadArgs if the appopriate information is given.

        WBCliArgs allows for additional types that ReadArgs() currently
        does not support. These templates are filtered from the string
        before passed to ReadArgs() by making the designation part of
        the template name. In this way, the user making a query about
        what the template is can see it without the forign type interfering
        with ReadArgs(), which may change how it behaves in the future.
        Therefore any new field type must have the specifier immediatly
        following the template, because WBCliArgs() will replace the
        slash of the forign specifier to an underscore before passage
        to ReadArgs(). So, for example, a template "FLOAT/D/A" would
        become translated to "FLOAT_D/A".

        The new forign specifiers are:
            /D  - Floating point (double, IEEE format)
            /H  - Hexadecimal number (converted to long)

        Behavior from Workbench is as expected.

    INPUTS
        array       - Array of TagItems
        tag1, ...   - TagItems
        data, ...   - Data required by Tag

    TAGITEMS
        WC_Template     - (char *) Template of the same format that
                          ReadArgs() wants. Required.

        WC_Args         - (long *) Array of arguments, corresponding to
                          the placement in the template, ReadArgs()-style.
                          Required.

        WC_Startup      - (struct WBStartup *) Startup message to process.
                          Will use first argument's ToolTypes for default
                          (if WC_DiskObject is not specified), and the rest
                          as parameters to the first /M template. Thus, an
                          array of fully qualified pathnames are created.

        WC_TrustStartup - (BOOL) Whether to trust WC_Startup as
                          valid. Default is TRUE. This is useful for
                          using "argv" and "argc" directly. Therefore,
                          this is an indication to WBCliArgs() that
                          the program started from Workbench or CLI.

                          The way one would normally use this with a normal
                          main() startup is as follows:
                            ...
                                WC_Startup, argv,
                                WC_TrustStartup, !argc,
                            ...

                          NOTE that if WC_Startup is not present or
                          NULL that WBCliArgs() will assume CLI startup.

        WC_DiskObject   - (struct DiskObject *) DiskObject to process
                          for ToolTypes. Overrides the object given in
                          WC_Startup. Only reads information from this;
                          you are still responsible for the cleanup.

        WC_DiskObjectName - Synonym for WC_DiskObject, name of the icon
                          to read for ToolTypes. WBCliArgs() in this
                          case acquires and unloads the disk object
                          on its own.

        WC_Help         - (char *) NULL-terminated help string to
                          use if user types second '?' from CLI.

        WC_MToolTypes   - (char *) If non-NULL, use the string as a
                          pattern match to the ToolTypes array to fill
                          in the /M multiple strings, when launched from
                          Workbench. Does not affect ReadArgs()/Cli
                          operation. Mutiple strings are then what was
                          matched in the ToolTypes rather than the
                          full pathnames of a multiple-select.

        WC_Project      - (BOOL) Use Project icon's ToolTypes, if present,
                          instead of Tool icon's. Drop back to Tool icon
                          if Project is not present. Default FALSE

        WC_GUI


    RESULT
        Returns a handle, a pointer to a PRIVATE structure if successful,
        or NULL. Handle must be passed to FreeWBCli() to release
        resources ONLY WHEN you are finished using the args array.
        Memory is dynamically allocated by WBCliArgs()
        to fill in some of the arguments in the args array, so do
        not FreeWBCli() until you are finished with using the
        args.

    NOTES
        The modifier '/N' has slightly different behavior for ToolTypes
        than for ReadArgs(). '/N' for ToolTypes can be either decimal,
        hexadecimal, or octal following the 'C' convention of denoting
        radix of numbers. Since ReadArgs() does not recognize this
        convention, ReadArgs() will still try to convert them as
        decimal.

    BUGS
        We have made every effort to make this call crash-proof. However,
        there are some low-memory or other error conditions that do
        not properly back out of operations yet. Those areas of
        concern are marked by three ampersands in the source code,
        and will be addressed shortly.

        The /H Template specifier is not fully supported yet.

    LIBRARIES
        Requires the following libraries to be opened:
            dos.library         (DOSBase)
            exec.library

    SEE ALSO
        FreeWBCli()
******************************************************************************/

// Includes

#include <exec/types.h>
#include <exec/memory.h>
#include <utility/tagitem.h>
#include <dos/rdargs.h>
#include <workbench/icon.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <intuition/intuition.h>

#include <ctype.h>
#include <string.h>

#include <proto/utility.h>
#include <proto/dos.h>
#include <proto/icon.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/wb.h>

#include "WBCliArgs.h"
#include "MWLib:h/protos.h"
#include "protos.h"

// Defines
#define APP_PAT     "=#?"   // For scanning ToolTypes using wildcards

#define IconBase        wbcli->Icon
#define UtilityBase     wbcli->Utility
#define IntuitionBase   wbcli->Intuition
#define GadToolsBase    wbcli->GadTools

#define ECS_LIB_VERSION 37

struct WBCli *WBCliArgs(struct TagItem *array)
{
    // Required elements
    ULONG *args = NULL;
    char *template = NULL;

    // Optional elements
    char *help = NULL;
    struct DiskObject *diskob = NULL, *ourdiskob = NULL;
    struct WBStartup *wbstartup = NULL;
    BOOL project;

    // Misc.
    struct WBCli *wbcli = NULL;
    BOOL failure = FALSE;

    // Check for required arguments
    if (wbcli = AllocVec(sizeof(*wbcli), MEMF_CLEAR))
    {
        InitLinkHead(&wbcli->gui);
        if (OpenAllWBCliLibraries(wbcli))
        {
            // Rake tags for params
            if (GetTagData(WC_TrustStartup, TRUE, array))
                wbstartup = GetTagData(WC_Startup, NULL, array);
            diskob = GetTagData(WC_DiskObject, NULL, array);
            template = GetTagData(WC_Template, NULL, array);
            help = GetTagData(WC_Help, NULL, array);
            args = GetTagData(WC_Args, NULL, array);
            project = GetTagData(WC_Project, FALSE, array);

            if (args && template)
            {
                if (wbstartup || diskob) // Reading disk object for ToolTypes
                {
                    if (!diskob) // If we don't have a disk object...
                    {
                        BPTR oldlock;

                        // use first or second argument for disk object
                        oldlock = CurrentDir(wbstartup->sm_ArgList[0].wa_Lock);
                        ourdiskob = diskob = GetDiskObjectNew(wbstartup->sm_ArgList[(!project || wbstartup->sm_NumArgs == 1) ? 0 : 1].wa_Name);
                        CurrentDir(oldlock);
                    }

                    // Parse Templates for processing
                    if (ParseWBCliTemplate(wbcli, template, args) != -1)
                    {
                        struct WBCGui *gui;

                        // Scan parsed Template for ToolTypes
                        for (gui = Container(NextLink(&wbcli->gui)); gui && !failure; gui = Container(NextLink(&gui->list)))
                        {
                            char *tt;

                            // Find the tooltype and process
                            if ((tt = FindToolType(diskob->do_ToolTypes, gui->name)) || (gui->alt_name && (tt = FindToolType(diskob->do_ToolTypes, gui->alt_name))))
                            {
                                if (gui->flags & WCG_Numeric) // numerical
                                {
                                    long *lg;

                                    if (lg = AllocRemember(&wbcli->key, sizeof(*lg), MEMF_CLEAR))
                                    {
                                        *lg = strtol(tt, NULL, 0); // stcd_l(tt, lg);
                                        *gui->parr = (ULONG) lg;
                                    }
                                    else failure = TRUE;
                                }

                                else if (gui->flags & WCG_Hex) // hexadecimal-numerical
                                {
                                    long *lg;

                                    if (lg = AllocRemember(&wbcli->key, sizeof(*lg), MEMF_CLEAR))
                                    {
                                        stch_l(tt, lg);
                                        *gui->parr = (ULONG) lg;
                                    }
                                    else failure = TRUE;
                                }

                                else if (gui->flags & WCG_Boolean) // switch
                                {
                                    *gui->parr = !stricmp("TRUE", tt)
                                                 || !stricmp("ON", tt)
                                                 || !stricmp("YES", tt)
                                                 || !stricmp("Y", tt);
                                }

                                else if (gui->flags & WCG_Toggle) // switch
                                {
                                    if (!stricmp("TRUE", tt)
                                        || !stricmp("ON", tt)
                                        || !stricmp("YES", tt)
                                        || !stricmp("Y", tt)
                                        || !stricmp("TOGGLE", tt))
                                    {
                                        *gui->parr = !*gui->parr;
                                    }
                                }

                                else if ((gui->flags & WCG_Multiple) && (gui->flags & WCG_String)) // Use the rest of the WBArgs for a list of parameters OR use the DiskObject's ToolType array
                                {
                                    char *dopat;

                                    if (dopat = GetTagData(WC_MToolTypes, NULL, array)) // Use matches in DiskObject's ToolType array
                                    {
                                        char *wstr, *wpat, **list;
                                        long patlen;
                                        // We will append an '=#?' to the end of pattern so that we can skip over the
                                        // remainder of the string. When we construct our array, we will use everything AFTER
                                        // the '='.

                                        if (wstr = AllocVec(strlen(dopat)+strlen(APP_PAT)+1, MEMF_CLEAR))
                                        {
                                            strcat(strcpy(wstr, dopat), APP_PAT);
                                            if (wpat = AllocVec(patlen = strlen(wstr) * 2 + 2, MEMF_CLEAR))
                                            {
                                                if (ParsePatternNoCase(wstr, wpat, patlen) != -1)
                                                {
                                                    long i, j, cnt;

                                                    // count number of matches
                                                    for (i = cnt = 0; diskob->do_ToolTypes[i]; ++i)   // count number of matches
                                                        if (MatchPatternNoCase(wpat, diskob->do_ToolTypes[i]))
                                                            ++cnt;

                                                    // now create array and extract tooltypes
                                                    if (list = AllocRemember(&wbcli->key, sizeof(*list) * (cnt+1), MEMF_CLEAR))
                                                    {
                                                        for (i = j = 0; diskob->do_ToolTypes[i]; ++i)   // count number of matches
                                                            if (MatchPatternNoCase(wpat, diskob->do_ToolTypes[i]))
                                                            {
                                                                char *s;

                                                                // now that we have a match, we will scan for the first '=' in
                                                                // string, and take the following as the string to use for
                                                                // the multiple string arrray.

                                                                for (s = diskob->do_ToolTypes[i]; *s && *s != '='; ++s)
                                                                    ;
                                                                if (*s) ++s; // one beyond '='
                                                                list[j++] = keyStrDup(&wbcli->key, s);
                                                            }

                                                        *gui->parr = list;
                                                    }
                                                    else failure = TRUE;
                                                }
                                                else failure = TRUE;
                                                FreeVec(wpat);
                                            }
                                            FreeVec(wstr);
                                        } // &&& Needs error checking
                                    }

                                    else // Use rest of WBArgs
                                    {
                                        if (wbstartup)
                                        {
                                            char **list;

                                            if (list = AllocRemember(&wbcli->key, sizeof(*list) * wbstartup->sm_NumArgs, MEMF_CLEAR))
                                            {
                                                long i;

                                                for (i = 1; i < wbstartup->sm_NumArgs; ++i)
                                                {
                                                    char buf[512];
                                                    BPTR oldlock, filelock;

                                                    oldlock = CurrentDir(wbstartup->sm_ArgList[i].wa_Lock);
                                                    filelock = Lock(wbstartup->sm_ArgList[i].wa_Name, ACCESS_READ);
                                                    NameFromLock(filelock, buf, sizeof(buf)); // Needs error checking &&&
                                                    UnLock(filelock);
                                                    CurrentDir(oldlock);
                                                    list[i-1] = keyStrDup(&wbcli->key, buf);
                                                }
                                                *gui->parr = list;
                                            }
                                            else failure = TRUE;
                                        }
                                        else // use string delimited by commas?
                                        {
                                        }
                                    }
                                }

                                else if (gui->flags & WCG_Float) // floating-point
                                {
                                    double *dg;

                                    if (dg = AllocRemember(&wbcli->key, sizeof(*dg), MEMF_CLEAR))
                                    {
                                        *dg = StrToDouble(tt, NULL);
                                        *gui->parr = (ULONG) dg;
                                    }
                                    else failure = TRUE;
                                }

                                else if (gui->flags & (WCG_String|WCG_Fixed)) // Just a plain string
                                {
                                    *gui->parr = keyStrDup(&wbcli->key, tt);
                                }
                            }
                            else if ((gui->flags & WCG_Required) && !GetTagData(WC_GUI, FALSE, array)) // Tooltype not found and was required!
                            {
                                notice("Missing Required ToolType %s%s%s%s%s%s/A",
                                                    gui->name,
                                                    (gui->flags & WCG_Multiple)  ? "/M" : "",
                                                    (gui->flags & WCG_Keyword)   ? "/K" : "",
                                                    (gui->flags & WCG_Float)     ? "/F" : "",
                                                    (gui->flags & WCG_Boolean)   ? "/S" : "",
                                                    (gui->flags & WCG_Numeric)   ? "/N" : (gui->flags & WCG_Hex) ? "/H" : "");
                                failure = TRUE;
                            }
                        }

#ifdef COMPLETED
                        if (GetTagData(WC_GUI, FALSE, array)) // create the GUI ?
                            failure = DoWBCGui(wbcli);
#endif
                    }
                }

                if (!wbstartup && !failure) // Do a ReadArgs if not launched from WB

                {
                    long arr_cnt;

                    if (help)
                    {
                        if (wbcli->rda = AllocRemember(&wbcli->key, sizeof(*wbcli->rda), MEMF_CLEAR))
                            wbcli->rda->RDA_ExtHelp = help;
                    }

                    if ((arr_cnt = ParseWBCliTemplate(wbcli, template, args)) != -1)
                    {
                        char *filtered_template;
                        ULONG *targs;

                        if (targs = AllocVec(sizeof(*targs) * (arr_cnt+1), MEMF_CLEAR))
                        {
                            if (filtered_template = FilterTemplateVec(template))
                            {
                                struct WBCGui *gui;

                                // transfer all Boolean values ONLY to new array
                                for (gui = Container(NextLink(&wbcli->gui)); gui; gui = Container(NextLink(&gui->list)))
                                    if (gui->flags & (WCG_Boolean|WCG_Toggle))
                                        targs[gui->index] = *gui->parr;

                                if (wbcli->rda = ReadArgs(filtered_template, targs, wbcli->rda))
                                {
                                    // decode any special strings, and copy back all booleans and anything else modified
                                    for (gui = Container(NextLink(&wbcli->gui)); gui; gui = Container(NextLink(&gui->list)))
                                    {
                                        if (gui->flags & (WCG_Boolean|WCG_Toggle))
                                        {
                                            *gui->parr = targs[gui->index];
                                        }

                                        else if (targs[gui->index])
                                        {
                                            if (gui->flags & WCG_Float)
                                            {
                                                if (gui->flags & WCG_Multiple)
                                                {
                                                    char **ads;
                                                    double **padd, *dd;
                                                    long nelem;

                                                    // size of original array?
                                                    for (nelem = 0, ads = (char **) targs[gui->index]; ads && ads[nelem]; ++nelem)
                                                        ;

                                                    // allocate new array, if needed
                                                    if (nelem)
                                                    {
                                                        if ((padd = AllocRemember(&wbcli->key, sizeof(*padd) * (nelem+1), MEMF_CLEAR))
                                                            && (dd = AllocRemember(&wbcli->key, sizeof(*dd) * nelem, MEMF_CLEAR)))
                                                        {
                                                            long i;

                                                            for (i = 0; i < nelem; ++i)
                                                            {
                                                                padd[i] = &dd[i];
                                                                dd[i] = StrToDouble(ads[i], NULL);
                                                            }

                                                            *gui->parr = (ULONG) padd;
                                                        }
                                                        else failure = TRUE;
                                                    }
                                                }

                                                else // single
                                                {
                                                    double *dd;

                                                    if (dd = AllocRemember(&wbcli->key, sizeof(*dd), MEMF_CLEAR))
                                                    {
                                                        *dd = StrToDouble((char *) targs[gui->index], NULL);
                                                        *gui->parr = (ULONG) dd;
                                                    }
                                                    else failure = TRUE;
                                                }
                                            }

                                            else if (gui->flags & WCG_Hex)
                                            {
                                                if (targs[gui->index])
                                                {
                                                    if (gui->flags & WCG_Multiple)
                                                    {
                                                    }
                                                    else
                                                    {
                                                    }
                                                }
                                            }

                                            else // direct transfer
                                            {
                                                *gui->parr = (ULONG) targs[gui->index];
                                            }
                                        }
                                    }
                                }
                                else failure = TRUE;

                                FreeVec(filtered_template);
                            }
                            else failure = TRUE;

                            FreeVec(targs);
                        }
                        else failure = TRUE;
                    }
                    else failure = TRUE;
                }
            }
            else
            {
                failure = TRUE;
                notice("WBCliArgs() PROG PANIC: args and/or template not given");
            }
        }
        else // Library failure
        {
            failure = TRUE;
            notice("WBCliArgs() PANIC: Library Failure");
        }
    }
    else // Programmer screwup / out-of-memory
    {
        failure = TRUE;
        notice("WBCliArgs() PANIC: %s", !args     ? "Missing Args Array" :
                                        !template ? "Missing Template"   :
                                        !wbcli    ? "Out of Memory"      : "System Corrupt");
    }

    if (failure) FreeWBCli(wbcli), wbcli = NULL;
    if (ourdiskob) FreeDiskObject(ourdiskob);
    return wbcli;
}

char *FilterTemplateVec(char *template) // filter out forign bits from template
{
    char *ft;

    if (ft = AllocVec(strlen(template)+1, MEMF_CLEAR))
    {
        char *s, *d;

        for (s = template, d = ft; *s; ++s, ++d)
            if (*s == '/' && (toupper(s[1]) == 'D' || toupper(s[1]) == 'H'))
                *d = '_';
            else
                *d = *s;
    }

    return ft;
}

BOOL OpenAllWBCliLibraries(struct WBCli *wbcli)
{
    BOOL ret = TRUE;

    if (IntuitionBase = OpenLibrary("intuition.library", ECS_LIB_VERSION))
    {
        if (IconBase = OpenLibrary("icon.library", ECS_LIB_VERSION))
        {
            if (UtilityBase = OpenLibrary("utility.library", ECS_LIB_VERSION))
            {
                if (GadToolsBase = OpenLibrary("gadtools.library", ECS_LIB_VERSION))
                {

                }
                else ret = FALSE;
            }
            else ret = FALSE;
        }
        else ret = FALSE;
    }
    else ret = FALSE;

    return ret;
}

void CloseAllWBCliLibraries(struct WBCli *wbcli)
{
    if (IntuitionBase)  CloseLibrary(IntuitionBase), IntuitionBase = NULL;
    if (IconBase)       CloseLibrary(IconBase)     , IconBase = NULL;
    if (UtilityBase)    CloseLibrary(UtilityBase)  , UtilityBase = NULL;
    if (GadToolsBase)   CloseLibrary(GadToolsBase) , GadToolsBase = NULL;
}

struct WBCli *WBCliArgsTags(ULONG Tag, ...)
{
    return WBCliArgs((struct TagItem *) &Tag);
}

struct WBCGui *AddGuiEntry(struct WBCli *wbcli) // allocate and add to tail of chain
{
    struct WBCGui *gui;

    if (gui = AllocRemember(&wbcli->key, sizeof(*gui), MEMF_CLEAR))
        AddLinkTail(&wbcli->gui, &gui->list, gui);

    return gui;
}

long ParseWBCliTemplate(struct WBCli *wbcli, char *template, ULONG *args) // create a WBGui list, return number of templates encountered, or -1 if error
{
    long index = 0;   // count of templates
    char *buf;
    BOOL failure = FALSE;

    if (buf = keyStrDup(&wbcli->key, template))
    {
        char *b, *s;
        BOOL done;

        for (b = s = buf, index = 0, done = FALSE; *b && !done && !failure; ++b)    // while there is still templates left
        {
            if (*b == ',' || !*(b+1))   // end of template detected?
            {
                char *r, *ss;
                ULONG _flag;
                struct WBCGui *gui;

                if (*b == ',') // adjust b if end of string
                    *b = NULL;
                else
                    ++b;

                // scan backwards for '/' statements, processing '/' statements
                for (r = b, _flag = NULL; r > s && *r != '='; --r)
                    if (*r == '/')
                    {
                        *r++ = NULL;
                        switch (*r)
                        {
                        case 'm': case 'M': _flag |= WCG_Multiple;  break;
                        case 'a': case 'A': _flag |= WCG_Required;  break;
                        case 'k': case 'K': _flag |= WCG_Keyword;   break;
                        case 's': case 'S': _flag |= WCG_Boolean;   break;
                        case 'f': case 'F': _flag |= WCG_Fixed;     break;
                        case 't': case 'T': _flag |= WCG_Toggle;    break;
                        case 'n': case 'N': _flag |= WCG_Numeric;   break;
                        case 'h': case 'H': _flag |= WCG_Hex;       break;    // not supported in ReadArgs()
                        case 'd': case 'D': _flag |= WCG_Float;     break;    // not supported in ReadArgs()
                        }
                    }

                if (!(_flag & WCG_FieldTypes)) // if no field type is defined, it is a string
                    _flag |= WCG_String;

                if (*r == '=')  // We use the parameter after the '=', if there is one
                {
                    ss = r+1;
                    *r = NULL;
                }
                else
                    ss = s;

                // now ss points to the string (ToolType), s points to alternate ToolType,
                //      and we have booleans for the slashes in _flag

                // Create a WBGui entry
                if (gui = AddGuiEntry(wbcli))
                {
                    gui->flags = _flag;
                    gui->parr = &args[index];
                    gui->index = index;
                    gui->name = ss;
                    gui->alt_name = (s != ss) ? s : NULL;
                }
                else
                    failure = TRUE;

                s = b+1;
                ++index;
            }
        }
    }
    else
        failure = TRUE;

    return (failure) ? -1 : index;
}

/****** CALib.library/FreeWBCli ***********************************************

    NAME
        FreeWBCli -- Free up the resources used by WBCliArgs()

    VERSION
        1.00    04 Feb 1992 - Inception

    AUTHOR
        Fred Mitchell, Products Assurance, Commodore-Amiga, Inc.

    SYNOPSIS
        void FreeWBCli(struct WBCli *wbcli)

    FUNCTION
        Releases all resources associated with WBCliArgs().

    INPUTS
        wbcli   - PRIVATE structure returned by WBCliArgs(). Can be NULL,
                  in which case it will be ignored.

    SEE ALSO
        WBCliArgs()

******************************************************************************/

void FreeWBCli(struct WBCli *wbcli)
{
    if (wbcli)
    {
        if (wbcli->rda) FreeArgs(wbcli->rda);
        FreeRemember(&wbcli->key, TRUE);
        CloseAllWBCliLibraries(wbcli);
        FreeVec(wbcli);
    }
}

/****i* CALib.library/DoWBCGui ************************************************

    NAME
        DoWBCGui -- Create a GUI for WBCli

    VERSION
        1.00    24 Mar 1992 - Inception

    AUTHOR
        Fred Mitchell, Products Assurance, Commodore-Amiga, Inc.

    SYNOPSIS
        BOOL DoWBCGui(struct WBCli *wbcli)

    FUNCTION
        Uses GadTools to create a GUI for the arguments. Will allow user
        to modify the initial settings.

        A list of elements is in wbcli->gui. This is scanned, and a GUI is
        created on that basis. For multiple (/M) templates, a ListView is
        created.

    INPUTS
        wbcli   - PRIVATE structure

    RESULT
        Returns TRUE if failure, or FALSE if ok. Failure status can be
        generated either by the user clicking CANCEL, or by a failure of
        the GUI/Window creation, out of memory, etc.

    SEE ALSO
        WBCliArgs()

******************************************************************************/

#ifdef COMPLETED
BOOL DoWBCGui(struct WBCli *wbcli)
{
    BOOL ret = FALSE;

    if (WBCGOpenWindow(wbcli))
    {
        if (WBCGLayoutGadgets(wbcli))
        {
            BOOL done;

            for (done = FALSE; !done; )
            {
                struct IntuiMessage *mess, m;

                while (mess = GT_GetIMsg(wbcli->gwin->UserPort))
                {
                    m = *mess;
                    GT_ReplyIMsg(mess);

                    switch (m.Class)
                    {
                    default:
                    }
                }

                if (!done)
                    WaitPort(wbcli->gwin->UserPort);
            }

        }

        WBCGCloseWindow(wbcli);
    }
    else ret = TRUE;

    return ret;
}

BOOL WBCGOpenWindow(struct WBCli *wbcli)
{
    BOOL ret = TRUE;

    if (wbcli->win = OpenWindowTags(NULL, TAG_DONE))

    return ret;
}
#endif
