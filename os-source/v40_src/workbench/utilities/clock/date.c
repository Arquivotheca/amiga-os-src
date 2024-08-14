
#include <exec/types.h>
#include <utility/date.h>
#include <utility/hooks.h>
#include <dos/datetime.h>

#include <clib/dos_protos.h>
#include <clib/locale_protos.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/locale_pragmas.h>

#include "texttable.h"
#include "date.h"


/*****************************************************************************/


extern struct Library    *DOSBase;
extern struct Library    *LocaleBase;
extern struct LocaleInfo  li;


/*****************************************************************************/


static VOID __asm PutCh(register __a0 struct Hook *hook,
                        register __a1 char c)
{
    *(char *)hook->h_Data = c;
    hook->h_Data = (APTR)((ULONG)hook->h_Data + 1);
}


/*****************************************************************************/


VOID GetDate(ULONG seconds, STRPTR format, STRPTR result)
{
struct DateTime dt;
struct Hook     hook;

    dt.dat_Stamp.ds_Days   = seconds / (60*60*24);
    dt.dat_Stamp.ds_Minute = (seconds / 60) % (60*24);
    dt.dat_Stamp.ds_Tick   = (seconds % 60) * 50;
    dt.dat_Format          = FORMAT_DOS;
    dt.dat_Flags           = 0;
    dt.dat_StrDay          = NULL;
    dt.dat_StrDate         = result;
    dt.dat_StrTime         = NULL;

    if (li.li_Locale)
    {
        hook.h_Entry = (APTR)PutCh;
        hook.h_Data  = result;
        FormatDate(li.li_Locale,format,&dt.dat_Stamp,&hook);
    }
    else
    {
        DateToStr(&dt);
    }
}


/*****************************************************************************/


VOID GetTime(ULONG seconds, STRPTR format, STRPTR result)
{
struct DateTime dt;
struct Hook     hook;

    dt.dat_Stamp.ds_Days   = seconds / (60*60*24);
    dt.dat_Stamp.ds_Minute = (seconds / 60) % (60*24);
    dt.dat_Stamp.ds_Tick   = (seconds % 60) * 50;
    dt.dat_Format          = FORMAT_DOS;
    dt.dat_Flags           = 0;
    dt.dat_StrDay          = NULL;
    dt.dat_StrDate         = NULL;
    dt.dat_StrTime         = result;

    if (li.li_Locale)
    {
        hook.h_Entry = (APTR)PutCh;
        hook.h_Data  = result;
        FormatDate(li.li_Locale,format,&dt.dat_Stamp,&hook);
    }
    else
    {
        DateToStr(&dt);
    }
}
