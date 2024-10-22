
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/memory.h>
#include <libraries/lowlevel.h>
#include <libraries/nonvolatile.h>
#include <graphics/gfx.h>
#include <graphics/sprite.h>
#include <graphics/videocontrol.h>
#include <graphics/gfxmacros.h>
#include <string.h>
#include <stdio.h>

#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/graphics_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>

#include "nrameditor.h"
#include "utils.h"
#include "cduibase.h"

#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>


/*****************************************************************************/


/* things we can do */
#define CMD_NOP    0
#define CMD_UP     1
#define CMD_DOWN   2
#define CMD_TOGGLE 3
#define CMD_EXIT   4


/*****************************************************************************/


/* info on nram bitmap */
#define NRAM_X_OFFSET           0
#define NRAM_Y_OFFSET           0
#define NRAM_WIDTH              249
#define NRAM_HEIGHT             135

#define KEY_LOCKED_X_OFFSET   0
#define KEY_LOCKED_Y_OFFSET   192
#define KEY_LOCK_WIDTH        11
#define KEY_LOCK_HEIGHT       10

/* where things go on the screen */
#define SCR_X_OFFSET           (CDUIBase->cb_XOffset)
#define SCR_Y_OFFSET           (CDUIBase->cb_YOffset)
#define SCR_PANEL_X_OFFSET     (SCR_X_OFFSET + 32)
#define SCR_PANEL_Y_OFFSET     (SCR_Y_OFFSET)
#define SCR_INBOX_X_OFFSET     (SCR_PANEL_X_OFFSET + 14)
#define SCR_INBOX_Y_OFFSET     (SCR_PANEL_Y_OFFSET + 13)
#define SCR_INBOX_WIDTH        222
#define SCR_INBOX_HEIGHT       85
#define SCR_ITEM_HEIGHT        14
#define SCR_ITEM_BASELINE      9
#define SCR_LOCK_WIDTH         18

#define SCR_UNLOCKED_COUNT_X_OFFSET (SCR_PANEL_X_OFFSET + 79)
#define SCR_UNLOCKED_COUNT_Y_OFFSET (SCR_PANEL_Y_OFFSET + 109)
#define SCR_LOCKED_COUNT_X_OFFSET   (SCR_PANEL_X_OFFSET + 175)
#define SCR_LOCKED_COUNT_Y_OFFSET   (SCR_PANEL_Y_OFFSET + 109)


/*****************************************************************************/


/* debox magic data */
extern UBYTE * __far nramkeys;
extern UBYTE * __far nramimage;
extern UBYTE * __far languageimage;
extern UBYTE * __far keysound;


/*****************************************************************************/


/* local variation of the original NVEntry structure */
struct LocalEntry
{
    struct MinNode le_Link;
    STRPTR         le_ItemName;
    STRPTR         le_AppName;
    ULONG          le_Size;
    ULONG          le_Protection;
};


/*****************************************************************************/


static VOID ShowEntries(struct CDUILib *CDUIBase, struct RastPort *entryRP,
		        struct MinList *itemList, LONG newItem, LONG oldItem)
{
WORD i;

    if (oldItem == -1)
        oldItem = newItem;

    if (newItem < oldItem)
    {
        for (i = 1; i <= SCR_ITEM_HEIGHT; i++)
        {
            WaitBeam(CDUIBase,SCR_INBOX_Y_OFFSET + SCR_INBOX_HEIGHT);
            BltBitMap(entryRP->BitMap,0,oldItem*SCR_ITEM_HEIGHT - i + 6,
                      CDUIBase->cb_BitMap,SCR_INBOX_X_OFFSET,SCR_INBOX_Y_OFFSET,
                      SCR_INBOX_WIDTH,SCR_INBOX_HEIGHT,0xc0,0xff,NULL);

            WaitTOF();
        }
    }
    else if (newItem > oldItem)
    {
        for (i = 1; i <= SCR_ITEM_HEIGHT; i++)
        {
            WaitBeam(CDUIBase,SCR_INBOX_Y_OFFSET + SCR_INBOX_HEIGHT);
            BltBitMap(entryRP->BitMap,0,oldItem*SCR_ITEM_HEIGHT + i + 6,
                      CDUIBase->cb_BitMap,SCR_INBOX_X_OFFSET,SCR_INBOX_Y_OFFSET,
                      SCR_INBOX_WIDTH,SCR_INBOX_HEIGHT,0xc0,0xff,NULL);

            WaitTOF();
        }
    }
    else
    {
        WaitBeam(CDUIBase,SCR_INBOX_Y_OFFSET + SCR_INBOX_HEIGHT);
        BltBitMap(entryRP->BitMap,0,newItem*SCR_ITEM_HEIGHT + 6,
                  CDUIBase->cb_BitMap,SCR_INBOX_X_OFFSET,SCR_INBOX_Y_OFFSET,
                  SCR_INBOX_WIDTH,SCR_INBOX_HEIGHT,0xc0,0xff,NULL);
    }

