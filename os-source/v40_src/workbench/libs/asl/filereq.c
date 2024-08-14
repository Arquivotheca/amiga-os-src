
/* includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/sghooks.h>
#include <libraries/gadtools.h>
#include <dos/dos.h>
#include <dos/datetime.h>
#include <utility/tagitem.h>
#include <graphics/regions.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <graphics/rpattr.h>
#include <devices/inputevent.h>
#include <utility/hooks.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <string.h>
#include <math.h>

#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>
#include <clib/locale_protos.h>
#include <clib/icon_protos.h>
#include <clib/wb_protos.h>
#include <clib/layers_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/wb_pragmas.h>
#include <pragmas/layers_pragmas.h>

#include "asl.h"
#include "aslbase.h"
#include "aslutils.h"
#include "filereq.h"
#include "asllists.h"
#include "layout.h"
#include "requtils.h"


/*****************************************************************************/


#define LocaleBase ri->ri_LocaleInfo.li_LocaleBase
#define catalog    ri->ri_LocaleInfo.li_Catalog


/*****************************************************************************/


/* stored in ln_Type field */
#define FILETYPE   1
#define DIRTYPE    2
#define VOLUMETYPE 3
#define ASSIGNTYPE 4

#define ln_Selected ln_Pri


struct FileEntry
{
    struct Node      fe_Link;
    UWORD            fe_Pad;
    struct DateStamp fe_Date;
    LONG             fe_Size;
};

struct DirEntry
{
    struct Node      de_Link;
    UWORD	     de_Pad;
    struct DateStamp de_Date;
};

struct AssignEntry
{
    struct Node ae_Link;
};

struct VolumeEntry
{
    struct Node     ve_Link;
    UWORD           ve_Pad;
    STRPTR          ve_DeviceName;
    ULONG           ve_BlocksUsed;
    ULONG           ve_NumBlocks;
    ULONG           ve_BytesPerBlock;
    struct MsgPort *ve_HandlerTask;
};


/*****************************************************************************/


#define SHIFTKEYS (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT)
#define ALTKEYS   (IEQUALIFIER_LALT | IEQUALIFIER_RALT)


/*****************************************************************************/


#define FR_NOP        0
#define FR_OK         1
#define FR_CANCEL     2
#define FR_PREVNAME   3
#define FR_NEXTNAME   4
#define FR_PARENT     5
#define FR_VOLUMES    6
#define FR_DELETE     7
#define FR_PATTERN    8
#define FR_DRAWER     9
#define FR_FILE       10
#define FR_LISTSCROLL 11
#define FR_LISTCLICK  12
#define FR_RESTORE    13


/*****************************************************************************/


static struct ASLMenu far AM[] =
{
    {NM_TITLE,  FR_NOP,      MSG_ASL_CONTROL_MENU},
      {NM_ITEM, FR_PREVNAME, MSG_ASL_CONTROL_LASTNAME},
      {NM_ITEM, FR_NEXTNAME, MSG_ASL_CONTROL_NEXTNAME},
      {NM_ITEM, FR_NOP,      MSG_BARLABEL},
      {NM_ITEM, FR_RESTORE,  MSG_ASL_CONTROL_RESTORE},
      {NM_ITEM, FR_PARENT,   MSG_ASL_CONTROL_PARENT},
      {NM_ITEM, FR_VOLUMES,  MSG_ASL_CONTROL_VOLUMES},
      {NM_ITEM, FR_DELETE,   MSG_ASL_CONTROL_DELETE},
      {NM_ITEM, FR_NOP,      MSG_BARLABEL},
      {NM_ITEM, FR_OK,       MSG_ASL_CONTROL_OK},
      {NM_ITEM, FR_CANCEL,   MSG_ASL_CONTROL_CANCEL},

    {NM_END,    FR_NOP,	     MSG_BARLABEL}
};


/*****************************************************************************/


/* Template for the file requester */
static struct ASLGadget far AG[] =
{
    {HGROUP_KIND,   FR_NOP,        0, MSG_NOTHING,            {  4,-16,  -8, 14}, 1, 0, 0,},
    {BUTTON_KIND,   FR_OK,         0, MSG_ASL_OK_GAD,         {  4,-16,   0,  0}, 1, 0, 0,},
    {BUTTON_KIND,   FR_VOLUMES,    0, MSG_ASL_FR_VOLUMES_GAD, { 78,-16,   0,  0}, 1, 0, 1,},
    {BUTTON_KIND,   FR_PARENT,     0, MSG_ASL_FR_PARENT_GAD,  {152,-16,   0,  0}, 1, 0, 2,},
    {BUTTON_KIND,   FR_CANCEL,     0, MSG_ASL_CANCEL_GAD,     {-74,-16,   0,  0}, 1, 0, 0,},

    {VGROUP_KIND,   FR_NOP,        0, MSG_NOTHING,            {  4,-64,  -8, 46}, 2, 0, 0,},
    {STRING_KIND,   FR_PATTERN,    0, MSG_ASL_FR_PATTERN_GAD, {  0,  0,  -1,  0}, 2, 0, 1,},
    {STRING_KIND,   FR_DRAWER,     0, MSG_ASL_FR_DRAWER_GAD,  {  0,  0,  -1,  0}, 2, 0, 0,},
    {STRING_KIND,   FR_FILE,       0, MSG_ASL_FR_FILE_GAD,    {  0,  0,  -1,  0}, 2, 0, 0,},

    {DGROUP_KIND,   FR_NOP,        0, MSG_NOTHING,            {  4,  2,  -8,-68}, 3, 0, 2,},
    {SCROLLER_KIND, FR_LISTSCROLL, 0, MSG_NOTHING,            {-18,  0,  18,  0}, 3, 0, 0,},
    {GENERIC_KIND,  FR_LISTCLICK,  0, MSG_NOTHING,            {  0,  0, -18,  0}, 3, 0, 0,},

    {END_KIND,}
};

#define	POSGAD		1
#define	VOLUMEGAD	2
#define	PARENTGAD	3
#define	NEGGAD		4

#define	PATTERNGAD	6
#define	DRAWERGAD	7
#define	FILEGAD		8

#define	SCROLLERGAD	10
#define	LISTGAD		11

#define MINLIGHTHEIGHT 3


/*****************************************************************************/


static VOID InitFileList(struct ExtFileReq *fr)
{
struct Node *node;

    while (node = RemHead((struct List *)&fr->fr_FileList))
    {
        if (node->ln_Type == VOLUMETYPE)
            FreeVec(((struct VolumeEntry *)node)->ve_DeviceName);

        FreeVec(node);
    }

    FreeList((struct List *)&fr->fr_TempFileList);

    fr->fr_MaxNameLen   = 10;
    fr->fr_MaxSizeLen   = strlen(GetString(&fr->fr_ReqInfo.ri_LocaleInfo,MSG_ASL_FR_DRAWER));
    fr->fr_TopItem      = 0;
    fr->fr_OldTopItem   = 0;
    fr->fr_OldItemCnt   = 0;
    fr->fr_DisplayValid = FALSE;
    fr->fr_ItemCnt      = 0;
    fr->fr_TempItemCnt  = 0;
    fr->fr_FullRender   = TRUE;
}


/*****************************************************************************/


static VOID InsertListEntry(struct ExtFileReq *fr, struct Node *newEntry)
{
UWORD               nameLen;
struct FileEntry   *file;
struct VolumeEntry *volume;
STRPTR              name;

    fr->fr_ItemCnt++;
    fr->fr_DisplayValid = FALSE;

    nameLen = strlen(newEntry->ln_Name);
    if (nameLen > fr->fr_MaxNameLen)
        fr->fr_MaxNameLen = nameLen;

    if (newEntry->ln_Type == FILETYPE)
    {
        file = (struct FileEntry *)newEntry;
        if (file->fe_Size >= 1000000)
        {
            fr->fr_MaxSizeLen = 11;
        }
        else if (file->fe_Size >= 1000)
        {
            if (fr->fr_MaxSizeLen < 7)
            {
                fr->fr_MaxSizeLen = 7;
            }
        }
    }
    else if (newEntry->ln_Type == VOLUMETYPE)
    {
        volume = (struct VolumeEntry *)newEntry;
        if (name = volume->ve_DeviceName)
        {
            nameLen = strlen(name);
            if (nameLen > fr->fr_MaxSizeLen)
                fr->fr_MaxSizeLen = nameLen;
        }
    }

    EnqueueAlpha2((struct List *)&fr->fr_FileList,newEntry);
}


/*****************************************************************************/


static VOID RemoveListEntry(struct ExtFileReq *fr, STRPTR name)
{
struct Node *node;

    if (node = FindNameNC(&fr->fr_FileList,FilePart(name)))
    {
        Remove(node);
        fr->fr_ItemCnt--;
        fr->fr_DisplayValid = FALSE;
        FreeVec(node);
    }
}


/*****************************************************************************/


#define FORMAT_DEF 4

static VOID ConvDate(struct DateStamp *ds, STRPTR string)
{
struct DateTime dt;
char            date[LEN_DATSTRING];
char            time[LEN_DATSTRING];

    dt.dat_Stamp   = *ds;
    dt.dat_Format  = FORMAT_DEF;
    dt.dat_Flags   = NULL;
    dt.dat_StrDay  = NULL;
    dt.dat_StrDate = date;
    dt.dat_StrTime = time;

    string[0] = 0;
    if (DateToStr(&dt))
    {
        strcpy(string, date);
        strcat(string, " ");
        strcat(string, time);
    }
}


/*****************************************************************************/


#define ROLLOVER_POINT 9999

static VOID ConvVolumeData(struct ExtFileReq *fr, ULONG blocksUsed, ULONG numBlocks,
                           ULONG bytesPerBlock, STRPTR string)
{
ULONG used, free, percent, total;
char  freechar, usedchar;

    string[0] = 0;

    total   = numBlocks;
    used    = blocksUsed;
    percent = (((used * 100) + (total>>1)) / total);
    total  *= bytesPerBlock;
    used   *= bytesPerBlock;
    free    = total - used;

    freechar=usedchar='K';

    if ((free >>= 10) > ROLLOVER_POINT)
    {
        freechar='M';
        free >>= 10;
    }

    if ((used >>= 10) > ROLLOVER_POINT)
    {
        usedchar='M';
        used >>= 10;
    }

    sprintf(string,GetString(&fr->fr_ReqInfo.ri_LocaleInfo,MSG_ASL_FR_VOLUME_INFO),percent,free,freechar,used,usedchar);
}


