
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/devices.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <exec/resident.h>
#include <dos/var.h>
#include <dos/dosextens.h>
#include <dos/datetime.h>
#include <internal/commands.h>
#include <utility/date.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include "version.h"
#include "versionbase.h"
#include "version_rev.h"


/*****************************************************************************/


#define TEMPLATE     "NAME,VERSION/N,REVISION/N,FILE/S,FULL/S,UNIT/N,INTERNAL/S,RES/S" VERSTAG
#define OPT_NAME     0
#define OPT_VERSION  1
#define OPT_REVISION 2
#define OPT_FILE     3
#define OPT_FULL     4
#define OPT_UNIT     5
#define OPT_INTERNAL 6
#define OPT_RES	     7
#define OPT_COUNT    8


/*****************************************************************************/


/* redefine this one to something more complicated once we are localized */
#define GetStr(strid) (strid)

#define MSG_VS_REQUIRES_V37 "This disk requires Kickstart version 2.04 or greater.\n"
#define MSG_VS_KICKSTART    "Kickstart"
#define MSG_VS_WORKBENCH    "Workbench"
#define MSG_VS_KICKREV      "%s %ld.%ld, "
/* #define MSG_VS_WBREV        "%s %ld.%ld" */
/** KLUDGE FOR STUPID LOCKHART **/
#define MSG_VS_WBREV        "%s"
/** **/
#define MSG_VS_NOVERSION    "Could not find version information for '%s'\n"


/*****************************************************************************/


struct VersionInfo *GetVersion(struct VersionLib *VersionBase, STRPTR name, ULONG tag1, ...);
VOID FreeVersion(struct VersionLib *VersionBase, struct VersionInfo *vi);
VOID PrintF(struct VersionLib *VersionBase, STRPTR fmt, ULONG args, ...);


/*****************************************************************************/