    /* slow things down... */
    for (i = 0; i < 10; i++)
        WaitTOF();
}


/*****************************************************************************/


VOID Convert0s(STRPTR buffer)
{
UWORD i;

    /* replace 0s with Os, in order to not have angled bars in our numbers */
    i = 0;
    while (buffer[i])
    {
        if (buffer[i] == '0')
            buffer[i] = 'O';

        i++;
    }
}


/*****************************************************************************/


static VOID RenderEntries(struct CDUILib *CDUIBase, struct BitMap *keyBM,
                          struct RastPort *entryRP, struct MinList *itemList)
{
struct LocalEntry *item;
UWORD              y;
char               name[80];
char               buffer[60];
ULONG              locked;
ULONG              unlocked;

    locked   = 0;
    unlocked = 0;

    SetRast(entryRP,3);  /* background should be color 2 */
    SetABPenDrMd(entryRP,2,3,JAM2);

    y = 3 * SCR_ITEM_HEIGHT;  /* leave three blank slots at the top */
    item = (struct LocalEntry *)itemList->mlh_Head;
    while (item->le_Link.mln_Succ)
    {
        if (item->le_Protection & NVEF_DELETE)
        {
            locked  += item->le_Size;

            BltTemplate((APTR)((ULONG)keyBM->Planes[0] + keyBM->BytesPerRow * KEY_LOCKED_Y_OFFSET),KEY_LOCKED_X_OFFSET,keyBM->BytesPerRow,
                        entryRP,3,y,KEY_LOCK_WIDTH,KEY_LOCK_HEIGHT);
        }
        else
        {
            unlocked += item->le_Size;
        }

        sprintf(name,"%.21s-%.21s",item->le_AppName,item->le_ItemName);
        sprintf(buffer,"%-21.21s %3ld",name,item->le_Size);
        Convert0s(buffer);
        Move(entryRP, SCR_LOCK_WIDTH, y + SCR_ITEM_BASELINE);
        Text(entryRP,buffer,strlen(buffer));

        y += SCR_ITEM_HEIGHT;
        item = (struct LocalEntry *)item->le_Link.mln_Succ;
    }

    WaitTOF();

    SetAPen(CDUIBase->cb_RastPort,1);
    RectFill(CDUIBase->cb_RastPort,SCR_UNLOCKED_COUNT_X_OFFSET,
                                   SCR_UNLOCKED_COUNT_Y_OFFSET + 1,
                                   SCR_UNLOCKED_COUNT_X_OFFSET + 24,
                                   SCR_UNLOCKED_COUNT_Y_OFFSET + 7);

    RectFill(CDUIBase->cb_RastPort,SCR_LOCKED_COUNT_X_OFFSET,
                                   SCR_LOCKED_COUNT_Y_OFFSET + 1,
                                   SCR_LOCKED_COUNT_X_OFFSET + 24,
                                   SCR_LOCKED_COUNT_Y_OFFSET + 7);

    SetABPenDrMd(CDUIBase->cb_RastPort,5,1,JAM1);
    sprintf(buffer,"%ld",unlocked);
    Convert0s(buffer);
    Move(CDUIBase->cb_RastPort,SCR_UNLOCKED_COUNT_X_OFFSET + (3-strlen(buffer))*4 - 1,SCR_UNLOCKED_COUNT_Y_OFFSET+7);
    Text(CDUIBase->cb_RastPort,buffer,strlen(buffer));