/*****************************************************************************/


static VOID RenderEntry(struct ExtFileReq *fr, struct Node *entry, struct Rectangle *rect)
{
struct RastPort    *rp;
struct FileEntry   *file;
struct DirEntry    *dir;
struct VolumeEntry *volume;
char                line[120];
char                date[120];
char                volSize[80];
UWORD               frontPen, backPen;
STRPTR              ptr;
STRPTR              name;
struct TextExtent   extent;
WORD                x0,x1;
WORD                y;
UWORD               sizeLen;
STRPTR              drawer;

    if (FRF_DOSAVEMODE & fr->fr_Options)
    {
        frontPen = fr->fr_LVBackPen;
        backPen  = fr->fr_LVTextPen;

        if ((entry->ln_Type == DIRTYPE) || (entry->ln_Type == ASSIGNTYPE))
            if (fr->fr_LVHighTextPen != backPen)
                frontPen = fr->fr_LVHighTextPen;

        if (entry->ln_Selected)
        {
            frontPen = fr->fr_LVTextPen;
            backPen  = fr->fr_LVBackPen;
        }
    }
    else
    {
        frontPen = fr->fr_LVTextPen;
        backPen  = fr->fr_LVBackPen;

        if ((entry->ln_Type == DIRTYPE) || (entry->ln_Type == ASSIGNTYPE))
            frontPen = fr->fr_LVHighTextPen;

        if (entry->ln_Selected)
        {
            frontPen = fr->fr_LVFillTextPen;
            backPen  = fr->fr_LVFillBackPen;
        }
    }

    sizeLen = 0;
    name    = entry->ln_Name;
    switch (entry->ln_Type)
    {
        case FILETYPE  : file = (struct FileEntry *)entry;
                         ConvDate(&file->fe_Date,date);
                         sprintf(line,"%lU",file->fe_Size);
                         sizeLen = strlen(line);
                         strcat(line,"  ");
                         strcat(line,date);
                         break;

        case DIRTYPE   : dir = (struct DirEntry *)entry;
                         ConvDate(&dir->de_Date,date);
                         drawer  = GetString(&fr->fr_ReqInfo.ri_LocaleInfo,MSG_ASL_FR_DRAWER);
                         sizeLen = strlen(drawer);
                         sprintf(line,"%s  %s",drawer,date);
                         break;

        case ASSIGNTYPE: strcpy(line,GetString(&fr->fr_ReqInfo.ri_LocaleInfo,MSG_ASL_FR_ASSIGN));
                         sizeLen = strlen(line);
                         strcpy(date,name);
                         strcat(date,":");
                         name = date;    /* use 'date' as our name buffer */
                         break;

        case VOLUMETYPE: volume = (struct VolumeEntry *)entry;

                         if (!(ptr = volume->ve_DeviceName))
                             ptr = GetString(&fr->fr_ReqInfo.ri_LocaleInfo,MSG_ASL_FR_VOLUME);

                         sizeLen = strlen(ptr);
                         if (volume->ve_HandlerTask)
                             ConvVolumeData(fr,volume->ve_BlocksUsed,volume->ve_NumBlocks,volume->ve_BytesPerBlock,volSize);
                         else
                             volSize[0] = 0;
                         sprintf(line,"%s  %s",ptr,volSize);
                         break;
    }

    rp = &fr->fr_Store->rs_LVRPort;

    SetABPenDrMd(rp,frontPen,backPen,JAM2);

    y = rect->MinY+(((rect->MaxY - rect->MinY + 1) - rp->TxHeight) / 2) + rp->TxBaseline + 1;

    Move(rp,rect->MinX+2,y);
    Text(rp,name,TextFit(rp,name,strlen(name),&extent,NULL,1,rect->MaxX-rect->MinX-3,32767));
    x0 = rp->cp_x;

    x1 = rect->MinX+2+(fr->fr_MaxNameLen+2+fr->fr_MaxSizeLen-sizeLen)*rp->TxWidth;
    if (x1 < rect->MaxX)
    {
       Move(rp,x1,y);
       Text(rp,line,TextFit(rp,line,strlen(line),&extent,NULL,1,rect->MaxX-x1,32767));
    }

    SetAPen(rp,backPen);
    RectFill(rp,rect->MinX,rect->MinY+1,rect->MinX+1,rect->MaxY);
    RectFill(rp,rect->MinX,rect->MinY,rect->MaxX,rect->MinY);

    if ((x0 < x1-1) && (x1 < rect->MaxX-1))
        RectFill(rp,x0,rect->MinY,x1-1,rect->MaxY);

    if (rp->cp_x <= rect->MaxX)
        RectFill(rp,rp->cp_x,rect->MinY+1,rect->MaxX,rect->MaxY);
}


/*****************************************************************************/


static VOID CreateVolumeList(struct ExtFileReq *fr)
{
struct DosList     *dl;
struct VolumeEntry *volume;
struct AssignEntry *assign;
struct MinList      hold;
STRPTR              name;
UWORD               len;
D_S(struct InfoData,id);

    fr->fr_MaxNameLen = 1;
    fr->fr_MaxSizeLen = strlen(GetString(&fr->fr_ReqInfo.ri_LocaleInfo,MSG_ASL_FR_VOLUME));
    len = strlen(GetString(&fr->fr_ReqInfo.ri_LocaleInfo,MSG_ASL_FR_ASSIGN));
    if (len > fr->fr_MaxSizeLen)
        fr->fr_MaxSizeLen = len;

    NewList((struct List *)&hold);

    dl = LockDosList(LDF_VOLUMES|LDF_ASSIGNS|LDF_READ);
    while (dl = NextDosEntry(dl,LDF_VOLUMES|LDF_ASSIGNS|LDF_READ))
    {
        name = (STRPTR)((ULONG)dl->dol_Name * 4);
        if (dl->dol_Type == DLT_VOLUME)
        {
            if (!dl->dol_Task || DoPkt1(dl->dol_Task,ACTION_DISK_INFO,(ULONG)id >> 2))
            {
                if (volume = AllocNamedNode2(sizeof(struct VolumeEntry),VOLUMETYPE,&name[1]))
                {
                    volume->ve_Link.ln_Type  = VOLUMETYPE;
                    volume->ve_NumBlocks     = id->id_NumBlocks;
                    volume->ve_BytesPerBlock = id->id_BytesPerBlock;
                    volume->ve_BlocksUsed    = id->id_NumBlocksUsed;
                    volume->ve_HandlerTask   = dl->dol_Task;
                    AddTail((struct List *)&hold,(struct Node *)volume);
                }
            }
        }
        else if ((dl->dol_Type == DLT_DIRECTORY)
             ||  (dl->dol_Type == DLT_LATE)
             ||  (dl->dol_Type == DLT_NONBINDING))
        {
            if (assign = AllocNode(sizeof(struct AssignEntry) + (ULONG)name[0] + 2))
            {
                assign->ae_Link.ln_Name     = (STRPTR)((ULONG)assign + sizeof(struct AssignEntry) + 1);
                assign->ae_Link.ln_Type     = ASSIGNTYPE;
                assign->ae_Link.ln_Name[-1] = ASSIGNTYPE;
                CopyMem(&name[1],assign->ae_Link.ln_Name,(ULONG)name[0]);
                InsertListEntry(fr,(struct Node *)assign);
            }
        }
    }

    dl = LockDosList(LDF_DEVICES|LDF_READ);
    while (dl = NextDosEntry(dl,LDF_DEVICES|LDF_READ))
    {
        volume = (struct VolumeEntry *)hold.mlh_Head;
        while (volume->ve_Link.ln_Succ)
        {
            if (volume->ve_HandlerTask && (volume->ve_HandlerTask == dl->dol_Task))
            {
                name = (STRPTR)((ULONG)dl->dol_Name * 4);
                if (volume->ve_DeviceName = AllocVec((ULONG)name[0]+3,MEMF_CLEAR|MEMF_ANY))
                {
                    CopyMem(name,volume->ve_DeviceName,(ULONG)name[0]+1);
                    strcat(volume->ve_DeviceName,")");
                    volume->ve_DeviceName[0] = '(';
                }
                break;
            }
            volume = (struct VolumeEntry *)volume->ve_Link.ln_Succ;
        }
    }

    UnLockDosList(LDF_DEVICES|LDF_READ);
    UnLockDosList(LDF_VOLUMES|LDF_ASSIGNS|LDF_READ);

    while (volume = (struct VolumeEntry *)RemHead((struct List *)&hold))
        InsertListEntry(fr,(struct Node *)volume);
}


/*****************************************************************************/


static VOID ReducePath(STRPTR path)
{
UWORD diff,len,i,j,start;

    len = strlen(path);

    i = len;
    while (len > 1)
    {
        if ((path[len-1] == '/') && (path[len-2] != '/') && (path[len-2] != ':'))
            path[--len] = 0;

        i--;
        if ((i == 0) || (path[i] == ':') || (path[i-1] == ':'))
            return;

        if ((i > 1) && (path[i] == '/') && (path[i-1] == '/') && (path[i-2] != ':') && (path[i-2] != '/'))
        {
            start = i;
            i -= 2;
            while ((i > 0) && (path[i] != ':') && (path[i] != '/'))
                i--;

            if ((path[i] == ':') || (path[i] == '/'))
                i++;

            j    = i;
            diff = start-i+1;
            len  -= diff;
            while (j < len)
            {
                path[j] = path[j+diff];
                j++;
            }
            path[len] = 0;
            i = len;
        }
    }
}


/*****************************************************************************/


ULONG GetMaxPen(struct RastPort *rp)
{
struct TagItem tags[2];
ULONG          result;

    tags[0].ti_Tag  = RPTAG_MaxPen;
    tags[0].ti_Data = (ULONG)&result;
    tags[1].ti_Tag  = TAG_DONE;

    GetRPAttrsA(rp,tags);

    return(result);   /* the compiler says this is not initialized, it's wrong! */
}


/*****************************************************************************/


