
#include <exec/types.h>
#include <dos/datetime.h>
#include <utility/hooks.h>
#include <string.h>
#include <stdlib.h>

#include <clib/locale_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/locale_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include "localebase.h"


/*****************************************************************************/


#define FORMAT_DEF 4

#define DOS_FORMAT "%d-%b-%y"
#define INT_FORMAT "%y-%b-%d"
#define USA_FORMAT "%m-%d-%y"
#define CDN_FORMAT "%d-%m-%y"


/*****************************************************************************/


static VOID ASM PutCh(REG(a0) struct Hook *hook,
                      REG(a2) APTR loc,
                      REG(a1) char c)
{
    *(char *)hook->h_Data = c;
    hook->h_Data = (APTR)((ULONG)hook->h_Data + 1);
}


/*****************************************************************************/


static ULONG ASM GetCh(REG(a0) struct Hook *hook)
{
char *ch;

    ch = hook->h_Data;
    hook->h_Data = (APTR)((ULONG)ch + 1);
    return((ULONG)*ch);
}


/*****************************************************************************/


static STRPTR GetDateFormat(struct ExtLocale *locale, UBYTE style)
{
STRPTR format;

    switch (style)
    {
        case FORMAT_USA: format = USA_FORMAT;
                         break;

        case FORMAT_INT: format = INT_FORMAT;
                         break;

        case FORMAT_CDN: format = CDN_FORMAT;
                         break;

        case FORMAT_DEF: format = locale->el_Locale.loc_ShortDateFormat;
                         break;

        default        : format = DOS_FORMAT;
                         break;
    }

    return(format);
}


/*****************************************************************************/


BOOL ASM DateToStrPatch(REG(d1) struct DateTime *dt,
                        REG(a6) struct LocaleLib *LocaleBase)
{
struct ExtLocale *locale;
struct Hook       hook;
STRPTR            format;
BOOL              result;
UWORD             len,mlen;
STRPTR            subst;
LONG              strNum;
struct DateStamp  now;
struct ClockData  cd;

    locale = (struct ExtLocale *)OpenLocale(NULL);
    result = TRUE;

    hook.h_Entry = (HOOKFUNC)PutCh;
    if (hook.h_Data = dt->dat_StrDate)
    {
        subst = NULL;
        if (dt->dat_Flags & DTF_SUBST)
        {
            DateStamp(&now);
            strNum = -1;

            if (now.ds_Days == dt->dat_Stamp.ds_Days + 1)
            {
                strNum = YESTERDAYSTR;
            }
            else if (now.ds_Days == dt->dat_Stamp.ds_Days)
            {
                strNum = TODAYSTR;
            }
            else if (now.ds_Days == dt->dat_Stamp.ds_Days - 1)
            {
                strNum = TOMORROWSTR;
            }
            else if (now.ds_Days < dt->dat_Stamp.ds_Days)
            {
                strNum = FUTURESTR;
            }
            else if (dt->dat_Stamp.ds_Days + 7 >= now.ds_Days)
            {
                Amiga2Date(dt->dat_Stamp.ds_Days*24*60*60,&cd);
                strNum = cd.wday + DAY_1;
            }

            if (strNum >= 0)
                subst = GetLocaleStr(locale,strNum);
        }

        if (subst)
        {
            strcpy(dt->dat_StrDate,subst);
        }
        else
        {
            FormatDate(locale,GetDateFormat(locale,dt->dat_Format),&dt->dat_Stamp,&hook);
        }

        if (dt->dat_Format == FORMAT_DOS)
        {
            len = strlen(dt->dat_StrDate);
            mlen = locale->el_MaxDateLen;

            while ((len < mlen) && (len < LEN_DATSTRING-1))
                dt->dat_StrDate[len++] = ' ';
            dt->dat_StrDate[len] = 0;
        }
    }

    if (hook.h_Data = dt->dat_StrTime)
    {
        if (dt->dat_Format == FORMAT_DEF)
            format = locale->el_Locale.loc_ShortTimeFormat;
        else
            format = "%H:%M:%S";

        FormatDate(locale,format,&dt->dat_Stamp,&hook);
    }

    if (hook.h_Data = dt->dat_StrDay)
    {
        FormatDate(locale,"%A",&dt->dat_Stamp,&hook);
    }

    CloseLocale((struct Locale *)locale);

    return(result);
}


/*****************************************************************************/


BOOL ASM StrToDatePatch(REG(d1) struct DateTime *dt,
                        REG(a6) struct LocaleLib *LocaleBase)
{
struct Hook       hook;
struct DateStamp  date;
struct ExtLocale *locale;
struct ClockData  cd;
STRPTR            format;
BOOLEAN           found;
WORD              i;
LONG              diff;

    locale = (struct ExtLocale *)OpenLocale(NULL);

    hook.h_Entry = (HOOKFUNC)GetCh;

    if (hook.h_Data = dt->dat_StrTime)
    {
        if (dt->dat_Format == FORMAT_DEF)
            format = locale->el_Locale.loc_TimeFormat;
        else
            format = "%H:%M:%S";

        if (!ParseDate(locale,&date,format,&hook))
        {
            hook.h_Data = dt->dat_StrTime;
            if (!ParseDate(locale,&date,"%H:%M",&hook))
            {
                return(FALSE);
            }
        }

        dt->dat_Stamp.ds_Minute = date.ds_Minute;
        dt->dat_Stamp.ds_Tick   = date.ds_Tick;
    }

    if (hook.h_Data = dt->dat_StrDate)
    {
        DateStamp(&date);
        found = FALSE;

        for (i = DAY_1; i <= ABDAY_7; i++)
        {
            if (Stricmp(dt->dat_StrDate,GetLocaleStr(locale,i)) == 0)
            {
                Amiga2Date(dt->dat_Stamp.ds_Days*24*60*60,&cd);
                i = (i-DAY_1) % 7;

                if (DTF_FUTURE & dt->dat_Flags)
                {
                    if (cd.wday == i)
                        diff = 7;
                    else
                        diff = abs(i-(WORD)cd.wday+7) % 7;
                }
                else
                {
                    if (cd.wday == i)
                        diff = -7;
                    else
                        diff = - ((WORD)abs((WORD)cd.wday-i+7) % 7);
                }
                date.ds_Days += diff;

                found = TRUE;
                break;
            }
        }

        if (Stricmp(dt->dat_StrDate,GetLocaleStr(locale,YESTERDAYSTR)) == 0)
        {
            date.ds_Days--;
        }
        else if (Stricmp(dt->dat_StrDate,GetLocaleStr(locale,TODAYSTR)) == 0)
        {
            /* do nothing */
        }
        else if (Stricmp(dt->dat_StrDate,GetLocaleStr(locale,TOMORROWSTR)) == 0)
        {
            date.ds_Days++;
        }
        else if (Stricmp(dt->dat_StrDate,GetLocaleStr(locale,FUTURESTR)) == 0)
        {
            date.ds_Days += 2;
        }
        else if (!found)
        {
            format = GetDateFormat(locale,dt->dat_Format);

            if (!ParseDate(locale,&date,format,&hook))
                return(FALSE);
        }
        dt->dat_Stamp.ds_Days = date.ds_Days;
    }

    CloseLocale(locale);

    return(TRUE);
}