LONG main(VOID)
{
struct VersionLib   libNode;
struct VersionLib  *VersionBase;
struct RDargs      *rdargs;
LONG                opts[OPT_COUNT];
LONG                failureCode;
LONG                failureLevel;
BPTR                file;
struct VersionInfo *vi;
char                date[30];
struct DateTime     dt;
ULONG               location;
LONG                version, revision;
STRPTR              name;
char                buffer[20];

    VersionBase = &libNode;

    failureCode  = 0;
    failureLevel = RETURN_FAIL;

    SysBase = (*((struct ExecBase **) 4));

    DOSBase = (struct DosLibrary *)OpenLibrary("dos.library",0);
    if (DOSBase->dl_lib.lib_Version >= 37)
    {
        UtilityBase = OpenLibrary("utility.library",37);

    	memset(opts,0,sizeof(opts));
      	if (rdargs = ReadArgs(TEMPLATE,opts,NULL))
      	{
      	    location = 0xffffffff;  /* look everywhere */

      	    if (opts[OPT_FILE])
      	        location = VILOCF_PATH;  /* only look on disk */

            if (!opts[OPT_NAME])
            {
                if (vi = GetVersion(VersionBase,NULL,GV_Kickstart,TRUE,
                                                     TAG_DONE))
                {
                    sprintf(buffer,"%ld.%ld",vi->vi_Version,vi->vi_Revision);
                    SetVar("Kickstart",buffer,strlen(buffer),LV_VAR|GVF_LOCAL_ONLY);

                    PrintF(VersionBase,GetStr(MSG_VS_KICKREV),(ULONG)vi->vi_InternalName,vi->vi_Version,vi->vi_Revision);
                    FreeVersion(VersionBase,vi);
                }
            }

            if (vi = GetVersion(VersionBase,(STRPTR)opts[OPT_NAME],GV_Location,location,
                                                                   TAG_DONE))
            {
                if (vi->vi_Flags & VIF_FOUNDITEM)
                {
                    if (vi->vi_Flags & VIF_FOUNDINFO)
                    {
                        name = vi->vi_InternalName;
/*
                        if (!name || opts[OPT_FILE])
                            name = vi->vi_Path;
*/

                        if (!name)
                            name = vi->vi_InternalName;

                        version  = vi->vi_Version;
                        revision = vi->vi_Revision;

                        /* name hopefully includes version number */
                        PrintF(VersionBase,GetStr(MSG_VS_WBREV),(ULONG)name);

                        if (opts[OPT_FULL])
                        {
                            if (vi->vi_Flags & VIF_DATE)
                            {
                                dt.dat_Format  = 4;  /* FORMAT_DEF; isn't defined yet */
                                dt.dat_StrDay  = NULL;
                                dt.dat_StrDate = date;
                                dt.dat_StrTime = NULL;
                                dt.dat_Flags   = 0;
                                dt.dat_Stamp   = vi->vi_Date;
                                DateToStr(&dt);

                                PrintF(VersionBase," (%s)",(ULONG)date);
                            }

                            if (vi->vi_Comment)
                                PrintF(VersionBase,"\n%s",(ULONG)vi->vi_Comment);
                        }
                        PutStr("\n");

                        FreeVersion(VersionBase,vi);

                        if (!opts[OPT_NAME])
                        {
                            sprintf(buffer,"%ld.%ld",version,revision);
                            SetVar("Workbench",buffer,strlen(buffer),LV_VAR|GVF_LOCAL_ONLY);
                        }

                        failureLevel = RETURN_OK;
                        if (opts[OPT_VERSION])
                        {
                            if (version < *(LONG *)opts[OPT_VERSION])
                            {
                                failureLevel = RETURN_WARN;
                            }
                            else if (version == *(LONG *)opts[OPT_VERSION])
                            {
                                if (opts[OPT_REVISION])
                                {
                                    if (revision < *(LONG *)opts[OPT_REVISION])
                                    {
                                        failureLevel = RETURN_WARN;
                                    }
                                }
                            }
                        }
                        else if (opts[OPT_REVISION])
                        {
                            if (revision < *(LONG *)opts[OPT_REVISION])
                                failureLevel = RETURN_WARN;
                        }
                    }
                    else
                    {
                        PrintF(VersionBase,GetStr(MSG_VS_NOVERSION),opts[OPT_NAME]);
                    }
                }
                else
                {
                    failureCode = ERROR_OBJECT_NOT_FOUND;
                }
            }
            else
            {
                failureCode = ERROR_NO_FREE_STORE;
            }

            FreeArgs(rdargs);
        }
        else
        {
            failureCode = IoErr();
        }

        if (failureCode)
            PrintFault(failureCode,NULL);

        CloseLibrary(UtilityBase);
    }
    else
    {
        if (file = Open("*",MODE_NEWFILE))
        {
	    Write(file,GetStr(MSG_VS_REQUIRES_V37),strlen(GetStr(MSG_VS_REQUIRES_V37)));
	    Close(file);
	}
        OPENFAIL;
    }

    CloseLibrary(DOSBase);

    return(failureLevel);
}


/*****************************************************************************/


VOID PrintF(struct VersionLib *VersionBase, STRPTR fmt, ULONG args, ...)
{
    VPrintf(fmt,(LONG *)&args);
}


/*****************************************************************************/


struct Segment *LocateSegment(struct VersionLib *VersionBase, STRPTR name)
{
struct Segment *seg;

    seg = FindSegment(name,NULL,FALSE);
    if (!seg)
        seg = FindSegment(name,NULL,TRUE);

    return (seg);
}


/*****************************************************************************/


struct Resident *FindRomTag(BPTR segList)
{
struct Resident *tag;
UWORD           *seg;
ULONG            i;
ULONG           *ptr;

    while (segList)
    {
        ptr     = (ULONG *)((ULONG)segList << 2);
        seg     = (UWORD *)((ULONG)ptr);
        segList = *ptr;

        for (i=*(ptr-1)>>1; (i>0) ; i--)
        {
            if (*(seg++) == RTC_MATCHWORD)
            {
                tag = (struct Resident *)(--seg);
                if (tag->rt_MatchTag == tag)
                {
                    return(tag);
                }
            }
        }
    }

    return(NULL);
}


/*****************************************************************************/


APTR FindSegListVersion(BPTR segList)
{
ULONG  i;
UBYTE *address;
ULONG  length;
ULONG *ptr;

    while (segList)
    {
        ptr     = (ULONG *)((ULONG)segList << 2);
        address = (UBYTE *)ptr;
        segList = *ptr;
        length  = *(ptr - 1);

        for (i = 0;  i < length-4; i++)
        {
            if (address[i] == '$')
            {
                if (!strncmp(&address[i],"$VER:",5))
                {
                    return(&address[i]);
                }
            }
        }
    }

    return(NULL);
}