static VOID RenderLV(struct ExtFileReq *fr)
{
UWORD             yPos;
UWORD             i,start,end;
LONG              scrCnt,dy;
struct Node      *node;
struct Rectangle  rect;
ULONG             oldMaxPen;

    start = 0;

    if (!fr->fr_DisplayValid)
    {
        end = fr->fr_VisibleItems;
    }
    else if (fr->fr_TopItem != fr->fr_OldTopItem)
    {
        scrCnt = fr->fr_TopItem - fr->fr_OldTopItem;

        if (abs(scrCnt)+1 < fr->fr_VisibleItems)
        {
            dy = scrCnt * fr->fr_ItemHeight;

            if (dy<0)
            {
                end = -scrCnt;
            }
            else
            {
                start = fr->fr_VisibleItems-scrCnt;
                end   = fr->fr_VisibleItems;
            }

            oldMaxPen = GetMaxPen(fr->fr_ReqInfo.ri_Window->RPort);
            SetMaxPen(fr->fr_ReqInfo.ri_Window->RPort,GetMaxPen(&fr->fr_Store->rs_LVRPort));

            LockLayerInfo(&fr->fr_ReqInfo.ri_Window->WScreen->LayerInfo);
            InstallLayerHook(fr->fr_ReqInfo.ri_Window->WLayer,LAYERS_NOBACKFILL);
            ScrollWindowRaster(fr->fr_ReqInfo.ri_Window,0,dy,
                               fr->fr_LVLeft,fr->fr_LVTop,
                               fr->fr_LVLeft+fr->fr_LVWidth-1,
                               fr->fr_LVTop+fr->fr_ItemHeight*fr->fr_VisibleItems-1);
            InstallLayerHook(fr->fr_ReqInfo.ri_Window->WLayer,LAYERS_BACKFILL);
            UnlockLayerInfo(&fr->fr_ReqInfo.ri_Window->WScreen->LayerInfo);

            SetMaxPen(fr->fr_ReqInfo.ri_Window->RPort,oldMaxPen);
        }
        else
        {
            end = fr->fr_VisibleItems;
        }
    }
    else
    {
        return;
    }

    i    = start + fr->fr_TopItem;
    yPos = fr->fr_LVTop+start*fr->fr_ItemHeight;

    node = fr->fr_FileList.lh_Head;
    while (i-- && node->ln_Succ)
	node = node->ln_Succ;

    rect.MinX = fr->fr_LVLeft;
    rect.MaxX = fr->fr_LVLeft+fr->fr_LVWidth-1;
    while (start < end)
    {
        if (!node->ln_Succ)
            break;

        rect.MinY = yPos;
        rect.MaxY = yPos+fr->fr_ItemHeight-1;
        RenderEntry(fr,node,&rect);
        node = node->ln_Succ;

        yPos = yPos + fr->fr_ItemHeight;
        start++;
    }

    if (!fr->fr_DisplayValid && (start < fr->fr_VisibleItems) && (fr->fr_FullRender))
    {
        if (FRF_DOSAVEMODE & fr->fr_Options)
            SetAPen(&fr->fr_Store->rs_LVRPort,fr->fr_LVTextPen);
        else
            SetAPen(&fr->fr_Store->rs_LVRPort,fr->fr_LVBackPen);

        RectFill(&fr->fr_Store->rs_LVRPort,
                 fr->fr_LVLeft,yPos,
                 fr->fr_LVLeft+fr->fr_LVWidth-1,
                 fr->fr_LVTop+fr->fr_LVHeight-1);

        fr->fr_FullRender = FALSE;
    }

    fr->fr_OldTopItem   = fr->fr_TopItem;
    fr->fr_OldItemCnt   = fr->fr_ItemCnt;
    fr->fr_DisplayValid = TRUE;
}


/*****************************************************************************/


static VOID DoFRGadgets(struct ExtFileReq *fr)
{
struct ReqInfo *ri;

    ri = &fr->fr_ReqInfo;

    if (FRF_DOPATTERNS & fr->fr_Options)
    {
       SetGadgetAttr(ri,PATTERNGAD,GTST_MaxChars, sizeof(fr->fr_PatternString),
                                   GTST_String,   fr->fr_PatternString,
                                   TAG_DONE);
    }

    SetGadgetAttr(ri,DRAWERGAD,GTST_MaxChars, sizeof(fr->fr_DrawerString),
                               GTST_String,   fr->fr_DrawerString,
                               TAG_DONE);

    if (!(FRF_DRAWERSONLY & fr->fr_Options2))
    {
        SetGadgetAttr(ri,FILEGAD,GTST_EditHook, &fr->fr_FileHook,
                                 GTST_MaxChars, sizeof(fr->fr_FileString),
                                 GTST_String,   fr->fr_FileString,
                                 TAG_DONE);
    }

    SetGadgetAttr(ri,SCROLLERGAD,GTSC_Visible, fr->fr_VisibleItems,
                                 GTSC_Total,   fr->fr_ItemCnt,
                                 GTSC_Top,     fr->fr_TopItem,
                                 GTSC_Arrows,  9,
                                 PGA_Freedom,  LORIENT_VERT,
                                 GA_Immediate, TRUE,
                                 GA_RelVerify, TRUE,
                                 TAG_DONE);
}


/*****************************************************************************/


static VOID LimitTopItem(struct ExtFileReq *fr)
{
    if (fr->fr_TopItem + fr->fr_VisibleItems > fr->fr_ItemCnt)
    {
        if (fr->fr_ItemCnt > fr->fr_VisibleItems)
        {
            fr->fr_TopItem = fr->fr_ItemCnt - fr->fr_VisibleItems;
        }
        else
        {
            fr->fr_TopItem = 0;
        }
    }
}


/*****************************************************************************/


static WORD GetMax(struct ExtFileReq *fr, LONG pen, ...)
{
struct DrawInfo *di;
WORD             maxPen;
LONG            *ptr;

    di     = fr->fr_ReqInfo.ri_DrawInfo;
    maxPen = 0;
    ptr    = &pen;
    while (*ptr != -1)
    {
        if (di->dri_Pens[*ptr] > maxPen)
            maxPen = di->dri_Pens[*ptr];

        ptr++;
    }

    return(maxPen);
}


/*****************************************************************************/


static BOOL CreateFRGadgets(struct ExtFileReq *fr)
{
struct ReqInfo  *ri = &fr->fr_ReqInfo;
struct TextFont *defFont;
struct Gadget   *gad;
struct DrawInfo *di;

    FreeLayoutGadgets(ri,TRUE);

    DoFRGadgets(fr);

    if (LayoutGadgets(ri,LGM_CREATE))
    {
	gad              = ri->ri_Template[LISTGAD].ag_Gadget;
        gad->Flags       = GFLG_GADGHNONE;
        gad->Activation  = GACT_IMMEDIATE;
        gad->GadgetType |= GTYP_BOOLGADGET;
        gad->GadgetID    = FR_LISTCLICK;

        AddGList(ri->ri_Window,ri->ri_Gadgets,-1,-1,NULL);
        RefreshGList(ri->ri_Gadgets,ri->ri_Window,NULL,-1);
        GT_RefreshWindow(ri->ri_Window,NULL);

        di      = ri->ri_DrawInfo;
        defFont = fr->fr_ReqInfo.ri_Window->RPort->Font;
        if (defFont->tf_Flags & FPF_PROPORTIONAL)
            defFont = GfxBase->DefaultFont;

        fr->fr_LVTextPen     = di->dri_Pens[TEXTPEN];
        fr->fr_LVBackPen     = di->dri_Pens[BACKGROUNDPEN];
        fr->fr_LVHighTextPen = di->dri_Pens[HIGHLIGHTTEXTPEN];
        fr->fr_LVFillTextPen = di->dri_Pens[FILLTEXTPEN];
        fr->fr_LVFillBackPen = di->dri_Pens[FILLPEN];

        fr->fr_Store->rs_LVRPort = *fr->fr_ReqInfo.ri_Window->RPort;
        fr->fr_LVLeft       = gad->LeftEdge+2;
        fr->fr_LVTop        = gad->TopEdge+1;
        fr->fr_LVWidth      = gad->Width-4;
        fr->fr_LVHeight     = gad->Height-2;
        fr->fr_ItemHeight   = defFont->tf_YSize+1;
        fr->fr_VisibleItems = fr->fr_LVHeight / fr->fr_ItemHeight;
        fr->fr_DisplayValid = FALSE;

        SetFont(&fr->fr_Store->rs_LVRPort,defFont);
        SetDrMd(&fr->fr_Store->rs_LVRPort,JAM2);

        SetMaxPen(&fr->fr_Store->rs_LVRPort,GetMax(fr,TEXTPEN,
                                                      BACKGROUNDPEN,
                                                      HIGHLIGHTTEXTPEN,
                                                      FILLTEXTPEN,
                                                      FILLPEN,
                                                      -1));

        LimitTopItem(fr);

        SetGadgetAttr(ri,SCROLLERGAD,GTSC_Visible, fr->fr_VisibleItems,
                                     GTSC_Top,     fr->fr_TopItem,
                                     TAG_DONE);


        fr->fr_MSpace = TextLength(ri->ri_Window->RPort,"m",1);

        return(TRUE);
    }

    return(FALSE);
}


/*****************************************************************************/


static VOID RenderFRDisplay(struct ExtFileReq *fr)
{
struct Gadget *gad;
struct Gadget *dgad;

    if (FRF_DOSAVEMODE & fr->fr_Options)
        SetAPen(&fr->fr_Store->rs_LVRPort,fr->fr_LVTextPen);
    else
        SetAPen(&fr->fr_Store->rs_LVRPort,fr->fr_LVBackPen);

    RectFill(&fr->fr_Store->rs_LVRPort,fr->fr_LVLeft,fr->fr_LVTop,
                                       fr->fr_LVLeft+fr->fr_LVWidth-1,
                                       fr->fr_LVTop+fr->fr_LVHeight-1);

    gad = fr->fr_ReqInfo.ri_Template[LISTGAD].ag_Gadget;
    AslDrawBevelBox(fr->fr_ReqInfo.ri_Window,
                    gad->LeftEdge-fr->fr_ReqInfo.ri_Window->BorderLeft,
                    gad->TopEdge-fr->fr_ReqInfo.ri_Window->BorderTop,
                    gad->Width,
                    gad->Height,
                    GT_VisualInfo, fr->fr_ReqInfo.ri_VisualInfo,
                    TAG_DONE);

    dgad = fr->fr_ReqInfo.ri_Template[DRAWERGAD].ag_Gadget;

    fr->fr_LightLeft   = gad->LeftEdge;
    fr->fr_LightTop    = dgad->TopEdge+1;
    fr->fr_LightWidth  = fr->fr_MSpace*2-1;
    fr->fr_LightHeight = (WORD)dgad->Height-4;

    if (fr->fr_LightHeight >= MINLIGHTHEIGHT)
    {
        AslDrawBevelBox(fr->fr_ReqInfo.ri_Window,fr->fr_LightLeft-fr->fr_ReqInfo.ri_Window->BorderLeft,
                                                 fr->fr_LightTop-fr->fr_ReqInfo.ri_Window->BorderTop,
                                                 fr->fr_LightWidth,fr->fr_LightHeight,
                                                 GT_VisualInfo, fr->fr_ReqInfo.ri_VisualInfo,
                                                 GTBB_Recessed, TRUE,
                                                 TAG_DONE);
    }
}