    sprintf(buffer,"%ld",locked);
    Convert0s(buffer);
    Move(CDUIBase->cb_RastPort,SCR_LOCKED_COUNT_X_OFFSET + (3-strlen(buffer))*4 - 1,SCR_LOCKED_COUNT_Y_OFFSET+7);
    Text(CDUIBase->cb_RastPort,buffer,strlen(buffer));
}


/*****************************************************************************/


static VOID StampBigLock(struct CDUILib *CDUIBase)
{
struct BitMap    *lockBM;
struct BMInfo    *lockBMInfo;
struct Rectangle  rect;
ULONG             id;

    GetBM(CDUIBase,&nramimage,&lockBM,&lockBMInfo);

    id = GetVPModeID(CDUIBase->cb_ViewPort);
    QueryOverscan(id,&rect,OSCAN_TEXT);

    CDUIBase->cb_XOffset = (CDUIBase->cb_Screen->Width - lockBMInfo->bmi_Width) / 2;
    CDUIBase->cb_YOffset = (rect.MaxY - lockBMInfo->bmi_Height + 1) / 2 - CDUIBase->cb_Screen->TopEdge;

    BltBitMap(lockBM,0,0,
              CDUIBase->cb_BitMap,SCR_PANEL_X_OFFSET,SCR_PANEL_Y_OFFSET,
              NRAM_WIDTH,NRAM_HEIGHT,0xc0,0xff,NULL);

    FreeBMInfo(lockBMInfo);
    WaitBlit();
    FreeBitMap(lockBM);
}


/*****************************************************************************/


struct SpriteInfo
{
    UBYTE SrcX;
    UBYTE SrcY;
    UBYTE DestY;
    UBYTE Width;
    UBYTE Height;
};

static const struct SpriteInfo si[] =
{
    {0,0,1,64,63},
    {0,68,3,64,58},
    {0,127,12,64,40},
    {0,167,25,64,14},
    {0,187,30,64,5},
    {64,0,1,64,63},
    {64,68,3,64,58},
    {64,127,12,64,40},
    {64,167,25,64,14},
    {64,187,30,64,5},
    {128,0,1,64,63},
    {128,68,3,64,58},
    {128,127,12,64,40},
    {128,167,25,64,14},
    {128,187,30,64,5},
    {192,0,1,22,63},
    {192,68,3,22,58},
    {192,127,12,22,40},
    {192,167,25,22,14},
    {192,187,30,22,5}
};


static VOID MakeKey(struct CDUILib *CDUIBase, struct BitMap *keyBM)
{
UWORD          i;
struct BitMap *tmpBM;

    for (i = 0; i < 20; i++)
    {
        tmpBM = AllocBitMap(64,70,2,BMF_CLEAR|BMF_INTERLEAVED,NULL);
        BltBitMap(keyBM,si[i].SrcX,si[i].SrcY,tmpBM,0,si[i].DestY,
                  si[i].Width,si[i].Height,0xc0,0xff,NULL);
        WaitBlit();
        CDUIBase->cb_Sprites[i] = AllocSpriteData(tmpBM,SPRITEA_Width,64,TAG_DONE);
        FreeBitMap(tmpBM);
    }

    for (i = 0; i < 20; i += 5)
    {
        GetExtSpriteA(CDUIBase->cb_Sprites[i],NULL);
        MoveSprite(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i],SCR_X_OFFSET-190,0);
    }

    /* get the sprites on the screen! */
    RemakeDisplay();
}


/*****************************************************************************/


#undef SysBase

static VOID InFader(VOID)
{
struct CDUILib  *CDUIBase;
struct ExecBase *SysBase;

    SysBase = (*((struct ExecBase **) 4));
    CDUIBase = SysBase->ThisTask->tc_UserData;

    Wait(SIGBREAKF_CTRL_D);

    FadeIn(CDUIBase);
    CDUIBase->cb_FaderTask = NULL;
}

#define SysBase CDUIBase->cb_SysLib


/*****************************************************************************/


#undef SysBase