/*****************************************************************************/


#define SKIPSPACES     {while (versionStr[i] == ' ') i++;}
#define TERMINATOR(ch) (((ch == 0) || (ch == '\n') || (ch == '\r')) ? TRUE : FALSE)


struct VersionInfo *ParseVersionStr(struct VersionLib *VersionBase,
                                    struct VersionInfo *vi,
                                    STRPTR versionStr,
                                    BOOL skipVer, BOOL skipRev)
{
ULONG            i,k;
ULONG            startName;
ULONG            endName;
LONG             startComment;
STRPTR           name;
LONG             len;
LONG             conv;
LONG             day;
LONG             month;
LONG             year;
LONG             seconds;
struct ClockData cd;
char             date[20];
struct DateTime  dt;
char             ch;

    vi->vi_Flags |= VIF_FOUNDITEM;

    if (versionStr)
        vi->vi_Flags |= VIF_FOUNDINFO;

    if (skipVer)
        vi->vi_Flags |= (VIF_FOUNDINFO | VIF_VERSION);

    if (skipRev)
        vi->vi_Flags |= (VIF_FOUNDINFO | VIF_REVISION);

    if (!versionStr)
        versionStr = "";

    i = 0;

    if (strncmp(versionStr,"$VER:",5) == 0)
        i = 5;

    SKIPSPACES;

    startName = i;

    do
    {
        while ((versionStr[i] != ' ') && !TERMINATOR(versionStr[i]))
            i++;

        endName = i;

        SKIPSPACES;

        ch = versionStr[i];
    }
    while (ch && (ch != '\n') && (ch != '\r') && ((ch < '0') || (ch > '9')));

    startComment = i;
    len = StrToLong(&versionStr[i],&conv);
    if (len > 0)
    {
        if (!skipVer)
        {
            vi->vi_Version  = conv;
            vi->vi_Flags   |= VIF_VERSION;
        }

        i += len;
/** KLUDGE FOR STUPID LOCKHART **/
        endName = i;
/** **/
        startComment = i;
    }

    SKIPSPACES;
    startComment = i;

    if (versionStr[i] == '.')
    {
        i++;
        len = StrToLong(&versionStr[i],&conv);
        if (len > 0)
        {
            if (!skipRev)
            {
                vi->vi_Revision = conv;
                vi->vi_Flags   |= VIF_REVISION;
            }

            i += len;
            while (!TERMINATOR(versionStr[i]) && (versionStr[i] != '('))
                i++;

/** KLUDGE FOR STUPID LOCKHART **/
            endName = i;
/** **/
            startComment = i;
            if (versionStr[i] == '(')
            {
/** KLUDGE FOR STUPID LOCKHART **/
                endName--;
/** **/
                k = 1;
                while ((k < sizeof(date)) && (versionStr[i+k] != ')'))
                {
                    date[k-1] = versionStr[i+k];
                    k++;
                }
                date[k-1] = 0;

                i++;
                len = StrToLong(&versionStr[i],&day);
                if (len > 0)
                {
                    i += len;
                    if (versionStr[i] == '.')
                    {
                        i++;

                        len = StrToLong(&versionStr[i],&month);
                        if (len > 0)
                        {
                            i += len;

                            if (versionStr[i] == '.')
                            {
                                i++;
                                len = StrToLong(&versionStr[i],&year);
                                if (len > 0)
                                {
                                    i += len;
                                    if (versionStr[i] == ')')
                                        i++;

                                    SKIPSPACES;

                                    startComment = i;

                                    cd.year  = year+1900;
                                    cd.month = month;
                                    cd.mday  = day;
                                    cd.hour  = 0;
                                    cd.min   = 0;
                                    cd.sec   = 0;

                                    if (seconds = Date2Amiga(&cd))
                                    {
                                        vi->vi_Date.ds_Days   = seconds / (60*60*24);
                                        vi->vi_Date.ds_Minute = (seconds % (60*60*24)) / 60;
                                        vi->vi_Date.ds_Tick   = (seconds % 60) * 50;
                                        vi->vi_Flags |= VIF_DATE;
                                    }
                                }
                            }
                        }
                    }
                }

                if (!(vi->vi_Flags & VIF_DATE))
                {
                    memset(&dt,0,sizeof(dt));
                    dt.dat_Format  = FORMAT_DOS;
                    dt.dat_StrDate = date;
                    if (StrToDate(&dt))
                    {
                        vi->vi_Date   = dt.dat_Stamp;
                        vi->vi_Flags |= VIF_DATE;
                    }
                }
            }
        }
    }

    if (name = vi->vi_InternalName)
    {
/** KLUDGE FOR STUPID LOCKHART **/
        sprintf(vi->vi_KludgeBuf,"%s %ld.%ld",name,vi->vi_Version,vi->vi_Revision);
        name = vi->vi_KludgeBuf;
/** **/

        len = strlen(name);
    }
    else
    {
        name = &versionStr[startName];
        len = endName - startName;
    }

    if (!(vi->vi_InternalName = AllocVec(len+1,MEMF_ANY|MEMF_CLEAR)))
    {
        FreeVersion(VersionBase,vi);
        return(NULL);
    }

    vi->vi_Flags |= VIF_INTERNALNAME;
    CopyMem(name,vi->vi_InternalName,len);

    /* handle comment */

    while (!TERMINATOR(versionStr[i]))
        i++;

    if (startComment >= 0)
    {
        name = &versionStr[startComment];
        if (len = i - startComment)
        {
            if (!(vi->vi_Comment = AllocVec(len+1,MEMF_ANY|MEMF_CLEAR)))
            {
                FreeVersion(VersionBase,vi);
                return(NULL);
            }

            vi->vi_Flags |= VIF_COMMENT;
            CopyMem(name,vi->vi_Comment,len);
        }
    }

    return(vi);
}