/*****************************************************************************/


static VOID SetStrValue(struct ExtFileReq *fr, WORD id, STRPTR text)
{
    SetGadgetAttr(&fr->fr_ReqInfo,id,GTST_String, text,
                                     TAG_DONE);
}


/*****************************************************************************/


static VOID ActivateGad(struct ExtFileReq *fr, UWORD currentGad, UWORD quals)
{
UWORD          next, prev;
struct Gadget *gad;

    if (quals & ALTKEYS)
        return;

    gad = NULL;
    while (!gad)
    {
        switch (currentGad)
        {
            case PATTERNGAD: next = DRAWERGAD;
                             prev = FILEGAD;
                             break;

            case DRAWERGAD : next = FILEGAD;
                             prev = PATTERNGAD;
                             break;

            case FILEGAD   : next = 0;
                             prev = DRAWERGAD;
                             break;
        }

        if (quals & SHIFTKEYS)
        {
            currentGad = prev;
        }
        else if (next != 0)
        {
            currentGad = next;
        }
        else
        {
            return;
        }
        gad = fr->fr_ReqInfo.ri_Template[currentGad].ag_Gadget;
    }

    ActivateGadget(gad,fr->fr_ReqInfo.ri_Window,NULL);
}


/*****************************************************************************/


LONG FixedParsePattern(STRPTR str, APTR buffer, ULONG bufSize)
{
LONG result;

    result = ParsePatternNoCase(str,buffer,bufSize);

    if (result < 0)
        ParsePatternNoCase("#?",buffer,bufSize);

    return(result);
}


/*****************************************************************************/


static VOID ParsePat(struct ExtFileReq *fr)
{
    FixedParsePattern(fr->fr_Pattern,fr->fr_Store->rs_ParsedPattern,sizeof(fr->fr_Store->rs_ParsedPattern));
}


/*****************************************************************************/


static VOID FRPreserve(struct ExtFileReq *fr)
{
    strcpy(fr->fr_Store->rs_OriginalFile,fr->fr_File);
    strcpy(fr->fr_Store->rs_OriginalDrawer,fr->fr_Drawer);
    strcpy(fr->fr_Store->rs_OriginalPattern,fr->fr_Pattern);
}


/*****************************************************************************/


static VOID FRRestore(struct ExtFileReq *fr)
{
    strcpy(fr->fr_File,fr->fr_Store->rs_OriginalFile);
    strcpy(fr->fr_Drawer,fr->fr_Store->rs_OriginalDrawer);
    strcpy(fr->fr_Pattern,fr->fr_Store->rs_OriginalPattern);
    ParsePat(fr);
}


/*****************************************************************************/


static VOID InsertList(struct ExtFileReq *fr)
{
struct Node *node;

    if (fr->fr_TempItemCnt)
    {
        SetGadgetAttr(&fr->fr_ReqInfo,SCROLLERGAD,GTSC_Total,fr->fr_ItemCnt+fr->fr_TempItemCnt,
                                                  TAG_DONE);

        fr->fr_TempItemCnt = 0;
    }

    while (node = RemHead((struct List *)&fr->fr_TempFileList))
	InsertListEntry(fr,node);
}


/*****************************************************************************/


static BOOL MakeDrawer(struct ExtFileReq *fr)
{
BOOL               created = FALSE;
struct DiskObject *diskObj;
struct EasyStruct  est;
BPTR               lock;
WORD               len;

    len = strlen(fr->fr_Drawer);
    if ((len > 0) && ((fr->fr_Drawer[len-1] != '/') && (fr->fr_Drawer[len-1] != ':')))
    {
        est.es_StructSize   = sizeof(struct EasyStruct);
        est.es_Flags        = 0;
        est.es_Title        = GetString(&fr->fr_ReqInfo.ri_LocaleInfo,MSG_ASL_FR_NEWDIR_TITLE);
        est.es_TextFormat   = GetString(&fr->fr_ReqInfo.ri_LocaleInfo,MSG_ASL_FR_NEWDIR_PROMPT);
        est.es_GadgetFormat = GetString(&fr->fr_ReqInfo.ri_LocaleInfo,MSG_ASL_FR_NEWDIR_GAD);

        if (EasyRequestArgs(fr->fr_ReqInfo.ri_Window,&est,NULL,&fr->fr_Drawer))
        {
            if (lock = CreateDir(fr->fr_Drawer))
            {
                created = TRUE;
                UnLock(lock);
                if (diskObj = GetDefDiskObject(WBDRAWER))
                {
                    PutDiskObject(fr->fr_Drawer,diskObj);
                    FreeDiskObject(diskObj);
                }
            }

	    if (!created)
		DisplayBeep(NULL);
        }
    }

    return (created);
}


/*****************************************************************************/


static BOOL DeleteFiles(struct ExtFileReq *fr)
{
BOOL              getDir = FALSE;
STRPTR            buffer;
LONG              len;
struct EasyStruct est;

    InsertList(fr);

    len = strlen(fr->fr_DrawerString) + strlen(fr->fr_FileString) + 10;
    if (buffer = AllocVec(len,MEMF_ANY))
    {
	strcpy(buffer,fr->fr_DrawerString);
	AddPart(buffer,fr->fr_FileString,len);
        ReducePath(buffer);

        est.es_StructSize   = sizeof(struct EasyStruct);
        est.es_Flags        = 0;
        est.es_Title        = GetString(&fr->fr_ReqInfo.ri_LocaleInfo,MSG_ASL_FR_DELETE_TITLE);;
        est.es_TextFormat   = GetString(&fr->fr_ReqInfo.ri_LocaleInfo,MSG_ASL_FR_DELETE_PROMPT);
        est.es_GadgetFormat = GetString(&fr->fr_ReqInfo.ri_LocaleInfo,MSG_ASL_FR_DELETE_GAD);

        if (EasyRequestArgs(fr->fr_ReqInfo.ri_Window,&est,NULL,&buffer))
        {
            if (DeleteFile(buffer))
	    {
		RemoveListEntry(fr,buffer);

                if (DeleteDiskObject(buffer))
                {
                    strcat(buffer,".info");
                    RemoveListEntry(fr,buffer);
                }

                if (strlen(fr->fr_FileString))
		{
		    fr->fr_FileString[0] = 0;
                    SetStrValue(fr,FILEGAD,"");
		    fr->fr_DisplayValid = FALSE;
		    fr->fr_FullRender   = TRUE;
		}
		else
                {
                    len = strlen(fr->fr_DrawerString);
                    if ((len == 0) || (fr->fr_DrawerString[len-1] == '/') || (fr->fr_DrawerString[len-1] == ':'))
                        strcat(fr->fr_DrawerString,"/");
                    else
                        strcat(fr->fr_DrawerString,"//");

                    ReducePath(fr->fr_DrawerString);
                    SetStrValue(fr,DRAWERGAD,fr->fr_DrawerString);
                    getDir = TRUE;
                }
	    }
	    else
	    {
                DisplayBeep(NULL);
	    }
        }

	FreeVec(buffer);
    }

    LimitTopItem(fr);
    RenderLV(fr);
    SetGadgetAttr(&fr->fr_ReqInfo,SCROLLERGAD,GTSC_Total, fr->fr_ItemCnt,
                                              GTSC_Top,   fr->fr_TopItem,
                                              TAG_DONE);

    return(getDir);
}


/*****************************************************************************/


/* returns TRUE if directory needs to be rescanned */
static BOOL SelectEntry(struct ExtFileReq *fr, struct Node *entry, UWORD quals,
                        BOOL clickedOn)
{
struct Node *node;
BOOL         getDir = FALSE;

    if ((FRF_DOSAVEMODE & fr->fr_Options)
    || (!(FRF_DOMULTISELECT & fr->fr_Options))
    || (!(quals & SHIFTKEYS)))
    {
        node = fr->fr_FileList.lh_Head;
        while (node)
        {
            node->ln_Selected = FALSE;
            node = node->ln_Succ;
        }
    }

    if (entry->ln_Type == FILETYPE)
    {
        entry->ln_Selected = TRUE;
        strcpy(fr->fr_FileString,entry->ln_Name);
        SetStrValue(fr,FILEGAD,fr->fr_FileString);
    }
    else if (clickedOn)
    {
        if (entry->ln_Type == DIRTYPE)
        {
            AddPart(fr->fr_DrawerString,entry->ln_Name,sizeof(fr->fr_DrawerString));
        }
        else
        {
            strcpy(fr->fr_DrawerString,entry->ln_Name);
            if ((entry->ln_Type == VOLUMETYPE) || (entry->ln_Type == ASSIGNTYPE))
                strcat(fr->fr_DrawerString,":");
        }
        SetStrValue(fr,DRAWERGAD,fr->fr_DrawerString);
        getDir = TRUE;
    }
    else
    {
        entry->ln_Selected = TRUE;

        strcpy(fr->fr_FileString,entry->ln_Name);
	if (entry->ln_Type == DIRTYPE)
	    strcat(fr->fr_FileString,"/");
	else if ((entry->ln_Type == VOLUMETYPE) || (entry->ln_Type == ASSIGNTYPE))
	    strcat(fr->fr_FileString,":");

        SetStrValue(fr,FILEGAD,fr->fr_FileString);
    }

    fr->fr_DisplayValid = FALSE;
    RenderLV(fr);

    return(getDir);
}


