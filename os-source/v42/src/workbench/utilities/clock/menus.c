
#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <libraries/gadtools.h>

#include <clib/gadtools_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>

#include <pragmas/gadtools_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>

#include "clock.h"
#include "menus.h"


/*****************************************************************************/


extern struct LocaleInfo  li;
extern struct Library    *GadToolsBase;
extern struct Library    *IntuitionBase;
extern struct Library    *SysBase;
extern struct Window     *wp;
extern APTR               vi;
extern UBYTE              clockType;
extern UBYTE              doDates;
extern UBYTE              doSeconds;
extern UBYTE              doAlarm;
extern UBYTE              formatNum;
extern STRPTR             timeSamples[];


/*****************************************************************************/


struct ClockMenu
{
    UBYTE             cm_Type;        /* NM_XXX from gadtools.h */
    AppStringsID      cm_Label;
    enum ClockCommand cm_Cmd;
    UWORD	      cm_ItemFlags;
};


/*****************************************************************************/


/* WARNING: Also change CreateClockMenus() if you change the order of these */
struct ClockMenu __far cmenus[] =
{
    {NM_TITLE,  MSG_CLK_PROJECT_MENU,       cc_NOP,           0},
      {NM_ITEM, MSG_CLK_PROJECT_ANALOG,     cc_Analog,        CHECKIT | CHECKED},
      {NM_ITEM, MSG_CLK_PROJECT_DIGITAL,    cc_Digital,       CHECKIT},
      {NM_ITEM, MSG_NOTHING,                cc_NOP,           0},
      {NM_ITEM, MSG_CLK_PROJECT_QUIT,       cc_Quit,          0},

    {NM_TITLE,  MSG_CLK_SETTINGS_MENU,      cc_NOP,           0},
      {NM_ITEM, MSG_CLK_SETTINGS_DATE,      cc_Date,          CHECKIT | MENUTOGGLE},
      {NM_ITEM, MSG_CLK_SETTINGS_SECONDS,   cc_Seconds,       CHECKIT | MENUTOGGLE},
      {NM_ITEM, MSG_CLK_SETTINGS_DFORMAT,   cc_NOP,           0},
        {NM_SUB,MSG_NOTHING,                cc_DateFmt1,      CHECKIT},
        {NM_SUB,MSG_NOTHING,                cc_DateFmt2,      CHECKIT},
        {NM_SUB,MSG_NOTHING,                cc_DateFmt3,      CHECKIT},
        {NM_SUB,MSG_NOTHING,                cc_DateFmt4,      CHECKIT},
        {NM_SUB,MSG_NOTHING,                cc_DateFmt5,      CHECKIT},
        {NM_SUB,MSG_NOTHING,                cc_DateFmt6,      CHECKIT},
      {NM_ITEM, MSG_NOTHING,                cc_NOP,           0},
      {NM_ITEM, MSG_CLK_SETTINGS_ALARM,     cc_Alarm,         CHECKIT | MENUTOGGLE},
      {NM_ITEM, MSG_CLK_SETTINGS_SETALARM,  cc_SetAlarm,      0},
      {NM_ITEM, MSG_NOTHING,                cc_NOP,           0},
      {NM_ITEM, MSG_CLK_SETTINGS_SAVE,      cc_SaveSettings,  0},

    {NM_END,    MSG_NOTHING,                 cc_NOP,          0}
};


/*****************************************************************************/


struct Menu     *menus;
struct MenuItem *analogItem;
struct MenuItem *digitalItem;


/*****************************************************************************/