/*****************************************************************************/


struct VersionInfo *ParseRomTag(struct VersionLib *VersionBase,
                                struct VersionInfo *vi,
                                struct Resident *romTag)
{
    vi->vi_InternalName = romTag->rt_Name;
    vi->vi_Version      = romTag->rt_Version;
    return(ParseVersionStr(VersionBase,vi,romTag->rt_IdString,TRUE,FALSE));
}


/*****************************************************************************/


#define BUFSIZE	4096


struct VersionInfo *FindFileVersion(struct VersionLib *VersionBase,
                                    struct VersionInfo *vi,
                                    STRPTR name)
{
UBYTE           *buffer;
BPTR             file;
BPTR             segList;
LONG             len;
LONG             i;
struct Resident *resident;
ULONG           *ptr;
STRPTR           vstring;
BOOLEAN          first;
BOOLEAN          found;
BOOLEAN          executable;

    found = FALSE;

    if (vi->vi_Path || (vi->vi_Path = AllocVec(256,MEMF_ANY)))
    {
        if (buffer = AllocVec(BUFSIZE+6,MEMF_ANY))
        {
            if (file = Open(name,MODE_OLDFILE))
            {
                vi->vi_Flags |= VIF_FOUNDITEM;

                if (!NameFromFH(file,vi->vi_Path,256))
                {
                    FreeVec(vi->vi_Path);
                    vi->vi_Path = NULL;
                }

                first      = TRUE;
                executable = FALSE;
                while (!found)
                {
                    len = Read(file,buffer,BUFSIZE);
                    if (len <= 0)
                        break;

                    if (first)
                    {
                        first = FALSE;
                        if (len > 20)
                        {
                            ptr = (ULONG *)buffer;
                            if (*ptr == 0x000003f3)
                            {
                                executable = TRUE;
                            }
                        }
                    }

                    for (i = 0; i < len; i++)
                    {
                        if (buffer[i] == '$')
                        {
                            if (len - i < 5)
                            {
                                Seek(file,-(len - i),OFFSET_CURRENT);
                                break;
                            }

                            if (strncmp(&buffer[i],"$VER:",5) == 0)
                            {
                                if (i > 0)
                                {
                                    Seek(file,-(len - i),OFFSET_CURRENT);
                                    break;
                                }
                                found   = TRUE;
                                vstring = &buffer[i];
                                break;
                            }
                        }
                    }

                    if (found)
                    {
                        vi = ParseVersionStr(VersionBase,vi,vstring,FALSE,FALSE);
                    }
                    else if (executable)
                    {
                        break;
                    }
                }
                Close(file);

                if (!found && executable)
                {
                    if (segList = LoadSeg(name))
                    {
                        if (vstring = FindSegListVersion(segList))
                        {
                            vi = ParseVersionStr(VersionBase,vi,vstring,FALSE,FALSE);
                        }
                        else if (resident = FindRomTag(segList))
                        {
                            vi = ParseRomTag(VersionBase,vi,resident);
                        }

                        UnLoadSeg(segList);
                    }
                }
            }
            FreeVec(buffer);

            if (vi && (!(vi->vi_Flags & VIF_FOUNDITEM)))
            {
                FreeVec(vi->vi_Path);
                vi->vi_Path = NULL;
            }

            return(vi);
        }
    }

    FreeVersion(VersionBase,vi);

    return(NULL);
}