/*****************************************************************************/


static VOID FreeMultiArgs(struct ExtFileReq *fr)
{
    if (fr->fr_ArgList)
    {
        UnLock(fr->fr_ArgList->wa_Lock);

        while (fr->fr_NumArgs--)
            FreeVec(fr->fr_ArgList[fr->fr_NumArgs].wa_Name);

        FreeVec(fr->fr_ArgList);
        fr->fr_ArgList = NULL;
    }
}


/*****************************************************************************/


static BOOL QuitFR(struct ExtFileReq *fr, BOOL cancelled, BPTR dirLock)
{
struct Node *node;
ULONG        cnt;

    if (dirLock)
        WaitPort(&fr->fr_Store->rs_PacketPort);   /* wait for packet return */

    if (cancelled)
    {
        UnLock(dirLock);
        return(FALSE);
    }

    if ((FRF_DOMULTISELECT & fr->fr_Options) && (!(FRF_DOSAVEMODE & fr->fr_Options)))
    {
	if (!dirLock)
	    dirLock = Lock(fr->fr_DrawerString, ACCESS_READ);
	    if (!dirLock)
	        return(FALSE);

        fr->fr_NumArgs = 0;
        cnt            = 0;
        node           = fr->fr_FileList.lh_Head;
        while (node->ln_Succ)
        {
            if (node->ln_Selected)
                cnt++;
            node = node->ln_Succ;
        }

        /* add one to the count, to include the string gadget contents */
        if (fr->fr_ArgList = AllocVec(sizeof(struct WBArg)*(cnt+1),MEMF_CLEAR|MEMF_ANY))
        {
            fr->fr_NumArgs = cnt;

            cnt  = 0;
            node = fr->fr_FileList.lh_Head;
            while (node->ln_Succ)
            {
                if (node->ln_Selected)
                {
                    fr->fr_ArgList[cnt].wa_Lock = dirLock;
                    if (!(fr->fr_ArgList[cnt].wa_Name = AllocVec(strlen(node->ln_Name)+1,MEMF_ANY)))
                    {
                        FreeMultiArgs(fr);
                        return(FALSE);
                    }

                    strcpy(fr->fr_ArgList[cnt].wa_Name,node->ln_Name);
                    cnt++;
                }
                node = node->ln_Succ;
            }

            if (fr->fr_FileString[0])
            {
                node = FindNameNC(&fr->fr_FileList,fr->fr_FileString);

                if (!node || (!node->ln_Selected))
                {
                    fr->fr_ArgList[cnt].wa_Lock = dirLock;
                    if (!(fr->fr_ArgList[cnt].wa_Name = AllocVec(strlen(fr->fr_FileString)+1,MEMF_ANY)))
                    {
                        FreeMultiArgs(fr);
                        return(FALSE);
                    }
                    fr->fr_NumArgs++;
                    strcpy(fr->fr_ArgList[cnt].wa_Name,fr->fr_FileString);
                }
            }
        }
	else
	{
	    UnLock(dirLock);
	}
    }
    else
    {
        UnLock(dirLock);
    }

    return(TRUE);
}


/*****************************************************************************/


static BOOL UserFilters(struct ExtFileReq *fr)
{
BOOL result;

    result = TRUE;
    if (fr->fr_CallOldFilter && fr->fr_ReqInfo.ri_OldHook)
    {
         result = (CallFunc(fr->fr_ReqInfo.ri_OldHook,FRF_FILTERFUNC,(ULONG)&fr->fr_Store->rs_AP,(ULONG)PUBLIC_FR(fr),fr->fr_ReqInfo.ri_A4) == 0);
    }

    if (result && fr->fr_FilterFunc)
    {
         result = CallHookPkt(fr->fr_FilterFunc,PUBLIC_FR(fr),&fr->fr_Store->rs_AP);
    }

    return(result);
}


/*****************************************************************************/


static VOID CursorSelect(struct ExtFileReq *fr, struct Gadget *gadget,
                         UWORD icode, UWORD quals)
{
struct Node    *node;
struct ReqInfo *ri;
BOOL            found;
ULONG           num;
ULONG           top;
UWORD           len;
UWORD           cnt;
BOOL            stringBased;

    ri = &fr->fr_ReqInfo;
    InsertList(fr);

    if (!IsListEmpty(&fr->fr_FileList))
    {
        top = fr->fr_TopItem;
        if (quals & ALTKEYS)
        {
            if (icode == CURSORUP)
            {
                SelectEntry(fr,fr->fr_FileList.lh_Head,0,FALSE);
                top = 0;
            }
            else
            {
                SelectEntry(fr,fr->fr_FileList.lh_TailPred,0,FALSE);
                if (fr->fr_ItemCnt > fr->fr_VisibleItems)
                    top = fr->fr_ItemCnt - fr->fr_VisibleItems;
                else
                    top = 0;
            }
        }
        else
        {
	    node = fr->fr_FileList.lh_Head;
	    while (node->ln_Succ && !node->ln_Selected)
	        node = node->ln_Succ;

            stringBased = FALSE;
            if (!node->ln_Succ)
            {
                stringBased = TRUE;
                if (len = strlen(fr->fr_FileString))
                    if ((fr->fr_FileString[len-1] == '/') || (fr->fr_FileString[len-1] == ':'))
                        fr->fr_FileString[len-1] = 0;

                if (!(node = FindClosest(&fr->fr_FileList,fr->fr_FileString)))
                    node = fr->fr_FileList.lh_TailPred;
            }

            cnt = 1;
            if ((quals & SHIFTKEYS) && (fr->fr_VisibleItems > 1))
                cnt = fr->fr_VisibleItems-1;

            found = FALSE;
            if (stringBased && (Stricmp(node->ln_Name,fr->fr_FileString)))
            {
                found = TRUE;
            }
            else if (icode == CURSORUP)
            {
                while ((node != fr->fr_FileList.lh_Head) && cnt)
                {
                    node = node->ln_Pred;
                    found = TRUE;
                    cnt--;
                }
            }
            else if (icode == CURSORDOWN)
            {
                while ((node != fr->fr_FileList.lh_TailPred) && cnt)
                {
                    node = node->ln_Succ;
                    found = TRUE;
                    cnt--;
                }
            }

            if (found)
            {
                SelectEntry(fr,node,0,FALSE);

                num = FindNodeNum((struct List *)&fr->fr_FileList,node);

                if (num < fr->fr_TopItem)
                    top = num;

                if (num > fr->fr_TopItem + fr->fr_VisibleItems - 1)
                    top = num - fr->fr_VisibleItems + 1;
            }
        }

        fr->fr_TopItem = top;
        RenderLV(fr);
        SetGadgetAttr(&fr->fr_ReqInfo,SCROLLERGAD,GTSC_Top, top,
                                                  TAG_DONE);
    }

    if (gadget)
	ActivateGadget(gadget,ri->ri_Window,NULL);
}


/*****************************************************************************/


static VOID DriveLight(struct ExtFileReq *fr, UWORD pen)
{
    if (fr->fr_LightHeight >= MINLIGHTHEIGHT)
    {
        SetAPen(fr->fr_ReqInfo.ri_Window->RPort,fr->fr_ReqInfo.ri_DrawInfo->dri_Pens[pen]);
        RectFill(fr->fr_ReqInfo.ri_Window->RPort,fr->fr_LightLeft+2,
                                                 fr->fr_LightTop+1,
                                                 fr->fr_LightLeft+fr->fr_LightWidth-3,
                                                 fr->fr_LightTop+fr->fr_LightHeight-2);
    }
}


/*****************************************************************************/


static VOID ActivateMajorGuy(struct ExtFileReq *fr)
{
    if (FRF_DRAWERSONLY & fr->fr_Options2)
        ActivateGadget(fr->fr_ReqInfo.ri_Template[DRAWERGAD].ag_Gadget,fr->fr_ReqInfo.ri_Window,NULL);
    else
        ActivateGadget(fr->fr_ReqInfo.ri_Template[FILEGAD].ag_Gadget,fr->fr_ReqInfo.ri_Window,NULL);
}


/*****************************************************************************/


static BOOL DoParent(struct ExtFileReq *fr)
{
UWORD len;
BPTR  lock;
BPTR  parent;
BOOL  getVolumes;

    getVolumes = TRUE;
    if (lock = Lock(fr->fr_DrawerString,ACCESS_READ))
    {
        if (parent = ParentDir(lock))
        {
            len = strlen(fr->fr_DrawerString);
            if ((len == 0) || (fr->fr_DrawerString[len-1] == '/') || (fr->fr_DrawerString[len-1] == ':'))
                strcat(fr->fr_DrawerString,"/");
            else
                strcat(fr->fr_DrawerString,"//");

            ReducePath(fr->fr_DrawerString);
            SetStrValue(fr,DRAWERGAD,fr->fr_DrawerString);
            getVolumes = FALSE;

            UnLock(parent);
        }
        UnLock(lock);
    }

    return(getVolumes);
}


/*****************************************************************************/