static VOID OutFader(VOID)
{
struct CDUILib  *CDUIBase;
struct ExecBase *SysBase;

    SysBase = (*((struct ExecBase **) 4));
    CDUIBase = SysBase->ThisTask->tc_UserData;

    Wait(SIGBREAKF_CTRL_D);

    FadeOut(CDUIBase);
    CDUIBase->cb_FaderTask = NULL;
}

#define SysBase CDUIBase->cb_SysLib


/*****************************************************************************/


#undef SysBase

static VOID ScrollKeyIn(VOID)
{
LONG             x, y;
struct CDUILib  *CDUIBase;
struct ExecBase *SysBase;

    SysBase = (*((struct ExecBase **) 4));
    CDUIBase = SysBase->ThisTask->tc_UserData;

    CDUIBase->cb_FaderTask = CreateTask("Fader",21,InFader,4096);

    y = SCR_PANEL_Y_OFFSET + 22;
    for (x = SCR_X_OFFSET-104; x < SCR_PANEL_X_OFFSET - 55; x += 2)
    {
        if (x == -66)
            Signal(CDUIBase->cb_FaderTask,SIGBREAKF_CTRL_D);

        Forbid();
        WaitTOF();
        MoveSprite(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[0],x,y);
        MoveSprite(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[5],x+32,y);
        MoveSprite(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[10],x+64,y);
        MoveSprite(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[15],x+96,y);
        Permit();
    }

    CDUIBase->cb_ScrollerTask = NULL;
}

#define SysBase CDUIBase->cb_SysLib


/*****************************************************************************/


#undef SysBase

static VOID ScrollKeyOut(VOID)
{
LONG             x, y;
struct CDUILib  *CDUIBase;
struct ExecBase *SysBase;

    SysBase = (*((struct ExecBase **) 4));
    CDUIBase = SysBase->ThisTask->tc_UserData;

    CDUIBase->cb_FaderTask = CreateTask("Fader",21,OutFader,4096);

    y = SCR_PANEL_Y_OFFSET + 22;
    for (x = SCR_PANEL_X_OFFSET - 55; x >= SCR_X_OFFSET-180; x -= 2)
    {
        if (x == -69)
            Signal(CDUIBase->cb_FaderTask,SIGBREAKF_CTRL_D);

        Forbid();
        WaitTOF();
        MoveSprite(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[15],x+96,y);
        MoveSprite(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[10],x+64,y);
        MoveSprite(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[5],x+32,y);
        MoveSprite(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[0],x,y);
        Permit();
    }

    CDUIBase->cb_ScrollerTask = NULL;
}

#define SysBase CDUIBase->cb_SysLib


/*****************************************************************************/


static VOID TurnKey(struct CDUILib *CDUIBase, BOOL lockIt)
{
WORD i;

    if (lockIt)
    {
        for (i = 1; i < 5; i++)
        {
            WaitTOF();
            ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i-1],CDUIBase->cb_Sprites[i],NULL);
            ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i-1+5],CDUIBase->cb_Sprites[i+5],NULL);
            ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i-1+10],CDUIBase->cb_Sprites[i+10],NULL);
            ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i-1+15],CDUIBase->cb_Sprites[i+15],NULL);
        }
    }
    else
    {
        for (i = 3; i >= 0; i--)
        {
            WaitTOF();
            ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i+1],CDUIBase->cb_Sprites[i],NULL);
            ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i+1+5],CDUIBase->cb_Sprites[i+5],NULL);
            ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i+1+10],CDUIBase->cb_Sprites[i+10],NULL);
            ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[i+1+15],CDUIBase->cb_Sprites[i+15],NULL);
        }
    }
}


/*****************************************************************************/


#ifdef TEST_VERSION

struct NVEntry nve[] =
{
    {NULL,NULL,"Lemmings",       0, NVEF_APPNAME},
    {NULL,NULL,"Mom",            22,NVEF_DELETE},
    {NULL,NULL,"Dad",            22,NVEF_DELETE},
    {NULL,NULL,"Killer Potatoes",0, NVEF_APPNAME},
    {NULL,NULL,"Bobby",          12,0},
    {NULL,NULL,"Sammy",          41,0},
    {NULL,NULL,"Cindy",          9, NVEF_DELETE},
    {NULL,NULL,"Boing!",         0, NVEF_APPNAME},
    {NULL,NULL,"Dale",           1, 0},
    {NULL,NULL,"=RJ=",           8, 0},
};