BOOL InstallClockMenus(VOID)
{
UWORD             i,j,sub;
struct NewMenu   *nm;
struct ClockMenu *cm;

    cm = cmenus;

    i = 0;
    while (cm[i++].cm_Type != NM_END) {}

    if (!(nm = AllocVec(sizeof(struct NewMenu)*i,MEMF_CLEAR|MEMF_PUBLIC)))
        return(NULL);

    j = 0;
    i = 0;
    sub = 0;
    while (cm[i].cm_Type != NM_END)
    {
        nm[j].nm_Type     = cm[i].cm_Type;
        nm[j].nm_Flags    = cm[i].cm_ItemFlags;
        nm[j].nm_UserData = (APTR)cm[i].cm_Cmd;

        if (cm[i].cm_Type == NM_TITLE)
        {
            nm[j].nm_Label = GetString(&li,cm[i].cm_Label);
        }
        else if (cm[i].cm_Type == NM_SUB)
        {
            nm[j].nm_MutualExclude = 0xffffffff - (1<<sub);
            sub++;
            if (!(nm[j].nm_Label = timeSamples[cm[i].cm_Cmd - cc_DateFmt1]))
                j--;
        }
        else if (cm[i].cm_Label == MSG_NOTHING)
        {
            nm[j].nm_Label = NM_BARLABEL;
        }
        else if (cm[i].cm_Type != NM_END)
        {
            nm[j].nm_CommKey = GetString(&li,cm[i].cm_Label);
            nm[j].nm_Label   = &nm[j].nm_CommKey[2];
            if (nm[j].nm_CommKey[0] == ' ')
            {
                nm[j].nm_CommKey = NULL;
            }
        }
        j++;
        i++;
    }
    nm[j].nm_Type = NM_END;

    if (menus = CreateMenusA(nm,NULL))
    {
        analogItem  = ItemAddress(menus,FULLMENUNUM(0,0,NOSUB));
        digitalItem = ItemAddress(menus,FULLMENUNUM(0,1,NOSUB));

        analogItem->MutualExclude = 2;
        digitalItem->MutualExclude = 1;

        if (LayoutMenus(menus,vi,GTMN_NewLookMenus,TRUE,
                                 TAG_DONE))
        {
            SetMenuStrip(wp,menus);
        }
        else
        {
            FreeMenus(menus);
	    menus = NULL;
	}
    }

    FreeVec(nm);

    return((BOOL)(menus != NULL));
}


/*****************************************************************************/


VOID RemoveClockMenus(VOID)
{
    ClearMenuStrip(wp);
    FreeMenus(menus);
}


/*****************************************************************************/


VOID AdjustClockMenus(VOID)
{
struct MenuItem *item;
UWORD            cnt;
struct MenuItem *secondsItem;
struct MenuItem *dateItem;
struct MenuItem *formatItem;
struct MenuItem *alarmItem;

    dateItem    = ItemAddress(menus,FULLMENUNUM(1,0,NOSUB));
    secondsItem = ItemAddress(menus,FULLMENUNUM(1,1,NOSUB));
    formatItem  = ItemAddress(menus,FULLMENUNUM(1,2,NOSUB));
    alarmItem   = ItemAddress(menus,FULLMENUNUM(1,4,NOSUB));

    ClearMenuStrip(wp);

    secondsItem->Flags &= ~(ITEMENABLED|CHECKED);
    formatItem->Flags  &= ~(ITEMENABLED|CHECKED);
    dateItem->Flags    &= ~CHECKED;

    if (clockType == ANALOG)
    {
        analogItem->Flags  |= CHECKED;
        digitalItem->Flags &= ~CHECKED;
        secondsItem->Flags |= ITEMENABLED;
    }
    else
    {
        analogItem->Flags  &= ~CHECKED;
        digitalItem->Flags |= CHECKED;

        if (timeSamples[0])
            formatItem->Flags |= ITEMENABLED;
    }

    if (doSeconds)
        secondsItem->Flags |= CHECKED;

    if (doDates)
        dateItem->Flags |= CHECKED;

    cnt  = 0;
    item = formatItem->SubItem;
    while (item)
    {
        item->Flags &= ~CHECKED;
        if (cnt == formatNum)
            item->Flags |= CHECKED;
        item = item->NextItem;
        cnt++;
    }

    alarmItem->Flags &= ~CHECKED;
    if (doAlarm)
        alarmItem->Flags |= CHECKED;

    ResetMenuStrip(wp,menus);
}