static BOOL HandleFRIDCMP(struct ExtFileReq *fr)
{
struct IntuiMessage   *intuiMsg;
ULONG                  class;
UWORD                  icode;
WORD                   mouseY;
UWORD                  quals;
struct Gadget         *gadget;
struct Window         *window;
ULONG                  seconds;
ULONG                  micros;
struct MenuItem       *menuItem;
struct ReqInfo        *ri;
struct Node           *node;
struct Node           *entry;
BOOL                   getDir, getVolumes, new, volumes;
STRPTR                 ptr;
BPTR                   lock;
BOOL                   accept;
BOOL                   newsize;
char                   infoPat[48];
struct FileLock       *lockPtr;
struct FileEntry      *file;
struct DirEntry       *dir;
struct Gadget         *gad;
struct AppMessage     *appMsg;
struct FileInfoBlock  *fib;
struct StandardPacket *packet;
struct FRStore        *store;
ULONG                  tickCnt;
BOOL                   firstBatch;
struct Node           *oldEntry;
ULONG                  oldSeconds;
ULONG                  oldMicros;
ULONG                  mask;
BOOLEAN                clearTempPat;

    ri           = &fr->fr_ReqInfo;
    volumes      = FALSE;
    getDir       = TRUE;
    getVolumes   = FALSE;
    lock         = NULL;
    newsize      = FALSE;
    store        = fr->fr_Store;
    fib          = &store->rs_AC.an_Info;
    packet       = &store->rs_StdPkt;
    clearTempPat = TRUE;

    mask = (1L<<store->rs_PacketPort.mp_SigBit) | (1L<<store->rs_AppPort.mp_SigBit);

    FixedParsePattern("#?.(INFO|BACKDROP)",infoPat,sizeof(infoPat));
    ParsePat(fr);

    ptr = FilePart(fr->fr_DrawerString);
    if (FixedParsePattern(ptr,store->rs_TempPattern,sizeof(store->rs_TempPattern)) > 0)
    {
        *ptr         = 0;
        clearTempPat = FALSE;
        SetStrValue(fr,DRAWERGAD,fr->fr_DrawerString);
    }

    ActivateMajorGuy(fr);

    while (TRUE)
    {
        if (getDir)
        {
            oldEntry   = NULL;
            oldSeconds = 0;      /* not needed, but keeps compiler from complaining of uninitialized var */
            oldMicros  = 0;
            tickCnt    = 0;
            firstBatch = TRUE;

            if (clearTempPat)
                store->rs_TempPattern[0] = 0;
            clearTempPat = TRUE;

            InitFileList(fr);

            if (!getVolumes)
            {
                RenderLV(fr);
                SetGadgetAttr(ri,SCROLLERGAD,
                              GTSC_Visible, fr->fr_VisibleItems,
                              GTSC_Total,   0,
                              GTSC_Top,     0,
                              TAG_DONE);
            }
            DriveLight(fr,FILLPEN);

            if (lock)
            {
                WaitPort(&store->rs_PacketPort);   /* wait for packet return */
                GetMsg(&store->rs_PacketPort);     /* get packet out of port */
                UnLock(lock);
                lock = NULL;
            }

            if (!getVolumes)
            {
                volumes = FALSE;
                if (lock = Lock(fr->fr_DrawerString,ACCESS_READ))
                {
                    if (Examine(lock,fib) && (fib->fib_DirEntryType > 0))
                    {
                        packet->sp_Pkt.dp_Type = ACTION_EXAMINE_NEXT;
                        packet->sp_Pkt.dp_Arg1 = lock;
                        packet->sp_Pkt.dp_Arg2 = (ULONG)fib >> 2;
                        lockPtr                = (struct FileLock *)((ULONG)lock << 2);
                        SendPkt(&packet->sp_Pkt,lockPtr->fl_Task,&store->rs_PacketPort);
                        store->rs_AC.an_Lock   = lock;
                    }
                    else
                    {
                        UnLock(lock);
                        lock = NULL;
                        DisplayBeep(ri->ri_Screen);
                        getVolumes = TRUE;
                    }
                }
                else if (IoErr() != ERROR_ACTION_NOT_KNOWN)
                {
                    DriveLight(fr,BACKGROUNDPEN);
                    if (FRF_DOSAVEMODE & fr->fr_Options)
                    {
                        getVolumes = !(MakeDrawer(fr));
                    }
                    else
                    {
                        DisplayBeep(ri->ri_Screen);
                        getVolumes = TRUE;
                    }
                }
            }

            if (getVolumes)
            {
                volumes = TRUE;
                CreateVolumeList(fr);

                RenderLV(fr);
                SetGadgetAttr(ri,SCROLLERGAD,
                              GTSC_Visible, fr->fr_VisibleItems,
                              GTSC_Total,   fr->fr_ItemCnt,
                              GTSC_Top,     0,
                              TAG_DONE);

                DriveLight(fr,BACKGROUNDPEN);
            }
            getDir     = FALSE;
            getVolumes = FALSE;
        }

        window = ri->ri_Window;
        if (newsize || (LAYERREFRESH & window->WLayer->Flags))
        {
             LockLayerInfo(&window->WScreen->LayerInfo);

             GT_BeginRefresh(window);
             if (newsize)
                 GT_EndRefresh(window,TRUE);

             RenderFRDisplay(fr);
             fr->fr_DisplayValid = FALSE;
             RenderLV(fr);

             if (lock)
                 DriveLight(fr,FILLPEN);

             if (!newsize)
                 GT_EndRefresh(window,TRUE);

             UnlockLayerInfo(&window->WScreen->LayerInfo);

             newsize = FALSE;
        }

        intuiMsg = GetReqMsg(ri,PUBLIC_FR(fr),NULL,mask);

        if (GetMsg(&store->rs_PacketPort))
        {
            if (packet->sp_Pkt.dp_Res1)
            {
                packet->sp_Pkt.dp_Type = ACTION_EXAMINE_NEXT;
                packet->sp_Pkt.dp_Arg1 = lock;
                packet->sp_Pkt.dp_Arg2 = (ULONG)fib >> 2;
                lockPtr                = (struct FileLock *)((ULONG)lock << 2);

                store->rs_AP.ap_Info = store->rs_AC.an_Info;

                SendPkt(&packet->sp_Pkt,lockPtr->fl_Task,&store->rs_PacketPort);

                BtoC(store->rs_AP.ap_Info.fib_FileName,store->rs_AP.ap_Info.fib_FileName);

                accept = TRUE;
                if ((store->rs_AP.ap_Info.fib_DirEntryType < 0) || (FRF_FILTERDRAWERS & fr->fr_Options2))
                {
                    accept = (((store->rs_AP.ap_Info.fib_DirEntryType > 0) || (!(FRF_DRAWERSONLY & fr->fr_Options2)))
                             && (!(FRF_REJECTICONS & fr->fr_Options2) || !MatchPatternNoCase(infoPat,store->rs_AP.ap_Info.fib_FileName))
                             && (!fr->fr_AcceptPattern || MatchPatternNoCase(fr->fr_AcceptPattern,store->rs_AP.ap_Info.fib_FileName))
                             && (!fr->fr_RejectPattern || !MatchPatternNoCase(fr->fr_RejectPattern,store->rs_AP.ap_Info.fib_FileName))
                             && (UserFilters(fr))
                             && (!store->rs_TempPattern[0] || MatchPatternNoCase(store->rs_TempPattern,store->rs_AP.ap_Info.fib_FileName))
                             && (!store->rs_ParsedPattern[0] || MatchPatternNoCase(store->rs_ParsedPattern,store->rs_AP.ap_Info.fib_FileName)));
                }
                else if (store->rs_AP.ap_Info.fib_DirEntryType > 0)
                {
                    accept = UserFilters(fr);
                }

                if (accept)
                {
                    node = NULL;
                    if (store->rs_AP.ap_Info.fib_DirEntryType > 0)
                    {
                        if (dir = AllocNamedNode2(sizeof(struct DirEntry),DIRTYPE,store->rs_AP.ap_Info.fib_FileName))
                        {
                            dir->de_Link.ln_Type = DIRTYPE;
                            dir->de_Date = store->rs_AP.ap_Info.fib_Date;
                            node = (struct Node *)dir;
                        }
                    }
                    else
                    {
                        if (file = AllocNamedNode2(sizeof(struct FileEntry),FILETYPE,store->rs_AP.ap_Info.fib_FileName))
                        {
                            file->fe_Link.ln_Type = FILETYPE;
                            file->fe_Date = store->rs_AP.ap_Info.fib_Date;
                            file->fe_Size = store->rs_AP.ap_Info.fib_Size;
                            node = (struct Node *)file;
                        }
                    }

                    if (node)
                    {
                        if ((fr->fr_OldItemCnt < fr->fr_VisibleItems) || (firstBatch))
                        {
                            if (fr->fr_ItemCnt == 0)
                                tickCnt = 0;

                            InsertListEntry(fr,node);
                            if (tickCnt > 4)
                            {
                                tickCnt    = 0;
                                firstBatch = FALSE;
                                RenderLV(fr);
                            }
                        }
                        else
                        {
                            AddTail((struct List *)&fr->fr_TempFileList,node);
                            fr->fr_TempItemCnt++;
                        }
                    }
                }
            }
            else
            {
                UnLock(lock);
                lock = NULL;

                SetGadgetAttr(ri,SCROLLERGAD,GTSC_Total,fr->fr_ItemCnt+fr->fr_TempItemCnt,
                                             TAG_DONE);

                RenderLV(fr);
                DriveLight(fr,BACKGROUNDPEN);
            }
        }

        while (appMsg = (struct AppMessage *)GetMsg(&store->rs_AppPort))
        {
            ActivateWindow(ri->ri_Window);

            if (appMsg->am_NumArgs)
            {
                NameFromLock(appMsg->am_ArgList->wa_Lock,fr->fr_DrawerString,sizeof(fr->fr_DrawerString));
                SetStrValue(fr,DRAWERGAD,fr->fr_DrawerString);
                getDir = TRUE;

                if ((appMsg->am_ArgList->wa_Name) && (strlen(appMsg->am_ArgList->wa_Name) > 0))
                {
                    strcpy(fr->fr_FileString,appMsg->am_ArgList->wa_Name);
                    SetStrValue(fr,FILEGAD,fr->fr_FileString);
                }
            }

            ReplyMsg((struct Message *)appMsg);
        }

        if (intuiMsg)
        {
            class   = intuiMsg->Class;
            icode   = intuiMsg->Code;
            gadget  = (struct Gadget *)intuiMsg->IAddress;
            window  = intuiMsg->IDCMPWindow;
            mouseY  = intuiMsg->MouseY;
            quals   = intuiMsg->Qualifier;
            seconds = intuiMsg->Seconds;
            micros  = intuiMsg->Micros;
            GT_ReplyIMsg(intuiMsg);

            if (gad = ri->ri_Template[PATTERNGAD].ag_Gadget)
                strcpy(fr->fr_PatternString,((struct StringInfo *)gad->SpecialInfo)->Buffer);

            if (gad = ri->ri_Template[DRAWERGAD].ag_Gadget)
                strcpy(fr->fr_DrawerString,((struct StringInfo *)gad->SpecialInfo)->Buffer);

            if (gad = ri->ri_Template[FILEGAD].ag_Gadget)
                strcpy(fr->fr_FileString,((struct StringInfo *)gad->SpecialInfo)->Buffer);

            switch (class)
            {
                case IDCMP_REFRESHWINDOW: break;

                case IDCMP_ACTIVEWINDOW : ActivateMajorGuy(fr);
                                          break;

                case IDCMP_CLOSEWINDOW  : return(QuitFR(fr,TRUE,lock));

                case IDCMP_MENUPICK     : while (icode != MENUNULL)
                                          {
                                              menuItem = ItemAddress(ri->ri_Menus,icode);
                                              switch ((UWORD)MENU_USERDATA(menuItem))
                                              {
                                                  case FR_CANCEL  : return(QuitFR(fr,TRUE,lock));
                                                  case FR_OK      : return(QuitFR(fr,FALSE,lock));

                                                  case FR_RESTORE : FRRestore(fr);
                                                                    SetStrValue(fr,FILEGAD,fr->fr_FileString);
                                                                    SetStrValue(fr,DRAWERGAD,fr->fr_DrawerString);
                                                                    SetStrValue(fr,PATTERNGAD,fr->fr_PatternString);
                                                                    getDir = TRUE;
                                                                    break;

                                                  case FR_PARENT  : getDir     = TRUE;
                                                                    getVolumes = DoParent(fr);
                                                                    break;

                                                  case FR_VOLUMES : getDir     = TRUE;
                                                                    getVolumes = !volumes;
                                                                    break;

                                                  case FR_DELETE  : getDir = DeleteFiles(fr);
                                                                    break;

                                                  case FR_PREVNAME: CursorSelect(fr,NULL,CURSORUP,quals);
								    break;

                                                  case FR_NEXTNAME: CursorSelect(fr,NULL,CURSORDOWN,quals);
                                                                    break;

                                                  default         : break;
                                              }
                                              icode = menuItem->NextSelect;
                                          }
                                          ActivateMajorGuy(fr);
                                          break;

                case IDCMP_MOUSEBUTTONS : if (icode == SELECTUP)
                                          {
                                              window->Flags &= ~(WFLG_RMBTRAP);
                                              ActivateMajorGuy(fr);
                                              break;
                                          }
                                          else if (icode == MENUDOWN)
                                          {
                                              if (oldEntry)
                                              {
                                                  oldEntry->ln_Selected = FALSE;
                                                  oldEntry = NULL;
                                                  SetStrValue(fr,FILEGAD,"");
                                                  fr->fr_FileString[0] = 0;
                                                  fr->fr_DisplayValid = FALSE;
                                                  RenderLV(fr);
                                              }
                                              window->Flags &= ~(WFLG_RMBTRAP);
                                          }
                                          else if ((icode == MIDDLEDOWN) && (!(quals & (SHIFTKEYS|ALTKEYS))))
                                          {
                                              getDir     = TRUE;
                                              getVolumes = !volumes;
                                          }
                                          break;

                case IDCMP_GADGETUP     : if ((gadget->GadgetType & STRGADGET) && (icode == -1))
                                          {
                                              ActivateGadget(gadget,window,NULL);
                                              break;
                                          }

                case IDCMP_MOUSEMOVE    :
                case IDCMP_GADGETDOWN   : switch ((UWORD)gadget->GadgetID)
                                          {
                                              case FR_CANCEL    : return(QuitFR(fr,TRUE,lock));
                                              case FR_OK        : return(QuitFR(fr,FALSE,lock));

                                              case FR_LISTSCROLL: fr->fr_TopItem = icode;
                                                                  InsertList(fr);
                                                                  RenderLV(fr);
                                                                  break;

                                              case FR_LISTCLICK : window->Flags |= WFLG_RMBTRAP;
                                                                  icode = (mouseY - (WORD)fr->fr_LVTop) / fr->fr_ItemHeight;
                                                                  if (icode < fr->fr_VisibleItems)
                                                                  {
                                                                      icode += fr->fr_TopItem;
                                                                      entry = fr->fr_FileList.lh_Head;
                                                                      while (icode-- && entry->ln_Succ)
                                                                          entry = entry->ln_Succ;

                                                                      if (entry->ln_Succ)
                                                                      {
                                                                          getDir = SelectEntry(fr,entry,quals,TRUE);

                                                                          if ((entry == oldEntry)
                                                                          && (!(FRF_DOSAVEMODE & fr->fr_Options))
                                                                          && (DoubleClick(oldSeconds,oldMicros,seconds,micros)))
                                                                              return(QuitFR(fr,FALSE,lock));

                                                                          oldEntry   = entry;
                                                                          oldSeconds = seconds;
                                                                          oldMicros  = micros;
                                                                      }
                                                                      else
                                                                      {
                                                                          oldEntry = NULL;
                                                                      }
                                                                  }
                                                                  break;

                                              case FR_PARENT    : getDir     = TRUE;
                                                                  getVolumes = DoParent(fr);
                                                                  ActivateMajorGuy(fr);
                                                                  break;

                                              case FR_VOLUMES   : getDir     = TRUE;
                                                                  getVolumes = !volumes;
                                                                  ActivateMajorGuy(fr);
                                                                  break;

                                              case FR_PATTERN   : ActivateGad(fr,PATTERNGAD,quals);
                                                                  getDir = TRUE;
                                                                  ParsePat(fr);
                                                                  break;

                                              case FR_DRAWER    : ReducePath(fr->fr_DrawerString);
                                                                  ptr = FilePart(fr->fr_DrawerString);
                                                                  if (FixedParsePattern(ptr,store->rs_TempPattern,sizeof(store->rs_TempPattern)) > 0)
                                                                  {
                                                                      *ptr = 0;
                                                                      clearTempPat = FALSE;
                                                                  }
                                                                  else
                                                                  {
                                                                      store->rs_TempPattern[0] = 0;
                                                                  }

                                                                  SetStrValue(fr,DRAWERGAD,fr->fr_DrawerString);
                                                                  ActivateGad(fr,DRAWERGAD,quals);
                                                                  getDir = TRUE;
                                                                  break;

                                              case FR_FILE      : if ((icode == CURSORUP) || (icode == CURSORDOWN))
                                                                  {
								      CursorSelect(fr,gadget,icode,quals);
                                                                  }
                                                                  else
                                                                  {
                                                                      ptr = FilePart(fr->fr_FileString);
                                                                      if (ptr == fr->fr_FileString)
                                                                      {
                                                                          if ((FRF_DOPATTERNS & fr->fr_Options)
                                                                          &&  (FixedParsePattern(fr->fr_FileString,store->rs_ParsedPattern,sizeof(store->rs_ParsedPattern)) > 0))
                                                                          {
                                                                              strcpy(fr->fr_PatternString,fr->fr_FileString);
                                                                              SetStrValue(fr,PATTERNGAD,fr->fr_PatternString);
                                                                              SetStrValue(fr,FILEGAD,"");
                                                                              getDir = TRUE;
                                                                          }
                                                                          else if ((quals & ALTKEYS) || icode == 9)
                                                                          {
                                                                              ActivateGad(fr,FILEGAD,quals);
                                                                          }
                                                                          else if (!(quals & SHIFTKEYS))
                                                                          {
                                                                              return(QuitFR(fr,FALSE,lock));
                                                                          }
                                                                      }
                                                                      else
                                                                      {
                                                                          new = FALSE;
                                                                          if (stpchr(fr->fr_FileString,':'))
                                                                          {
                                                                              fr->fr_DrawerString[0] = 0;
                                                                              new = TRUE;
                                                                          }

                                                                          if (ptr != fr->fr_FileString)
                                                                          {
                                                                              AddPart(fr->fr_DrawerString,fr->fr_FileString,sizeof(fr->fr_DrawerString));
                                                                              strcpy(fr->fr_FileString,ptr);
                                                                              ptr    = FilePart(fr->fr_DrawerString);
                                                                              ptr[0] = 0;
                                                                              new    = TRUE;
                                                                          }

                                                                          if (new)
                                                                          {
                                                                              ReducePath(fr->fr_DrawerString);
                                                                              SetStrValue(fr,DRAWERGAD,fr->fr_DrawerString);
                                                                          }

                                                                          if ((FRF_DOPATTERNS & fr->fr_Options)
                                                                          &&  (FixedParsePattern(fr->fr_FileString,store->rs_ParsedPattern,sizeof(store->rs_ParsedPattern)) > 0))
                                                                          {
                                                                              SetStrValue(fr,PATTERNGAD,fr->fr_FileString);
                                                                              strcpy(fr->fr_PatternString,fr->fr_FileString);
                                                                              SetStrValue(fr,FILEGAD,"");
                                                                              getDir = TRUE;
                                                                          }
                                                                          else
                                                                          {
                                                                              SetStrValue(fr,FILEGAD,fr->fr_FileString);
                                                                              if (new)
                                                                              {
                                                                                  getDir = TRUE;
                                                                              }
                                                                          }
                                                                      }

                                                                      if (getDir)
                                                                          ActivateGadget(gadget,window,NULL);
                                                                      else
                                                                          ActivateGad(fr,FILEGAD,quals);
                                                                      ParsePat(fr);
                                                                  }
                                                                  break;

                                              default           : break;
                                          }
                                          break;

                case IDCMP_NEWSIZE      : if (!CreateFRGadgets(fr))
                                              return(QuitFR(fr,TRUE,lock));
                                          newsize = TRUE;
                                          InsertList(fr);
                                          break;

                case IDCMP_INTUITICKS   : tickCnt++;
                                          if (fr->fr_ZoomOnFileSig >= 0)
                                          {
                                              if (SetSignal(0,1L << fr->fr_ZoomOnFileSig) & (1L << fr->fr_ZoomOnFileSig))
                                              {
                                                  InsertList(fr);

                                                  if (!IsListEmpty(&fr->fr_FileList))
                                                  {
                                                      if (!(node = FindClosest(&fr->fr_FileList,fr->fr_FileString)))
                                                          node = fr->fr_FileList.lh_TailPred;

                                                      fr->fr_TopItem = FindNodeNum(&fr->fr_FileList,node);
                                                      LimitTopItem(fr);

                                                      SetGadgetAttr(ri,SCROLLERGAD,GTSC_Top, fr->fr_TopItem,
                                                                                   TAG_DONE);
                                                      RenderLV(fr);
                                                  }
                                              }
                                          }
                                          break;

                case IDCMP_RAWKEY       : if ((icode == CURSORUP) || (icode == CURSORDOWN))
                			      CursorSelect(fr,NULL,icode,quals);
                			  else if (icode & IECODE_UP_PREFIX)
                			      ActivateMajorGuy(fr);
                			  break;

                case IDCMP_DISKINSERTED :
                case IDCMP_DISKREMOVED  : getDir     = volumes;
                                          getVolumes = volumes;
                                          break;
            }
        }
    }
}