/*****************************************************************************/


VOID FreeVersion(struct VersionLib *VersionBase,
                 struct VersionInfo *vi)
{
    FreeVec(vi->vi_Path);
    FreeVec(vi->vi_InternalName);
    FreeVec(vi->vi_Comment);
    FreeVec(vi);
}


/*****************************************************************************/


struct VersionInfo *GetVersionA(struct VersionLib *VersionBase,
                                STRPTR name, struct TagItem *tagList)
{
struct VersionInfo *vi;
struct Resident    *resident;
struct Library     *library;
struct Segment     *segment;
struct DosList     *dl;
STRPTR              libName;
ULONG               location;
ULONG               len;

    if (vi = AllocVec(sizeof(struct VersionInfo),MEMF_CLEAR))
    {
        vi->vi_Version        = -1;
        vi->vi_Revision       = -1;
        vi->vi_Date.ds_Days   = -1;
        vi->vi_Date.ds_Minute = -1;
        vi->vi_Date.ds_Tick   = -1;

        if (!name)
        {
            if (GetTagData(GV_Kickstart,FALSE,tagList) == TRUE)
            {
                vi->vi_Version  = SysBase->LibNode.lib_Version;
                vi->vi_Revision = SysBase->SoftVer;
                vi = ParseVersionStr(VersionBase,vi,GetStr(MSG_VS_KICKSTART),TRUE,TRUE);
            }
            else
            {
                vi->vi_Flags = VIF_FOUNDITEM;
                if (library = OpenLibrary("version.library",0))
                {
                    vi->vi_InternalName = "Workbench";
                    vi->vi_Version      = library->lib_Version;
                    vi->vi_Revision     = library->lib_Revision;
                    vi = ParseVersionStr(VersionBase,vi,library->lib_IdString,TRUE,TRUE);

                    CloseLibrary(library);
                }
            }
        }
        else
        {
            location = GetTagData(GV_Location,0xffffffff,tagList);

            /* check for a ROM resident module */
            if (location & VILOCF_EXECRESIDENT)
            {
                if (resident = FindResidentNC(VersionBase,name))
                {
                    vi->vi_Location = VILOCF_EXECRESIDENT;
                    vi              = ParseRomTag(VersionBase,vi,resident);
                    location        = 0; /* don't look anywhere else */
                }
            }

            /* check for it in the DOS resident list */
            if (location & VILOCF_DOSRESIDENT)
            {
                Forbid();

                if (segment = LocateSegment(VersionBase,name))
                {
                    /* internals are special since they share the shell number */
                    if ((segment->seg_UC == -2) || (segment->seg_UC == -999))
                    {
                        if (resident = FindResidentNC(VersionBase,"shell"))
                        {
                            vi->vi_Location = VILOCF_EXECRESIDENT;
                            vi              = ParseRomTag(VersionBase,vi,resident);
                        }
                    }
                    else
                    {
                        vi->vi_Location = VILOCF_DOSRESIDENT;
                        vi              = ParseVersionStr(VersionBase,vi,FindSegListVersion(segment->seg_Seg),FALSE,FALSE);
                    }

                    location = 0; /* don't look anywhere else */
                }

                Permit();
            }

            /* check for it as a file system */
            if (location & VILOCF_FILESYSTEM)
            {
                len = strlen(name);
                if (len && (name[len-1] == ':'))
                {
                    name[len-1] = 0;  /* !!! WARNING, ISN'T LEGAL, WHAT WOULD HAPPEN WITH A ROM CALLER?? !!! */

                    dl = FindDosEntry(LockDosList(LDF_READ|LDF_DEVICES),name,LDF_DEVICES);
                    UnLockDosList(LDF_READ|LDF_DEVICES);

                    if (dl)
                    {
                        vi->vi_Flags    = VIF_FOUNDITEM;
                        vi->vi_Location = VILOCF_FILESYSTEM;

                        if (dl->dol_misc.dol_handler.dol_Startup)
                        {
                            if (resident = FindRomTag(dl->dol_misc.dol_handler.dol_SegList))
                                vi = ParseRomTag(VersionBase,vi,resident);
                        }

                        location = 0; /* don't look anywhere else */
                    }
                    name[len-1] = ':';
                }
            }

            /* check for it in Exec's library list */
            if (location & VILOCF_LOADEDLIBRARY)
            {
                Forbid();

                if (library = (struct Library *)FindNameNC(VersionBase,&SysBase->LibList,FilePart(name)))
                {
                    vi->vi_Location     = VILOCF_LOADEDLIBRARY;
                    vi->vi_InternalName = library->lib_Node.ln_Name;
                    vi->vi_Version      = library->lib_Version;
                    vi->vi_Revision     = library->lib_Revision;
                    vi                  = ParseVersionStr(VersionBase,vi,library->lib_IdString,TRUE,TRUE);
                    location            = 0; /* don't look anywhere else */
                }

                Permit();
            }

            /* check for it in Exec's device list */
            if (location & VILOCF_LOADEDDEVICE)
            {
                Forbid();

                if (library = (struct Library *)FindNameNC(VersionBase,&SysBase->DeviceList,FilePart(name)))
                {
                    vi->vi_Location     = VILOCF_LOADEDDEVICE;
                    vi->vi_InternalName = library->lib_Node.ln_Name;
                    vi->vi_Version      = library->lib_Version;
                    vi->vi_Revision     = library->lib_Revision;
                    vi                  = ParseVersionStr(VersionBase,vi,library->lib_IdString,TRUE,TRUE);
                    location            = 0; /* don't look anywhere else */
                }

                Permit();
            }

            /* check for it with AmigaDOS path */
            if (location & VILOCF_PATH)
            {
                vi->vi_Location = VILOCF_PATH;
                vi              = FindFileVersion(VersionBase,vi,name);

                if (!vi || (vi->vi_Flags & VIF_FOUNDITEM))
                    location = 0; /* don't look anywhere else */
            }

            /* check for it in LIBS: */
            if (location & VILOCF_LIBSASSIGN)
            {
                if (libName = AllocVec(strlen(FilePart(name))+6,MEMF_ANY))
                {
                    strcpy(libName,"LIBS:");
                    strcat(libName,FilePart(name));

                    vi->vi_Location = VILOCF_LIBSASSIGN;
                    vi              = FindFileVersion(VersionBase,vi,libName);

                    FreeVec(libName);
                }
                else
                {
                    FreeVersion(VersionBase,vi);
                    vi = NULL;
                }

                if (!vi || (vi->vi_Flags & VIF_FOUNDITEM))
                    location = 0; /* don't look anywhere else */
            }

            /* check for it in DEVS: */
            if (location & VILOCF_DEVSASSIGN)
            {
                if (libName = AllocVec(strlen(FilePart(name))+6,MEMF_ANY))
                {
                    strcpy(libName,"DEVS:");
                    strcat(libName,FilePart(name));

                    vi->vi_Location = VILOCF_DEVSASSIGN;
                    vi              = FindFileVersion(VersionBase,vi,libName);

                    FreeVec(libName);
                }
                else
                {
                    FreeVersion(VersionBase,vi);
                    vi = NULL;
                }
            }

            if (vi && (!(vi->vi_Flags & VIF_FOUNDITEM)))
                vi->vi_Location = 0;
        }
    }

    return(vi);
}


/*****************************************************************************/


struct VersionInfo *GetVersion(struct VersionLib *VersionBase,
                               STRPTR name, ULONG tag1, ...)
{
    return(GetVersionA(VersionBase,name,(struct TagItem *)&tag1));
}