#endif


/*****************************************************************************/


VOID NRAMEditor(struct CDUILib *CDUIBase)
{
struct BitMap       *keyBM;
struct BMInfo       *keyBMInfo;
BOOLEAN              quit;
UBYTE                cmd;
LONG                 currentItem;
LONG                 numItems;
UWORD                i;
struct BitMap       *entryBM;
struct RastPort      entryRP;
struct NVEntry      *entry;
struct MinList      *entryList;
STRPTR               appName;
struct LocalEntry   *item;
struct MinList       itemList;
struct IntuiMessage *intuiMsg;
BOOL                 first;
UWORD                icode;
#ifdef TEST_VERSION
struct MinList       nvl;
#endif

    VideoControlTags(CDUIBase->cb_Screen->ViewPort.ColorMap,
                     VTAG_BORDERSPRITE_SET,     TRUE,
                     VTAG_PF2_TO_SPRITEPRI_SET, 0,
                     VC_IntermediateCLUpdate,   FALSE,
                     VTAG_SPODD_BASE_SET,       NUM_PLAYFIELD_COLORS,
                     VTAG_SPEVEN_BASE_SET,      NUM_PLAYFIELD_COLORS,
                     VTAG_FULLPALETTE_SET,      TRUE,
                     VTAG_SPRITERESN_SET,       SPRITERESN_70NS,
                     TAG_DONE);

    RemakeDisplay();

    GetColors(CDUIBase,&languageimage,&nramkeys);

    StampBigLock(CDUIBase);

    GetBM(CDUIBase,&nramkeys,&keyBM,&keyBMInfo);

#ifdef TEST_VERSION
    NewList((struct List *)&nvl);
    for (i = 0; i < 10; i++)
        AddTail((struct List *)&nvl,(struct Node *)&nve[i]);
    entryList = &nvl;
#else
    entryList = GetNVList(NULL);
#endif

    NewList((struct List *)&itemList);
    numItems = 0;

    entry = (struct NVEntry *)entryList->mlh_Head;
    while (entry->nve_Node.mln_Succ)
    {
        if (entry->nve_Protection & NVEF_APPNAME)
        {
            appName = entry->nve_Name;
        }
        else
        {
            item = AllocVec(sizeof(struct LocalEntry),MEMF_ANY);
            item->le_ItemName   = entry->nve_Name;
            item->le_AppName    = appName;
            item->le_Size       = entry->nve_Size;
            item->le_Protection = entry->nve_Protection;
            AddTail((struct List *)&itemList,(struct Node *)item);
            numItems++;
        }

        entry = (struct NVEntry *)entry->nve_Node.mln_Succ;
    }

    entryBM = AllocBitMap(SCR_INBOX_WIDTH,numItems*SCR_ITEM_HEIGHT + 7*SCR_ITEM_HEIGHT,1,BMF_CLEAR|BMF_INTERLEAVED,NULL);
    InitRastPort(&entryRP);
    SetFont(&entryRP,CDUIBase->cb_RastPort->Font);
    entryRP.BitMap = entryBM;

    RenderEntries(CDUIBase,keyBM,&entryRP,&itemList);

    currentItem = numItems / 2;  /* show the middle of it all */

    ShowEntries(CDUIBase,&entryRP,&itemList,currentItem,-1);
    MakeKey(CDUIBase,keyBM);
    CDUIBase->cb_ScrollerTask = CreateTask("Scroller",22,ScrollKeyIn,4096);

    while (CDUIBase->cb_ScrollerTask)
        WaitTOF();

    first = TRUE;
    quit = FALSE;
    while (!quit)
    {
        WaitPort(CDUIBase->cb_Window->UserPort);
        intuiMsg = (struct IntuiMessage *)GetMsg(CDUIBase->cb_Window->UserPort);
        icode = intuiMsg->Code;

        if (first)
        {
            if (intuiMsg->Qualifier & IEQUALIFIER_REPEAT)
                icode = 0;
            else
                first = FALSE;
        }

        switch (icode)
        {
            case RAWKEY_PORT0_BUTTON_RED:
            case RAWKEY_PORT1_BUTTON_RED:
            case RAWKEY_PORT2_BUTTON_RED:
            case RAWKEY_PORT3_BUTTON_RED:
            case 0x44                   : cmd = CMD_TOGGLE;
                                          break;

            case RAWKEY_PORT0_BUTTON_BLUE:
            case RAWKEY_PORT1_BUTTON_BLUE:
            case RAWKEY_PORT2_BUTTON_BLUE:
            case RAWKEY_PORT3_BUTTON_BLUE:
            case 0x45                    : cmd = CMD_EXIT;
                                           break;

            case RAWKEY_PORT0_JOY_UP:
            case RAWKEY_PORT1_JOY_UP:
            case RAWKEY_PORT2_JOY_UP:
            case RAWKEY_PORT3_JOY_UP:
            case 0x4c               : cmd = CMD_UP;
                                      break;

            case RAWKEY_PORT0_JOY_DOWN:
            case RAWKEY_PORT1_JOY_DOWN:
            case RAWKEY_PORT2_JOY_DOWN:
            case RAWKEY_PORT3_JOY_DOWN:
            case 0x4d                 : cmd = CMD_DOWN;
                                        break;

            default                 : cmd = CMD_NOP;
        }

        ReplyMsg(intuiMsg);

        switch (cmd)
        {
            case CMD_UP    : if (numItems)
                             {
                                 if (currentItem > 0)
                                 {
                                     currentItem--;
                                     ShowEntries(CDUIBase,&entryRP,&itemList,currentItem,currentItem+1);
                                 }
                             }
			     break;

            case CMD_DOWN  : if (numItems)
                             {
                                 if (currentItem < numItems - 1)
                                 {
                                     currentItem++;
                                     ShowEntries(CDUIBase,&entryRP,&itemList,currentItem,currentItem-1);
                                 }
                             }
			     break;

            case CMD_TOGGLE: if (numItems)
                             {
                                 i = currentItem;
                                 item = (struct LocalEntry *)itemList.mlh_Head;
                                 while (i > 0)
                                 {
                                     item = (struct LocalEntry *)item->le_Link.mln_Succ;
                                     i--;
                                 }

                                 if (NVEF_DELETE & item->le_Protection)
                                     item->le_Protection &= ~(NVEF_DELETE);
                                 else
                                     item->le_Protection |= NVEF_DELETE;

                                 TurnKey(CDUIBase,TRUE);
                                 PlaySample(CDUIBase,(UBYTE *)&keysound);
                                 RenderEntries(CDUIBase,keyBM,&entryRP,&itemList);
                                 ShowEntries(CDUIBase,&entryRP,&itemList,currentItem,-1);
#ifndef TEST_VERSION
                                 SetNVProtection(item->le_AppName,item->le_ItemName,item->le_Protection);
#endif
                                 TurnKey(CDUIBase,FALSE);
                             }
                             break;

            case CMD_EXIT  : CDUIBase->cb_ScrollerTask = CreateTask("Scroller",22,ScrollKeyOut,4096);
                             while (item = (struct LocalEntry *)RemHead((struct List *)&itemList))
                                 FreeVec(item);

                             FreeNVData(entryList);

			     FreeBM(CDUIBase,keyBM,keyBMInfo);
                             FreeBitMap(entryBM);

                             while (CDUIBase->cb_ScrollerTask || CDUIBase->cb_FaderTask)
                                 WaitTOF();

                             FreeSprite(CDUIBase->cb_Sprites[0]->es_SimpleSprite.num);
                             FreeSprite(CDUIBase->cb_Sprites[5]->es_SimpleSprite.num);
                             FreeSprite(CDUIBase->cb_Sprites[10]->es_SimpleSprite.num);
                             FreeSprite(CDUIBase->cb_Sprites[15]->es_SimpleSprite.num);

                             for (i = 0; i < 20; i++)
                                 FreeSpriteData(CDUIBase->cb_Sprites[i]);

			     return;
        }
    }
}