/*****************************************************************************/


static VOID ProcessFRTags(struct ExtFileReq *fr, struct TagItem *tagList)
{
struct TagItem *tag;
struct TagItem *tags = tagList;
ULONG           data;

    while (tag = NextTagItem(&tags))
    {
        data = tag->ti_Data;
        switch(tag->ti_Tag)
        {
            case ASLFR_InitialFile     : strncpy(fr->fr_FileString,(STRPTR)data,sizeof(fr->fr_FileString));
                                         break;

            case ASLFR_InitialDrawer   : strncpy(fr->fr_DrawerString,(STRPTR)data,sizeof(fr->fr_DrawerString));
                                         break;

            case ASLFR_InitialPattern  : strncpy(fr->fr_PatternString,(STRPTR)data,sizeof(fr->fr_PatternString));
                                         break;

            case ASLFR_RejectPattern   : fr->fr_RejectPattern = (APTR)data;
                                         break;

            case ASLFR_AcceptPattern   : fr->fr_AcceptPattern = (APTR)data;
                                         break;

            case ASLFR_HookFunc        : fr->fr_ReqInfo.ri_OldHook = (APTR)data;
                                         break;

            case ASLFR_FilterFunc      : fr->fr_FilterFunc = (struct Hook *)data;
                                         break;

            case ASLFR_Flags1          : fr->fr_Options                 = (ULONG)data;
                                         fr->fr_ReqInfo.ri_CallOldHook  = (BOOL)(data & FRF_INTUIFUNC);
                                         fr->fr_CallOldFilter           = (BOOL)(data & FRF_FILTERFUNC);
                                         fr->fr_ReqInfo.ri_PrivateIDCMP = (BOOL)(data & FRF_PRIVATEIDCMP);
                                         break;

            case ASLFR_Flags2          : fr->fr_Options2 = (ULONG)data;
                                         break;

            case ASLFR_UserData        : fr->fr_UserData = (APTR)data;
                                         break;

            default                    : ProcessStdTag(&fr->fr_ReqInfo,tag);
                                         break;
        }
    }

    fr->fr_Options = AslPackBoolTags(fr->fr_Options,tagList,
                                     ASLFR_DoPatterns,    FRF_DOPATTERNS,
                                     ASLFR_DoMultiSelect, FRF_DOMULTISELECT,
                                     ASLFR_DoSaveMode,    FRF_DOSAVEMODE,
                                     TAG_DONE);

    fr->fr_Options2 = AslPackBoolTags(fr->fr_Options2,tagList,
                                      ASLFR_DrawersOnly,   FRF_DRAWERSONLY,
                                      ASLFR_FilterDrawers, FRF_FILTERDRAWERS,
                                      ASLFR_RejectIcons,   FRF_REJECTICONS,
                                      TAG_DONE);
}



