
#include <exec/memory.h>
#include <devices/timer.h>
#include <intuition/intuition.h>
#include <workbench/icon.h>
#include <stdio.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/icon_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include "clock.h"
#include "icon.h"
#include "clock.h"


/*****************************************************************************/


extern struct Library *SysBase;
extern struct Library *IconBase;
extern struct Library *UtilityBase;

extern struct Window  *wp;

extern UBYTE clockType;
extern UBYTE doSeconds;
extern UBYTE doDates;
extern UBYTE formatNum;


/*****************************************************************************/


static VOID RemoveTT(STRPTR *ttypes, STRPTR entry)
{
UWORD i;

    i = 0;
    while (ttypes[i])
    {
        if (Strnicmp(ttypes[i],entry,strlen(entry)) == 0)
        {
            FreeVec(ttypes[i]);

            while (ttypes[i])
            {
                ttypes[i] = ttypes[i+1];
                i++;
            }
            i = 0;
        }
        else
        {
            i++;
        }
    }
}


/*****************************************************************************/


static VOID AddTT(STRPTR *ttypes, STRPTR entry, STRPTR value)
{
UWORD i;
UWORD len;

    RemoveTT(ttypes,entry);

    i = 0;
    while (ttypes[i])
        i++;

    len = strlen(entry)+1;
    if (value)
        len += strlen(value)+1;

    if (ttypes[i] = AllocVec(len,MEMF_PUBLIC))
    {
        strcpy(ttypes[i],entry);
        if (value)
        {
            strcat(ttypes[i],"=");
            strcat(ttypes[i],value);
        }
    }
}


/*****************************************************************************/


static VOID AddNumTT(STRPTR *ttypes, STRPTR entry, LONG value)
{
char buffer[14];

    sprintf(buffer,"%ld",value);
    AddTT(ttypes,entry,buffer);
}


/*****************************************************************************/


static VOID AddBoolTT(STRPTR *ttypes, STRPTR entry, BOOL state)
{
    if (state)
        AddTT(ttypes,entry,NULL);
    else
        RemoveTT(ttypes,entry);
}


/*****************************************************************************/


VOID SaveClockIcon(VOID)
{
STRPTR            *ttypes;
STRPTR            *new;
UWORD              i;
UWORD              cnt;
struct DiskObject *diskObj;

    if (diskObj = GetDiskObject("PROGDIR:Clock"))
    {
        cnt = 0;
        if (ttypes = diskObj->do_ToolTypes)
        {
            while (ttypes[cnt])
                cnt++;
        }

        if (new = AllocVec((cnt+10)*4,MEMF_CLEAR))
        {
            i = 0;
            while (i < cnt)
            {
                if (new[i] = AllocVec(strlen(ttypes[i])+1,MEMF_PUBLIC))
                    strcpy(new[i],ttypes[i]);
                i++;
            }

            AddBoolTT(new,"DIGITAL", (clockType == DIGITAL));
            AddBoolTT(new,"SECONDS", doSeconds);
            AddBoolTT(new,"DATE",    doDates);

            AddNumTT(new,"LEFT",   wp->LeftEdge);
            AddNumTT(new,"TOP",    wp->TopEdge);
            AddNumTT(new,"WIDTH",  wp->Width);
            AddNumTT(new,"HEIGHT", wp->Height);
            AddNumTT(new,"FORMAT", formatNum);

            diskObj->do_ToolTypes = new;
            PutDiskObject("PROGDIR:Clock",diskObj);
            diskObj->do_ToolTypes = ttypes;

            while (cnt > 0)
                FreeVec(new[--cnt]);

            FreeVec(new);
        }

        FreeDiskObject(diskObj);
    }
}