/*****************************************************************************/


#undef SysBase

static ULONG ASM EditHook(REG(a0) struct Hook *hook,
                          REG(a2) struct SGWork *sgw,
                          REG(a1) ULONG *msg)
{
struct ExtFileReq *fr = hook->h_Data;
struct Library    *SysBase = (*((struct Library **) 4));

    if (*msg != SGH_KEY)
        return(0);

    if (sgw->IEvent->ie_Qualifier & IEQUALIFIER_RCOMMAND)
    {
        switch (sgw->IEvent->ie_Code)
        {
            case 0x67     :
            case KEYCODE_Q:
            case KEYCODE_Z:
            case KEYCODE_X: if (fr->fr_ZoomOnFileSig >= 0)
                                Signal(fr->fr_ReqInfo.ri_Window->UserPort->mp_SigTask,1L << fr->fr_ZoomOnFileSig);
                            break;

            default       : sgw->Actions = SGA_REUSE | SGA_END;
                            sgw->Code    = -1;
                            break;
        }
    }
    else if ((sgw->IEvent->ie_Code == CURSORUP)
         ||  (sgw->IEvent->ie_Code == CURSORDOWN))
    {
        sgw->Actions |= SGA_END;
        sgw->Code     = sgw->IEvent->ie_Code;
    }
    else if (sgw->EditOp != EO_NOOP)
    {
        if (fr->fr_ZoomOnFileSig >= 0)
            Signal(fr->fr_ReqInfo.ri_Window->UserPort->mp_SigTask,1L << fr->fr_ZoomOnFileSig);
    }

    return (1);
}

#define SysBase AslBase->ASL_SysBase


/*****************************************************************************/


struct ExtFileReq *AllocFileRequest(struct TagItem *tagList)
{
struct ExtFileReq *fr;

    if (fr = AllocVec(sizeof(struct ExtFileReq),MEMF_CLEAR|MEMF_ANY))
    {
	NewList(&fr->fr_FileList);
	NewList((struct List *)&fr->fr_TempFileList);

        fr->fr_ReqInfo.ri_Coords.Left   = 30;
        fr->fr_ReqInfo.ri_Coords.Top    = 20;
        fr->fr_ReqInfo.ri_Coords.Width  = 318;
        fr->fr_ReqInfo.ri_Coords.Height = 178;

        fr->fr_ReqInfo.ri_TitleID     = MSG_ASL_FR_TITLE;
        fr->fr_ReqInfo.ri_IDCMPFlags  = IDCMP_MENUPICK | IDCMP_REFRESHWINDOW | IDCMP_CLOSEWINDOW | IDCMP_NEWSIZE | IDCMP_RAWKEY | IDCMP_ACTIVEWINDOW | IDCMP_INTUITICKS | IDCMP_MOUSEBUTTONS | IDCMP_DISKINSERTED | IDCMP_DISKREMOVED | SCROLLERIDCMP | BUTTONIDCMP;
        fr->fr_ReqInfo.ri_WindowFlags = WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET | WFLG_SIZEGADGET | WFLG_SIMPLE_REFRESH | WFLG_ACTIVATE | WFLG_SIZEBBOTTOM;

        fr->fr_Pattern = fr->fr_PatternString;
        fr->fr_Drawer  = fr->fr_DrawerString;
        fr->fr_File    = fr->fr_FileString;

        strcpy(fr->fr_PatternString,"#?");

        ProcessFRTags(fr,tagList);

        fr->fr_Coords = fr->fr_ReqInfo.ri_Coords;

	fr->fr_FileHook.h_Entry = EditHook;
	fr->fr_FileHook.h_Data  = fr;
    }

    return(fr);
}


/*****************************************************************************/


APTR FileRequest(struct ExtFileReq *fr, struct TagItem *tagList)
{
struct ReqInfo *ri     = &fr->fr_ReqInfo;
struct ASLLib  *asl    = AslBase;
LONG            error  = ERROR_NO_FREE_STORE;
BOOL            result = FALSE;
struct Process *process;
struct Message *msg;
struct FRStore *store;

    FreeMultiArgs(fr);
    if (store = fr->fr_Store = PrepareRequester(ri,AG,sizeof(AG),sizeof(struct FRStore)))
    {
        ProcessFRTags(fr,tagList);
        PrepareLocale(&fr->fr_ReqInfo);

        /* Do requester specific layout */
        if (!(FRF_DOPATTERNS & fr->fr_Options))
        {
            ri->ri_Template[PATTERNGAD].ag_LOGroup = 0;
        }

        if (FRF_DRAWERSONLY & fr->fr_Options2)
        {
            ri->ri_Template[FILEGAD].ag_LOGroup = 0;
        }

        FRPreserve(fr);

        process                         = (struct Process *)FindTask(NULL);
        store->rs_PacketPort.mp_Flags   = PA_SIGNAL;
        store->rs_PacketPort.mp_SigTask = (struct Task *)process;
        store->rs_PacketPort.mp_SigBit  = process->pr_MsgPort.mp_SigBit;
        NewList(&store->rs_PacketPort.mp_MsgList);

        store->rs_AppPort.mp_Flags   = PA_SIGNAL;
        store->rs_AppPort.mp_SigTask = (struct Task *)process;
        store->rs_AppPort.mp_SigBit  = process->pr_MsgPort.mp_SigBit;
        NewList(&store->rs_AppPort.mp_MsgList);

        store->rs_StdPkt.sp_Msg.mn_Node.ln_Name = (STRPTR)&store->rs_StdPkt.sp_Pkt;
        store->rs_StdPkt.sp_Pkt.dp_Link         = &store->rs_StdPkt.sp_Msg;
        store->rs_AP.ap_Current                 = &store->rs_AC;

        if (OpenRequester(ri,AM))
        {
            fr->fr_ZoomOnFileSig = AllocSignal(-1);

            if (asl->ASL_WorkbenchBase = OpenLibrary("workbench.library",37))
                fr->fr_AppWindow = AddAppWindowA(0,0,ri->ri_Window,&store->rs_AppPort,NULL);

            if (CreateFRGadgets(fr))
            {
                error = 0;

                InitFileList(fr);

                RenderFRDisplay(fr);

                result = HandleFRIDCMP(fr);

                InitFileList(fr);
            }

            if (fr->fr_AppWindow)
            {
                RemoveAppWindow(fr->fr_AppWindow);
                fr->fr_AppWindow = NULL;

                while (msg = GetMsg(&store->rs_AppPort))
                    ReplyMsg(msg);
            }
            CloseLibrary(asl->ASL_WorkbenchBase);

            if (fr->fr_ZoomOnFileSig >= 0)
                FreeSignal(fr->fr_ZoomOnFileSig);

            CloseRequester(ri);
        }

        fr->fr_Coords = ri->ri_Coords;

        if (!result)
            FRRestore(fr);

        CleanupRequester(ri,store);
    }

    SetIoErr(error);

    /* kludge to keep old sw running. Turns out old versions of ASL were
     * returning the file name in D0 instead of a BOOL. We keep doing this...
     */
    if (result)
        return(fr->fr_File);

    return(NULL);
}


/*****************************************************************************/


VOID FreeFileRequest(struct ExtFileReq *fr)
{
    FreeMultiArgs(fr);
    FreeVec(fr);
}
