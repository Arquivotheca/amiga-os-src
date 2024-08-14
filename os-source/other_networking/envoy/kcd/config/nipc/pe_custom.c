
/* includes */
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <devices/keymap.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <intuition/gadgetclass.h>
#include <graphics/text.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <libraries/locale.h>
#include <dos/dos.h>
#include <dos/exall.h>
#include <string.h>
#include <stdio.h>

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_protos.h>
#include <clib/utility_protos.h>
#include <clib/asl_protos.h>
#include <clib/icon_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/icon_pragmas.h>

/* application includes */
#include "pe_custom.h"
#include "pe_strings.h"
#include "pe_utils.h"
#include "pe_iff.h"
#include <devices/sana2.h>

#define SysBase ed->ed_SysBase


/*****************************************************************************/


/* The IFF chunks known to this prefs editor. IFFPrefChunkCnt says how many
 * chunks there are
 */
#define IFFPrefChunkCnt 6
static LONG far IFFPrefChunks[] =
{
    ID_PREF, ID_PRHD,
    ID_PREF, ID_NDEV,
    ID_PREF, ID_NRRM,
    ID_PREF, ID_NLRM,
    ID_PREF, ID_NIRT,
    ID_PREF, ID_HOST
};

BOOL DummyBuffFunc(APTR to, APTR from, LONG length);

struct TagItem BMTags[] =
{
    S2_CopyToBuff,	DummyBuffFunc,
    S2_CopyFromBuff,	DummyBuffFunc,
    TAG_DONE,		0
};

/*****************************************************************************/


/* The PrefHeader structure this editor outputs */
static struct PrefHeader far IFFPrefHeader =
{
    0,   /* version */
    0,   /* type    */
    0    /* flags   */
};


/*****************************************************************************/


#define WRITE_ALL    0
#define WRITE_WB     1
#define WRITE_SYS    2
#define WRITE_SCREEN 3


/*****************************************************************************/


EdStatus InitEdData(EdDataPtr ed)
{
    InitPrefs(&ed->ed_PrefsDefaults);
    InitPrefs(&ed->ed_PrefsWork);
    InitPrefs(&ed->ed_PrefsInitial);
    ed->ed_CurrentDevice = NULL;
    ed->ed_CurrentRoute = NULL;
    ed->ed_CurrentLocRealm = NULL;
    ed->ed_CurrentRemRealm = NULL;
    ed->ed_IPGadHook.h_Entry = StringHook;
    ed->ed_V39 = (ed->ed_IntuitionBase->lib_Version >= 39) ? TRUE : FALSE;
    return(ES_NORMAL);
}

void InitPrefs(struct ExtPrefs *prefs)
{
    NewList(&prefs->ep_NIPCDevices);
    NewList(&prefs->ep_NIPCRoutes);
    NewList(&prefs->ep_NIPCLocalRealms);
    NewList(&prefs->ep_NIPCRemoteRealms);
}

/*****************************************************************************/


VOID CleanUpEdData(EdDataPtr ed)
{
    FreePrefs(ed, &ed->ed_PrefsDefaults);
    FreePrefs(ed, &ed->ed_PrefsWork);
    FreePrefs(ed, &ed->ed_PrefsInitial);
}

VOID FreePrefs(EdDataPtr ed, struct ExtPrefs *prefs)
{
    FreeList(ed, &prefs->ep_NIPCDevices,sizeof(struct NIPCDevice));
    FreeList(ed, &prefs->ep_NIPCLocalRealms,sizeof(struct NIPCRealm));
    FreeList(ed, &prefs->ep_NIPCRemoteRealms,sizeof(struct NIPCRealm));
    FreeList(ed, &prefs->ep_NIPCRoutes,sizeof(struct NIPCRoute));
}

VOID FreeList(EdDataPtr ed, struct List *list, ULONG nodeSize)
{
struct Node *node;

    while(node = RemHead(list))
    {
        FreeMem(node, nodeSize);
    }
}

VOID InsertRealmSorted (EdDataPtr ed, struct List *list, struct NIPCRealm *node)
{
	struct Node *n;

	for (n = list->lh_TailPred; n->ln_Pred; n = n->ln_Pred)
	{
		if(Stricmp(((struct NIPCRealm *)n)->nz_Prefs.nzp_RealmName,node->nz_Prefs.nzp_RealmName) <= 0)
		{
			Insert(list,(struct Node *)node,n);	// we go backwards since Insert inserts after the node
			return;
		}
	}
	AddHead(list,(struct Node *)node);
}

/*****************************************************************************/


EdStatus ReadPrefs(EdDataPtr ed, struct IFFHandle *iff, struct ContextNode *cn)
{
struct NIPCDevicePrefs tmp_dev;
struct NIPCDevice *nd;
struct NIPCRoutePrefs tmp_route;
struct NIPCRoute *nr;
struct NIPCRealmPrefs tmp_realm;
struct NIPCRealm *nz;
struct NIPCHostPrefs tmp_host;
ULONG hostchunksize;
ULONG checkflags;

    tmp_host.nhp_HostFlags = 0;
    tmp_host.nhp_OwnerName[0]='\0';

    if (cn->cn_Type != ID_PREF)
        return(ES_IFF_UNKNOWNCHUNK);

    switch(cn->cn_ID)
    {
        case ID_NDEV:
                        if(ReadChunkBytes(iff,&tmp_dev,sizeof(struct NIPCDevicePrefs)) == sizeof(struct NIPCDevicePrefs))
                        {
                            if(nd = AllocMem(sizeof(struct NIPCDevice),MEMF_CLEAR|MEMF_PUBLIC))
                            {
                                CopyMem((APTR)&tmp_dev,(APTR)&nd->nd_Prefs,sizeof(struct NIPCDevicePrefs));
                                AddTail(&ed->ed_PrefsWork.ep_NIPCDevices,(struct Node *)nd);
                                nd->nd_Node.ln_Name = (STRPTR)&nd->nd_Prefs.ndp_DevPathName;
                                return(ES_NORMAL);
                            }
                            return(ES_NO_MEMORY);
                        }
                        else
                            return(ES_IFFERROR);
                        break;
        case ID_NIRT:
                        if(ReadChunkBytes(iff,&tmp_route,sizeof(struct NIPCRoutePrefs)) == sizeof(struct NIPCRoutePrefs))
                        {
                            if(nr = AllocMem(sizeof(struct NIPCRoute),MEMF_CLEAR|MEMF_PUBLIC))
                            {
                                CopyMem((APTR)&tmp_route,(APTR)&nr->nr_Prefs,sizeof(struct NIPCRoutePrefs));
                                AddTail(&ed->ed_PrefsWork.ep_NIPCRoutes,(struct Node *)nr);
                                nr->nr_Node.ln_Name = (STRPTR)&nr->nr_String;
                                return(ES_NORMAL);
                            }
                            return(ES_NO_MEMORY);
                        }
                        else
                            return(ES_IFFERROR);
                        break;
        case ID_NRRM:
        case ID_NLRM:
                        if(ReadChunkBytes(iff,&tmp_realm,sizeof(struct NIPCRealmPrefs)) == sizeof(struct NIPCRealmPrefs))
                        {
                            if(nz = AllocMem(sizeof(struct NIPCRealm),MEMF_CLEAR|MEMF_PUBLIC))
                            {
                                ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_HostFlags |= (NHPFLAGF_REALMS|NHPFLAGF_REALMSERVER);
                                CopyMem((APTR)&tmp_realm,(APTR)&nz->nz_Prefs,sizeof(struct NIPCRealmPrefs));
                                nz->nz_Node.ln_Name = (STRPTR)&nz->nz_String;
                                if(cn->cn_ID == ID_NRRM)
                                    InsertRealmSorted(ed,&ed->ed_PrefsWork.ep_NIPCRemoteRealms,nz);
                                else
                                    InsertRealmSorted(ed,&ed->ed_PrefsWork.ep_NIPCLocalRealms,nz);
                                return(ES_NORMAL);
                            }
                            return(ES_NO_MEMORY);
                        }
                        else
                            return(ES_IFFERROR);
                        break;

        case ID_HOST:	hostchunksize = sizeof(struct NIPCHostPrefs);
        		if(cn->cn_Size < hostchunksize)
        		{
        		    checkflags = TRUE;
        		    hostchunksize = cn->cn_Size;
			}

                        if(ReadChunkBytes(iff,&tmp_host,hostchunksize) == hostchunksize)
                        {
                            CopyMem((APTR)&tmp_host,&ed->ed_PrefsWork.ep_NIPCHostPrefs,sizeof(struct NIPCHostPrefs));
                            if(checkflags && ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_RealmServAddr)
                                ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_HostFlags |= NHPFLAGF_REALMS;

                            return(ES_NORMAL);
                        }
                        else
                            return(ES_IFFERROR);
                        break;
        default:
                        return(ES_IFFERROR);
    }
}

EdStatus OpenPrefs(EdDataPtr ed, STRPTR name)
{
    FreePrefs(ed, &ed->ed_PrefsDefaults);
    FreePrefs(ed, &ed->ed_PrefsWork);
    FreePrefs(ed, &ed->ed_PrefsInitial);
    return(ReadIFF(ed,name,IFFPrefChunks,IFFPrefChunkCnt,ReadPrefs));
}


/*****************************************************************************/


EdStatus WritePrefs(EdDataPtr ed, struct IFFHandle *iff, struct ContextNode *cn)
{
    struct NIPCDevice *nd;
    struct NIPCRoute  *nr;
    struct NIPCRealm  *nz;

    if ((ed->ed_Write == WRITE_ALL))
    {
        if(!PushChunk(iff,0,ID_HOST,sizeof(struct NIPCHostPrefs)))
        {
            if(WriteChunkBytes(iff,&ed->ed_PrefsWork.ep_NIPCHostPrefs,sizeof(struct NIPCHostPrefs)) == sizeof(struct NIPCHostPrefs))
            {
                PopChunk(iff);
            }
        }
        nd = (struct NIPCDevice *)ed->ed_PrefsWork.ep_NIPCDevices.lh_Head;
        while(nd->nd_Node.ln_Succ)
        {
            if(Stricmp("(new)",(STRPTR)&nd->nd_Prefs.ndp_DevPathName))
            {
                if(!PushChunk(iff,0,ID_NDEV,sizeof(struct NIPCDevicePrefs)))
                {
                    if(WriteChunkBytes(iff,&nd->nd_Prefs,sizeof(struct NIPCDevicePrefs)) == sizeof(struct NIPCDevicePrefs))
                    {
                        PopChunk(iff);
                    }
                }
            }
            nd = (struct NIPCDevice *)nd->nd_Node.ln_Succ;
        }
        nr = (struct NIPCRoute *)ed->ed_PrefsWork.ep_NIPCRoutes.lh_Head;
        while(nr->nr_Node.ln_Succ)
        {
            if(Stricmp("(new)",(STRPTR)nd->nd_Node.ln_Name))
            {
                if(!PushChunk(iff,0,ID_NIRT,sizeof(struct NIPCRoutePrefs)))
                {
                    if(WriteChunkBytes(iff,&nr->nr_Prefs,sizeof(struct NIPCRoutePrefs)) == sizeof(struct NIPCRoutePrefs))
                    {
                        PopChunk(iff);
                    }
                }
            }
            nr = (struct NIPCRoute *)nr->nr_Node.ln_Succ;
        }
        nz = (struct NIPCRealm *)ed->ed_PrefsWork.ep_NIPCLocalRealms.lh_Head;
        while(nz->nz_Node.ln_Succ)
        {
            if(Stricmp("(new)",(STRPTR)nd->nd_Node.ln_Name))
            {
                if(!PushChunk(iff,0,ID_NLRM,sizeof(struct NIPCRealmPrefs)))
                {
                    if(WriteChunkBytes(iff,&nz->nz_Prefs,sizeof(struct NIPCRealmPrefs)) == sizeof(struct NIPCRealmPrefs))
                    {
                        PopChunk(iff);
                    }
                }
            }
            nz = (struct NIPCRealm *)nz->nz_Node.ln_Succ;
        }
        nz = (struct NIPCRealm *)ed->ed_PrefsWork.ep_NIPCRemoteRealms.lh_Head;
        while(nz->nz_Node.ln_Succ)
        {
            if(Stricmp("(new)",(STRPTR)nd->nd_Node.ln_Name))
            {
                if(!PushChunk(iff,0,ID_NRRM,sizeof(struct NIPCRealmPrefs)))
                {
                    if(WriteChunkBytes(iff,&nz->nz_Prefs,sizeof(struct NIPCRealmPrefs)) == sizeof(struct NIPCRealmPrefs))
                    {
                        PopChunk(iff);
                    }
                }
            }
            nz = (struct NIPCRealm *)nz->nz_Node.ln_Succ;
        }

    }
    return(ES_NORMAL);
}


EdStatus SavePrefs(EdDataPtr ed, STRPTR name)
{
    return(WriteIFF(ed,name,&IFFPrefHeader,WritePrefs));
}


/*****************************************************************************/


#define NW_WIDTH     600
#define NW_HEIGHT    180
#define NW_IDCMP     (IDCMP_MOUSEBUTTONS | IDCMP_MENUPICK | IDCMP_REFRESHWINDOW | IDCMP_INTUITICKS | BUTTONIDCMP | CHECKBOXIDCMP | SLIDERIDCMP | CYCLEIDCMP | TEXTIDCMP | LISTVIEWIDCMP)
#define NW_FLAGS     (WFLG_ACTIVATE | WFLG_DEPTHGADGET | WFLG_DRAGBAR | WFLG_SIMPLE_REFRESH)
#define NW_MINWIDTH  NW_WIDTH
#define NW_MINHEIGHT NW_HEIGHT
#define NW_MAXWIDTH  NW_WIDTH
#define NW_MAXHEIGHT NW_HEIGHT
#define ZOOMWIDTH    200

struct EdMenu far EM[] = {
    {NM_TITLE,  MSG_PROJECT_MENU,           EC_NOP, 0},
      {NM_ITEM, MSG_PROJECT_OPEN,           EC_OPEN, 0},
      {NM_ITEM, MSG_PROJECT_SAVE_AS,        EC_SAVEAS, 0},
      {NM_ITEM, MSG_NOTHING,                EC_NOP, 0},
      {NM_ITEM, MSG_PROJECT_QUIT,           EC_CANCEL, 0},

    {NM_TITLE,  MSG_EDIT_MENU,              EC_NOP, 0},
      {NM_ITEM, MSG_EDIT_RESET_TO_DEFAULTS, EC_RESETALL, 0},
      {NM_ITEM, MSG_EDIT_LAST_SAVED,        EC_LASTSAVED, 0},
      {NM_ITEM, MSG_EDIT_RESTORE,           EC_RESTORE, 0},

    {NM_TITLE,  MSG_OPTIONS_MENU,           EC_NOP, 0},
      {NM_ITEM, MSG_OPTIONS_SAVE_ICONS,     EC_SAVEICONS, CHECKIT|MENUTOGGLE},

    {NM_END,    MSG_NOTHING,                EC_NOP, 0}
};

/* main display gadgets */
struct EdGadget far EG[] = {

    /* Main Panel */

    {BUTTON_KIND,   10,  160,  152, 14, MSG_SAVE_GAD,         EC_SAVE,            0},
    {BUTTON_KIND,   224, 160,  152, 14, MSG_USE_GAD,          EC_USE,             0},
    {BUTTON_KIND,   444, 160,  152, 14, MSG_CANCEL_GAD,       EC_CANCEL,          0},
    {BUTTON_KIND,   402, 26,   188, 24, 0,                    EC_MAIN_ROUTES,     0},
    {BUTTON_KIND,   402, 66,   188, 24, 0,                    EC_MAIN_REALMS,     0},
    {BUTTON_KIND,   402, 106,  188, 24, 0,                    EC_MAIN_DEVICES,    0},

    {STRING_KIND,   156, 26,   164, 14, MSG_HOST_NAME,        EC_MAIN_HOSTNAME,   0},
    {STRING_KIND,   156, 80,   164, 14, MSG_REALM_NAME,       EC_MAIN_REALMNAME,  0},
    {STRING_KIND,   156, 96,   164, 14, MSG_REALM_ADDR,       EC_MAIN_REALMADDR,  0},

    {CYCLE_KIND,    10,  4,    200, 14, 0,                    EC_PANEL,           0},

    /* Realms Panel */

    {BUTTON_KIND,   444, 160,  152, 14, 0,                    EC_REALM_CANCEL,    0},
    {BUTTON_KIND,   12,  142,  132, 14, MSG_ADD_REALM,        EC_LOCREALM_ADD,    0},
    {BUTTON_KIND,   148, 142,  132, 14, MSG_DEL_REALM,        EC_LOCREALM_DELETE, 0},
    {BUTTON_KIND,   310, 142,  132, 14, MSG_ADD_REALM,        EC_REMREALM_ADD,    0},
    {BUTTON_KIND,   446, 142,  132, 14, MSG_DEL_REALM,        EC_REMREALM_DELETE, 0},

    {TEXT_KIND,     10,  22,   272, 12, MSG_LOC_TXT,          0,                  PLACETEXT_IN | NG_HIGHLABEL},
    {TEXT_KIND,     308, 22,   272, 12, MSG_REM_TXT,          0,                  PLACETEXT_IN | NG_HIGHLABEL},

    {STRING_KIND,   10,  126,  136, 14, 0,                    EC_LOCREALM_NAME,   0},
    {STRING_KIND,   146, 126,  136, 14, 0,                    EC_LOCREALM_NET,    0},
    {STRING_KIND,   308, 126,  136, 14, 0,                    EC_REMREALM_NAME,   0},
    {STRING_KIND,   444, 126,  136, 14, 0,                    EC_REMREALM_NET,   0},

    {LISTVIEW_KIND, 10,  50,   290, 76, MSG_LOC_REALMS,       EC_LOCREALM_LIST,   PLACETEXT_ABOVE},
    {LISTVIEW_KIND, 308, 50,   290, 76, MSG_REM_REALMS,       EC_REMREALM_LIST,   PLACETEXT_ABOVE},

    /* Routes Panel */

    {BUTTON_KIND,   10,  160,  152, 14, 0,                    EC_ROUTE_ACCEPT,    0},
    {BUTTON_KIND,   444, 160,  152, 14, 0,                    EC_ROUTE_CANCEL,    0},
    {BUTTON_KIND,   146, 130,  168, 14, MSG_ADD_ROUTE,        EC_ROUTE_ADD,       0},
    {BUTTON_KIND,   314, 130,  168, 14, MSG_DEL_ROUTE,        EC_ROUTE_DELETE,    0},

    {STRING_KIND,   146, 116,  168, 14, 0,                    EC_ROUTE_DEST,      0},
    {STRING_KIND,   314, 116,  168, 14, 0,                    EC_ROUTE_GATEWAY,   0},

    {TEXT_KIND,	    146, 26,   168, 14, MSG_ROUTE_DEST,	      0,		  PLACETEXT_IN},
    {LISTVIEW_KIND, 146, 40,   336, 76, 0,		      EC_ROUTE_LIST,      0},

    /* Devices Panel */

    {BUTTON_KIND,   10,  160,  152, 14, 0,                    EC_DEVICE_ACCEPT,    0},
    {BUTTON_KIND,   444, 160,  152, 14, 0,                    EC_DEVICE_CANCEL,    0},
    {BUTTON_KIND,   10,  126,  110, 14, MSG_DEVICE_ADD,       EC_DEVICE_ADD,       0},
    {BUTTON_KIND,   120, 126,  110, 14, MSG_DEVICE_DEL,       EC_DEVICE_DEL,       0},

    {TEXT_KIND,     240, 8,    356, 16, MSG_DEVICE_TXT,       0,                   PLACETEXT_IN | NG_HIGHLABEL},

    {CYCLE_KIND,    364, 120,  110, 14, MSG_DEVICE_STATUS,    EC_DEVICE_STATUS,    0},

    {CHECKBOX_KIND, 476, 42,   26,  11, MSG_DEVICE_DEFAULT,   EC_DEVICE_MASK_ABLE, PLACETEXT_RIGHT},
    {CHECKBOX_KIND, 476, 58,   26,  11, MSG_DEVICE_DEFAULT,   EC_DEVICE_ADDR_ABLE, PLACETEXT_RIGHT},
    {CHECKBOX_KIND, 476, 74,   26,  11, MSG_DEVICE_DEFAULT,   EC_DEVICE_IPTYPE_ABLE,  PLACETEXT_RIGHT},
    {CHECKBOX_KIND, 476, 90,   26,  11, MSG_DEVICE_DEFAULT,   EC_DEVICE_ARPTYPE_ABLE, PLACETEXT_RIGHT},

    {STRING_KIND,   10,  100,  220, 14, 0,                    EC_DEVICE_NAME,      0},
    {INTEGER_KIND,  402, 104,  72,  14, MSG_DEVICE_UNIT,      EC_DEVICE_UNIT,      0},
    {STRING_KIND,   338, 24,   136, 14, MSG_DEVICE_IP,        EC_DEVICE_IP,        0},
    {STRING_KIND,   338, 40,   136, 14, MSG_DEVICE_MASK,      EC_DEVICE_MASK,      0},
    {STRING_KIND,   338, 56,   136, 14, MSG_DEVICE_ADDR,      EC_DEVICE_ADDR,      0},
    {INTEGER_KIND,  402, 72,   72,  14, MSG_DEVICE_IPTYPE,    EC_DEVICE_IPTYPE,    0},
    {INTEGER_KIND,  402, 88,   72,  14, MSG_DEVICE_ARPTYPE,   EC_DEVICE_ARPTYPE,   0},

    {LISTVIEW_KIND, 10,  24,   220, 116,0,                    EC_DEVICE_LIST,      PLACETEXT_ABOVE},
    {LISTVIEW_KIND, 236, 104,  360, 52, MSG_DEVICE_INFO,      0,                   PLACETEXT_ABOVE},

    /* New Main Panel Gadgets */

    {STRING_KIND,   156, 42,   164, 14, MSG_OWNER_NAME,	      EC_MAIN_OWNERNAME,   0},
    {CHECKBOX_KIND, 564, 26,   26,  11, MSG_USE_REALMSERVER,  EC_MAIN_REALMS,	   0},
    {CHECKBOX_KIND, 564, 42,  26,  11, MSG_IS_REALMSERVER,   EC_MAIN_REALMSERVER, 0},

    {TEXT_KIND,	    314, 26,   168, 14, MSG_ROUTE_GATE,	      0,		  PLACETEXT_IN},

};

/*****************************************************************************/


BOOL CreateDisplay(EdDataPtr ed)
{
UWORD zoomSize[4];

    zoomSize[0] = -1;
    zoomSize[1] = -1;
    zoomSize[2] = ZOOMWIDTH;
    zoomSize[3] = ed->ed_Screen->WBorTop + ed->ed_Screen->Font->ta_YSize + 1;

    ed->ed_StatusLabels[0] = GetString(&ed->ed_LocaleInfo,MSG_DEVICE_STATUSLABEL0);
    ed->ed_StatusLabels[1] = GetString(&ed->ed_LocaleInfo,MSG_DEVICE_STATUSLABEL1);

    ed->ed_PanelLabels[0] = GetString(&ed->ed_LocaleInfo,MSG_PANEL_MAIN);
    ed->ed_PanelLabels[1] = GetString(&ed->ed_LocaleInfo,MSG_PANEL_DEVICES);
    ed->ed_PanelLabels[2] = GetString(&ed->ed_LocaleInfo,MSG_PANEL_ROUTES);
    ed->ed_PanelLabels[3] = GetString(&ed->ed_LocaleInfo,MSG_PANEL_REALMS);

    ed->ed_LastAdded = NULL; /* CreateContext(&ed->ed_Gadgets); */

    RenderGadgets(ed);

    if ((ed->ed_LastAdded)
    &&  (ed->ed_Menus = CreatePrefsMenus(ed,EM))
    &&  (ed->ed_Window = OpenPrefsWindow(ed,WA_InnerWidth,  NW_WIDTH,
                                            WA_InnerHeight, NW_HEIGHT,
                                            WA_MinWidth,    NW_MINWIDTH,
                                            WA_MinHeight,   NW_MINHEIGHT,
                                            WA_MaxWidth,    NW_MAXWIDTH,
                                            WA_MaxHeight,   NW_MAXHEIGHT,
                                            WA_IDCMP,       NW_IDCMP,
                                            WA_Flags,       NW_FLAGS,
                                            WA_Zoom,        zoomSize,
                                            WA_AutoAdjust,  TRUE,
                                            WA_PubScreen,   ed->ed_Screen,
                                            WA_Title,       GetString(&ed->ed_LocaleInfo,MSG_NDEV_NAME),
                                            WA_NewLookMenus,TRUE,
                                            WA_Gadgets,     ed->ed_Gadgets,
                                            TAG_DONE)))
    {
        return(TRUE);
    }

    DisposeDisplay(ed);
    return(FALSE);
}


/*****************************************************************************/


VOID DisposeDisplay(EdDataPtr ed)
{
    if (ed->ed_Window)
    {
        ClearMenuStrip(ed->ed_Window);
        CloseWindow(ed->ed_Window);
    }
    FreeMenus(ed->ed_Menus);
    FreeGadgets(ed->ed_Gadgets);
}


/*****************************************************************************/


VOID CenterLine(EdDataPtr ed, struct Window *wp, AppStringsID id,
                UWORD x, UWORD y, UWORD w)
{
STRPTR str;
UWORD  len;

    str = GetString(&ed->ed_LocaleInfo,id);
    len = strlen(str);

    Move(wp->RPort,(w-TextLength(wp->RPort,str,len)) / 2 + wp->BorderLeft + x,wp->BorderTop+y);
    Text(wp->RPort,str,len);
}


/*****************************************************************************/


VOID DrawBB(EdDataPtr ed, SHORT x, SHORT y, SHORT w, SHORT h, ULONG tags, ...)
{
    DrawBevelBoxA(ed->ed_Window->RPort,x+ed->ed_Window->BorderLeft,
                                       y+ed->ed_Window->BorderTop,
                                       w,h,(struct TagItem *)&tags);
}

/*****************************************************************************/

void ClearGadPtrs(EdDataPtr ed)
{
    ULONG *gad;
    for (gad = (ULONG *) &ed->ed_MainHostName; gad <= (ULONG *)&ed->ed_DeviceDelete; gad++)
    {
        *gad = NULL;
    }
}

/*****************************************************************************/

VOID RenderGadgets(EdDataPtr ed)
{
UWORD           i;
struct Node    *node;
BOOL able, checked;
ULONG num;

    switch(ed->ed_Panel)
    {
        case PANEL_MAIN:
                            if(!ed->ed_LastAdded)
                            {
                                ClearGadPtrs(ed);
                                ed->ed_LastAdded = CreateContext(&ed->ed_Gadgets);
                                DoPrefsGadget(ed,&EG[0],NULL,TAG_DONE);
//                                DoPrefsGadget(ed,&EG[1],NULL,TAG_DONE);
                                DoPrefsGadget(ed,&EG[2],NULL,TAG_DONE);
                            /*    DoPrefsGadget(ed,&EG[3],NULL,TAG_DONE);
                                DoPrefsGadget(ed,&EG[4],NULL,TAG_DONE);
                                DoPrefsGadget(ed,&EG[5],NULL,TAG_DONE); */
                            };

                            if((ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_HostFlags & NHPFLAGF_REALMSERVER) &&
                            	(ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_HostFlags & NHPFLAGF_REALMS))
                            	ed->ed_PanelLabels[3] = GetString(&ed->ed_LocaleInfo,MSG_PANEL_REALMS);
			    else
			    	ed->ed_PanelLabels[3] = NULL;

                            ed->ed_PanelGad = DoPrefsGadget(ed,&EG[9],ed->ed_PanelGad,
                                                            GTCY_Active, (ULONG)ed->ed_Panel,
                                                            GTCY_Labels, ed->ed_PanelLabels,
                                                            TAG_DONE);

                            ed->ed_MainHostName = DoPrefsGadget(ed,&EG[6],ed->ed_MainHostName,
                                                                GTST_String,   ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_HostName,
                                                                GTST_MaxChars, 63,
                                                                TAG_DONE);

                            ed->ed_MainOwnerName = DoPrefsGadget(ed,&EG[50],ed->ed_MainOwnerName,
                                                                GTST_String,   ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_OwnerName,
                                                                GTST_MaxChars, 63,
                                                                TAG_DONE);

                            checked = (ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_HostFlags & NHPFLAGF_REALMS);

                            ed->ed_MainUseRealms = DoPrefsGadget(ed,&EG[51],ed->ed_MainUseRealms,
                            					 GTCB_Checked, checked,
                            					 TAG_DONE);

                            if(checked)
                            {
                                ed->ed_MainRealmName = DoPrefsGadget(ed,&EG[7],ed->ed_MainRealmName,
                                                                     GTST_String,   ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_RealmName,
                                                                     GTST_MaxChars, 63,
                                                                     TAG_DONE);
                                IPToStr(ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_RealmServAddr,ed->ed_Str1);

                                ed->ed_MainRealmAddress = DoPrefsGadget(ed,&EG[8],ed->ed_MainRealmAddress,
                                                                        GTST_String, ed->ed_Str1,
                                                                        GTST_MaxChars, 16,
                                                                        GTST_EditHook, &ed->ed_IPGadHook,
                                                                        TAG_DONE);

                                ed->ed_MainIsRealmServer = DoPrefsGadget(ed,&EG[52],ed->ed_MainIsRealmServer,
                                					 GTCB_Checked, (ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_HostFlags & NHPFLAGF_REALMSERVER),
                                					 TAG_DONE);
                     	    }
                            break;

        case PANEL_REALMS:
                            if(!ed->ed_LastAdded)
                            {
                                ClearGadPtrs(ed);
                                ed->ed_LastAdded = CreateContext(&ed->ed_Gadgets);
                                DoPrefsGadget(ed,&EG[0],NULL,TAG_DONE);
//                                DoPrefsGadget(ed,&EG[1],NULL,TAG_DONE);
                                DoPrefsGadget(ed,&EG[2],NULL,TAG_DONE);
                                DoPrefsGadget(ed,&EG[11],NULL,TAG_DONE);
                                DoPrefsGadget(ed,&EG[13],NULL,TAG_DONE);
                                DoPrefsGadget(ed,&EG[15],NULL,TAG_DONE);
                                DoPrefsGadget(ed,&EG[16],NULL,TAG_DONE);
                            };
                            ed->ed_PanelGad = DoPrefsGadget(ed,&EG[9],ed->ed_PanelGad,
                                                            GTCY_Active, (ULONG)ed->ed_Panel,
                                                            GTCY_Labels, ed->ed_PanelLabels,
                                                            TAG_DONE);

                            ed->ed_LocalRealmDelete = DoPrefsGadget(ed,&EG[12],ed->ed_LocalRealmDelete,
                                                                    GA_Disabled, !(ed->ed_CurrentLocRealm),
                                                                    TAG_DONE);

                            ed->ed_RemoteRealmDelete = DoPrefsGadget(ed,&EG[14],ed->ed_RemoteRealmDelete,
                                                                     GA_Disabled, !(ed->ed_CurrentRemRealm),
                                                                     TAG_DONE);

                            node = ed->ed_PrefsWork.ep_NIPCLocalRealms.lh_Head;
                            while(node->ln_Succ)
                            {
                                IPToStr(((struct NIPCRealm *)node)->nz_Prefs.nzp_RealmAddr,ed->ed_Str1);
                                sprintf(node->ln_Name,"%-16.16s %-16.16s",((struct NIPCRealm *)node)->nz_Prefs.nzp_RealmName,ed->ed_Str1);
                                node = node->ln_Succ;
                            }
                            node = ed->ed_PrefsWork.ep_NIPCRemoteRealms.lh_Head;
                            while(node->ln_Succ)
                            {
                                IPToStr(((struct NIPCRealm *)node)->nz_Prefs.nzp_RealmAddr,ed->ed_Str1);
                                sprintf(node->ln_Name,"%-16.16s %-16.16s",((struct NIPCRealm *)node)->nz_Prefs.nzp_RealmName,ed->ed_Str1);
                                node = node->ln_Succ;
                            }
                            if(ed->ed_CurrentLocRealm)
                            {
                                strcpy(ed->ed_Str1,ed->ed_CurrentLocRealm->nz_Prefs.nzp_RealmName);
                                IPToStr(ed->ed_CurrentLocRealm->nz_Prefs.nzp_RealmAddr,ed->ed_Str2);
                            }
                            else
                            {
                                ed->ed_Str1[0]=0x00;
                                ed->ed_Str2[0]=0x00;
                            }
                            ed->ed_LocalRealmName = DoPrefsGadget(ed,&EG[17],ed->ed_LocalRealmName,
                                                                  GTST_String,   ed->ed_Str1,
                                                                  GTST_MaxChars, 63,
                                                                  GA_Disabled, !(ed->ed_CurrentLocRealm),
                                                                  TAG_DONE);

                            ed->ed_LocalRealmAddress = DoPrefsGadget(ed,&EG[18],ed->ed_LocalRealmAddress,
                                                                  GTST_String,   ed->ed_Str2,
                                                                  GTST_MaxChars, 63,
                                                                  GTST_EditHook, &ed->ed_IPGadHook,
                                                                  GA_Disabled, !(ed->ed_CurrentLocRealm),
                                                                  TAG_DONE);

                            if(ed->ed_CurrentRemRealm)
                            {
                                strcpy(ed->ed_Str1,ed->ed_CurrentRemRealm->nz_Prefs.nzp_RealmName);
                                IPToStr(ed->ed_CurrentRemRealm->nz_Prefs.nzp_RealmAddr,ed->ed_Str2);
                            }
                            else
                            {
                                ed->ed_Str1[0]=0x00;
                                ed->ed_Str2[0]=0x00;
                            }
                            ed->ed_RemoteRealmName = DoPrefsGadget(ed,&EG[19],ed->ed_RemoteRealmName,
                                                                   GTST_String,   ed->ed_Str1,
                                                                   GTST_MaxChars, 63,
                                                                   GA_Disabled, !(ed->ed_CurrentRemRealm),
                                                                   TAG_DONE);

                            ed->ed_RemoteRealmAddress = DoPrefsGadget(ed,&EG[20],ed->ed_RemoteRealmAddress,
                                                                   GTST_String,   ed->ed_Str2,
                                                                   GTST_MaxChars, 64,
                                                                   GTST_EditHook, &ed->ed_IPGadHook,
                                                                   GA_Disabled, !(ed->ed_CurrentRemRealm),
                                                                   TAG_DONE);

			    if(ed->ed_V39)
			    {
                                i = 0;
                                node = ed->ed_PrefsWork.ep_NIPCLocalRealms.lh_Head;
                                while(node->ln_Succ)
                                {
                                    if(node == (struct Node *)ed->ed_CurrentLocRealm)
                                        break;
                                    i++;
                                    node = node->ln_Succ;
                                }
                                if(!node->ln_Succ)
                                    i=~0;

                                ed->ed_LocalRealmList = DoPrefsGadget(ed,&EG[21],ed->ed_LocalRealmList,
                                                                      GTLV_Selected,    i,
                                                                      GTLV_ShowSelected, 0,
                                                                      GTLV_Labels,      &ed->ed_PrefsWork.ep_NIPCLocalRealms,
                                                                      LAYOUTA_SPACING,  1,
                                                                      GTLV_ScrollWidth, 18,
                                                                      TAG_DONE);
                                i = 0;
                                node = ed->ed_PrefsWork.ep_NIPCRemoteRealms.lh_Head;
                                while(node->ln_Succ)
                                {
                                    if(node == (struct Node *)ed->ed_CurrentRemRealm)
                                        break;
                                    i++;
                                    node = node->ln_Succ;
                                }
                                if(!node->ln_Succ)
                                    i=~0;

                                ed->ed_RemoteRealmList = DoPrefsGadget(ed,&EG[22],ed->ed_RemoteRealmList,
                                                                       GTLV_Selected,    i,
                                                                       GTLV_ShowSelected,0,
                                                                       GTLV_Labels,      &ed->ed_PrefsWork.ep_NIPCRemoteRealms,
                                                                       LAYOUTA_SPACING,  1,
                                                                       GTLV_ScrollWidth, 18,
                                                                       TAG_DONE);
                            }
                            else
                            {
                            	ed->ed_LocalRealmList = DoPrefsGadget(ed,&EG[21],ed->ed_LocalRealmList,
                                                                      GTLV_Labels,      &ed->ed_PrefsWork.ep_NIPCLocalRealms,
                                                                      LAYOUTA_SPACING,  1,
                                                                      GTLV_ScrollWidth, 18,
                                                                      TAG_DONE);

                                ed->ed_RemoteRealmList = DoPrefsGadget(ed,&EG[22],ed->ed_RemoteRealmList,
                                                                       GTLV_Labels,      &ed->ed_PrefsWork.ep_NIPCRemoteRealms,
                                                                       LAYOUTA_SPACING,  1,
                                                                       GTLV_ScrollWidth, 18,
                                                                       TAG_DONE);
                            }
                            break;

        case PANEL_ROUTES:
                            if(!ed->ed_LastAdded)
                            {
                                ClearGadPtrs(ed);
                                ed->ed_LastAdded = CreateContext(&ed->ed_Gadgets);
                                DoPrefsGadget(ed,&EG[0],NULL,TAG_DONE);
//                                DoPrefsGadget(ed,&EG[1],NULL,TAG_DONE);
                                DoPrefsGadget(ed,&EG[2],NULL,TAG_DONE);
                                DoPrefsGadget(ed,&EG[25],NULL,TAG_DONE);
                               /* DoPrefsGadget(ed,&EG[26],NULL,TAG_DONE); */
                               DoPrefsGadget(ed,&EG[29],NULL,TAG_DONE);
                               DoPrefsGadget(ed,&EG[53],NULL,TAG_DONE);
                            };
                            ed->ed_PanelGad = DoPrefsGadget(ed,&EG[9],ed->ed_PanelGad,
                                                            GTCY_Active, (ULONG)ed->ed_Panel,
                                                            GTCY_Labels, ed->ed_PanelLabels,
                                                            TAG_DONE);

                            ed->ed_RouteDelete = DoPrefsGadget(ed,&EG[26],ed->ed_RouteDelete,
                                                               GA_Disabled, !(ed->ed_CurrentRoute),
                                                               TAG_DONE);
                            if(ed->ed_CurrentRoute)
                            {
                                if(ed->ed_CurrentRoute->nr_Prefs.nrp_DestNetwork)
                                    IPToStr(ed->ed_CurrentRoute->nr_Prefs.nrp_DestNetwork,ed->ed_Str1);
                                else
                                    strcpy(ed->ed_Str1,"default");
                            }
                            else
                                ed->ed_Str1[0]=0x00;

                            ed->ed_RouteDestination = DoPrefsGadget(ed,&EG[27],ed->ed_RouteDestination,
                                                                    GTST_String,   ed->ed_Str1,
                                                                    GTST_MaxChars, 16,
                                                                    GA_Disabled, !(ed->ed_CurrentRoute),
                                                                    TAG_DONE);
                            if(ed->ed_CurrentRoute)
                                IPToStr(ed->ed_CurrentRoute->nr_Prefs.nrp_Gateway,ed->ed_Str1);
                            else
                                ed->ed_Str1[0]=0x00;

                            ed->ed_RouteGateway = DoPrefsGadget(ed,&EG[28],ed->ed_RouteGateway,
                                                                GTST_String,   ed->ed_Str1,
                                                                GTST_MaxChars, 16,
                                                                GTST_EditHook, &ed->ed_IPGadHook,
                                                                GA_Disabled, !(ed->ed_CurrentRoute),
                                                                TAG_DONE);
/*
                            if(ed->ed_CurrentRoute)
                                num = ed->ed_CurrentRoute->nr_Prefs.nrp_Hops;
                            else
                                num = 0;

                            ed->ed_RouteHops = DoPrefsGadget(ed,&EG[29],ed->ed_RouteHops,
                                                             GTIN_Number,   num,
                                                             GTIN_MaxChars, 3,
                                                             GA_Disabled, !(ed->ed_CurrentRoute),
                                                             TAG_DONE);
*/

                            node = ed->ed_PrefsWork.ep_NIPCRoutes.lh_Head;
                            while(node->ln_Succ)
                            {
                                if(((struct NIPCRoute *)node)->nr_Prefs.nrp_DestNetwork == 0L)
                                    strcpy(ed->ed_Str1,"default");
                                else
                                    IPToStr(((struct NIPCRoute *)node)->nr_Prefs.nrp_DestNetwork,ed->ed_Str1);
                                IPToStr(((struct NIPCRoute *)node)->nr_Prefs.nrp_Gateway,ed->ed_Str2);
                                node->ln_Name = (STRPTR) &((struct NIPCRoute *)node)->nr_String;
                                sprintf(node->ln_Name,"%-16.16s     %-16.16s",ed->ed_Str1,ed->ed_Str2);
                                node = node->ln_Succ;
                            }

			    if(ed->ed_V39)
			    {
                                i = 0;
                                node = ed->ed_PrefsWork.ep_NIPCRoutes.lh_Head;
                                while(node->ln_Succ)
                                {
                                    if(node == (struct Node *)ed->ed_CurrentRoute)
                                        break;
                                    i++;
                                    node = node->ln_Succ;
                                }
                                if(!node->ln_Succ)
                                    i=~0;

                                ed->ed_RouteList = DoPrefsGadget(ed,&EG[30],ed->ed_RouteList,
                                                                 GTLV_Selected,    i,
                                                                 GTLV_ShowSelected, 0,
                                                                 GTLV_Labels,      &ed->ed_PrefsWork.ep_NIPCRoutes,
                                                                 LAYOUTA_SPACING,  1,
                                                                 GTLV_ScrollWidth, 18,
                                                                 TAG_DONE);
                            }
                            else
                            {
                            	ed->ed_RouteList = DoPrefsGadget(ed,&EG[30],ed->ed_RouteList,
                                                                 GTLV_Labels,      &ed->ed_PrefsWork.ep_NIPCRoutes,
                                                                 LAYOUTA_SPACING,  1,
                                                                 GTLV_ScrollWidth, 18,
                                                                 TAG_DONE);
                            }
                            break;
        case PANEL_DEVICES:
                            if(!ed->ed_LastAdded)
                            {
                                ClearGadPtrs(ed);
                                ed->ed_LastAdded = CreateContext(&ed->ed_Gadgets);
                                DoPrefsGadget(ed,&EG[0],NULL,TAG_DONE);
//                                DoPrefsGadget(ed,&EG[1],NULL,TAG_DONE);
                                DoPrefsGadget(ed,&EG[2],NULL,TAG_DONE);
                              /*  DoPrefsGadget(ed,&EG[33],NULL,TAG_DONE); */
                              /*  DoPrefsGadget(ed,&EG[34],NULL,TAG_DONE); */
                              /*  DoPrefsGadget(ed,&EG[35],NULL,TAG_DONE); */
                            };

                            if(ed->ed_V39)
                            {
                                i = 0;
                                node = ed->ed_PrefsWork.ep_NIPCDevices.lh_Head;
                                while(node->ln_Succ)
                                {
                                    if(node == (struct Node *)ed->ed_CurrentDevice)
                                        break;
                                    i++;
                                    node = node->ln_Succ;
                                }
                                if(!node->ln_Succ)
                                    i=~0;
                                ed->ed_DeviceList = DoPrefsGadget(ed,&EG[48],ed->ed_DeviceList,
                                                                  GTLV_Selected,     i,
                                                                  GTLV_ShowSelected, 0,
                                                                  GTLV_Labels,       &ed->ed_PrefsWork.ep_NIPCDevices,
                                                                  LAYOUTA_SPACING,   1,
                                                                  GTLV_ScrollWidth,  18,
                                                                  TAG_DONE);
                            }
                            else
                            {
                            	ed->ed_DeviceList = DoPrefsGadget(ed,&EG[48],ed->ed_DeviceList,
                                                                  GTLV_Labels,       &ed->ed_PrefsWork.ep_NIPCDevices,
                                                                  LAYOUTA_SPACING,   1,
                                                                  GTLV_ScrollWidth,  18,
                                                                  TAG_DONE);
                            }

                            EG[33].eg_TopEdge = EG[34].eg_TopEdge = ed->ed_DeviceList->Height + 24;

                            ed->ed_PanelGad = DoPrefsGadget(ed,&EG[9],ed->ed_PanelGad,
                                                            GTCY_Active, (ULONG)ed->ed_Panel,
                                                            GTCY_Labels, ed->ed_PanelLabels,
                                                            TAG_DONE);

			    ed->ed_DeviceAdd = DoPrefsGadget(ed,&EG[33],ed->ed_DeviceDelete,TAG_DONE);

                            ed->ed_DeviceDelete = DoPrefsGadget(ed,&EG[34],ed->ed_DeviceDelete,
                                                                GA_Disabled, !(ed->ed_CurrentDevice),
                                                                TAG_DONE);

                            if(ed->ed_CurrentDevice)
                                able = (ed->ed_CurrentDevice->nd_Prefs.ndp_Flags & NDPFLAGF_ONLINE);
                            else
                                able = 0;

                            ed->ed_DeviceStatus = DoPrefsGadget(ed,&EG[36],ed->ed_DeviceStatus,
                                                                GTCY_Active, (ULONG)able,
                                                                GTCY_Labels, ed->ed_StatusLabels,
                                                                GA_Disabled, !(ed->ed_CurrentDevice),
                                                                TAG_DONE);

                            if(ed->ed_CurrentDevice)
                                checked = !(ed->ed_CurrentDevice->nd_Prefs.ndp_Flags & NDPFLAGF_SUBNET);
                            else
                                checked = TRUE;
                            ed->ed_DeviceSubnetAble = DoPrefsGadget(ed,&EG[37],ed->ed_DeviceSubnetAble,
                                                                    GTCB_Checked, checked,
                                                                    GA_Disabled,   !(ed->ed_CurrentDevice),
                                                                    TAG_DONE);

                            if(ed->ed_CurrentDevice)
                                checked = !(ed->ed_CurrentDevice->nd_Prefs.ndp_Flags & NDPFLAGF_HARDADDR);
                            else
                                checked = TRUE;
                            ed->ed_DeviceAddressAble = DoPrefsGadget(ed,&EG[38],ed->ed_DeviceAddressAble,
                                                                     GTCB_Checked, checked,
                                                                     GA_Disabled,   !(ed->ed_CurrentDevice),
                                                                     TAG_DONE);

                            if(ed->ed_CurrentDevice)
                                checked = (ed->ed_CurrentDevice->nd_Prefs.ndp_Flags & NDPFLAGF_IPTYPE);
                            else
                                checked = TRUE;
                            ed->ed_DeviceIPTypeAble = DoPrefsGadget(ed,&EG[39],ed->ed_DeviceIPTypeAble,
                                                                    GTCB_Checked, checked,
                                                                    GA_Disabled,   !(ed->ed_CurrentDevice),
                                                                    TAG_DONE);

                            if(ed->ed_CurrentDevice)
                                checked = (ed->ed_CurrentDevice->nd_Prefs.ndp_Flags & NDPFLAGF_ARPTYPE);
                            else
                                checked = TRUE;
                            ed->ed_DeviceARPTypeAble = DoPrefsGadget(ed,&EG[40],ed->ed_DeviceARPTypeAble,
                                                                     GTCB_Checked, checked,
                                                                     GA_Disabled,   !(ed->ed_CurrentDevice),
                                                                     TAG_DONE);

/*
                            if(ed->ed_CurrentDevice)
                                strcpy(ed->ed_Str1,ed->ed_CurrentDevice->nd_Prefs.ndp_DevPathName);
                            else
                                ed->ed_Str1[0]=0x00;
                            ed->ed_DevicePathName = DoPrefsGadget(ed,&EG[41],ed->ed_DevicePathName,
                                                                  GTST_String,   ed->ed_Str1,
                                                                  GTST_MaxChars, 255,
                                                                  GA_Disabled,   !(ed->ed_CurrentDevice),
                                                                  TAG_DONE);
*/

                            if(ed->ed_CurrentDevice)
                                num = ed->ed_CurrentDevice->nd_Prefs.ndp_Unit;
                            else
                                num = 0;

                            ed->ed_DeviceUnit = DoPrefsGadget(ed,&EG[42],ed->ed_DeviceUnit,
                                                              GTIN_Number,   num,
                                                              GTIN_MaxChars, 10,
                                                              GA_Disabled, !(ed->ed_CurrentDevice),
                                                              TAG_DONE);

                            if(ed->ed_CurrentDevice)
                                IPToStr(ed->ed_CurrentDevice->nd_Prefs.ndp_IPAddress,ed->ed_Str1);
                            else
                                ed->ed_Str1[0]=0x00;
                            ed->ed_DeviceIP = DoPrefsGadget(ed,&EG[43],ed->ed_DeviceIP,
                                                            GTST_String,   ed->ed_Str1,
                                                            GTST_MaxChars, 16,
                                                            GTST_EditHook, &ed->ed_IPGadHook,
                                                            GA_Disabled,   !(ed->ed_CurrentDevice),
                                                            TAG_DONE);

                            if(ed->ed_CurrentDevice)
                            {
                                able = !(ed->ed_CurrentDevice->nd_Prefs.ndp_Flags & NDPFLAGF_SUBNET);
                                IPToStr(ed->ed_CurrentDevice->nd_Prefs.ndp_IPSubnet,ed->ed_Str1);
                            }
                            else
                            {
                                able = TRUE;
                                ed->ed_Str1[0]=0x00;
                            }
                            ed->ed_DeviceMask = DoPrefsGadget(ed,&EG[44],ed->ed_DeviceMask,
                                                              GTST_String,   ed->ed_Str1,
                                                              GTST_MaxChars, 16,
                                                              GTST_EditHook, &ed->ed_IPGadHook,
                                                              GA_Disabled,   able,
                                                              TAG_DONE);

                            if(ed->ed_CurrentDevice)
                            {
                                able = !(ed->ed_CurrentDevice->nd_Prefs.ndp_Flags & NDPFLAGF_HARDADDR);
                                strcpy(ed->ed_AddressStr,ed->ed_CurrentDevice->nd_Prefs.ndp_HardString);
                            }
                            else
                            {
                                able = TRUE;
                                ed->ed_AddressStr[0]=0x00;
                            }
                            ed->ed_DeviceAddress = DoPrefsGadget(ed,&EG[45],ed->ed_DeviceAddress,
                                                                 GTST_String,   ed->ed_AddressStr,
                                                                 GTST_MaxChars, 40,
                                                                 GA_Disabled,   able,
                                                                 TAG_DONE);

                            if(ed->ed_CurrentDevice)
                            {
                                num = ed->ed_CurrentDevice->nd_Prefs.ndp_IPType;
                                able = (ed->ed_CurrentDevice->nd_Prefs.ndp_Flags & NDPFLAGF_IPTYPE);
                            }
                            else
                            {
                                num = 0;
                                able = TRUE;
                            };
                            ed->ed_DeviceIPType = DoPrefsGadget(ed,&EG[46],ed->ed_DeviceIPType,
                                                                GTIN_Number,   num,
                                                                GTIN_MaxChars, 8,
                                                                GA_Disabled,   able,
                                                                TAG_DONE);

                            if(ed->ed_CurrentDevice)
                            {
                                num = ed->ed_CurrentDevice->nd_Prefs.ndp_ARPType;
                                able = (ed->ed_CurrentDevice->nd_Prefs.ndp_Flags & NDPFLAGF_ARPTYPE);
                            }
                            else
                            {
                                num = 0;
                                able = TRUE;
                            }
                            ed->ed_DeviceARPType = DoPrefsGadget(ed,&EG[47],ed->ed_DeviceARPType,
                                                                 GTIN_Number,   num,
                                                                 GTIN_MaxChars, 8,
                                                                 GA_Disabled,   able,
                                                                 TAG_DONE);

                            break;
    }
}

/*****************************************************************************/

VOID ProcessSpecialCommand(EdDataPtr ed, EdCommand ec)
{
BOOL refresh;
UWORD icode;
struct Node *node;
ULONG num;
STRPTR hardstring;
UBYTE *hardaddr, x, parselength;
struct Gadget *act = NULL;
struct Gadget *curr = NULL;
struct NIPCDevice *old_dev;
struct NIPCRealm *old_realm;
struct NIPCRoute *old_route;


    refresh = FALSE;

    icode = ed->ed_CurrentMsg.Code;

    switch (ec)
    {
        /* Main Screen */
        case EC_MAIN_HOSTNAME   : strcpy(ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_HostName,((struct StringInfo *)ed->ed_MainHostName->SpecialInfo)->Buffer);
//                                  act = ed->ed_MainRealmName;
                                  break;

        case EC_MAIN_REALMNAME  : strcpy(ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_RealmName,((struct StringInfo *)ed->ed_MainRealmName->SpecialInfo)->Buffer);
//                                  act = ed->ed_MainRealmAddress;
                                  break;

        case EC_MAIN_REALMADDR  : if(!StrtoIP(((struct StringInfo *)ed->ed_MainRealmAddress->SpecialInfo)->Buffer,&ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_RealmServAddr))
                                  {
                                      act = ed->ed_MainRealmAddress;
                                      refresh = TRUE;
                                  }
                                  break;

        case EC_PANEL           : ed->ed_Panel = icode;
                                  MakeNewDisplay(ed);
                                  break;

	case EC_MAIN_OWNERNAME  : strcpy(ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_OwnerName,((struct StringInfo *)ed->ed_MainOwnerName->SpecialInfo)->Buffer);
				  break;

	case EC_MAIN_REALMS	: if(ed->ed_MainUseRealms->Flags & GFLG_SELECTED)
                                      ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_HostFlags |= NHPFLAGF_REALMS;
                                  else
                                  {
                                      ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_HostFlags &= ~(NHPFLAGF_REALMS|NHPFLAGF_REALMSERVER);
                                      ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_RealmServAddr = 0L;
                                      ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_RealmName[0]='\0';
                                  }
                                  MakeNewDisplay(ed);
                                  refresh = TRUE;
				  break;

	case EC_MAIN_REALMSERVER: if(ed->ed_MainIsRealmServer->Flags & GFLG_SELECTED)
                                      ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_HostFlags |= NHPFLAGF_REALMSERVER;
                                  else
                                      ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_HostFlags &= ~NHPFLAGF_REALMSERVER;
                                  refresh = TRUE;
				  break;

        /* Realms Screen */
        case EC_LOCREALM_ADD    : old_realm = ed->ed_CurrentLocRealm;
                                  if(ed->ed_CurrentLocRealm = AllocMem(sizeof(struct NIPCRealm),MEMF_PUBLIC|MEMF_CLEAR))
                                  {
                                      ed->ed_CurrentLocRealm->nz_Node.ln_Name = (STRPTR) &ed->ed_CurrentLocRealm->nz_String;
                                      strcpy(ed->ed_CurrentLocRealm->nz_Prefs.nzp_RealmName,"(new)");
				      ed->ed_LocalRealmList = DoPrefsGadget(ed,&EG[21],ed->ed_LocalRealmList,
                                                                  GTLV_Labels,      ~0);
                                      InsertRealmSorted(ed,&ed->ed_PrefsWork.ep_NIPCLocalRealms,ed->ed_CurrentLocRealm);
                                      refresh = TRUE;
                                      act = ed->ed_LocalRealmName;
                                  }
                                  else
                                      ed->ed_CurrentLocRealm = old_realm;
                                  break;

        case EC_LOCREALM_DELETE : if(ed->ed_CurrentLocRealm)
                                  {
				      ed->ed_LocalRealmList = DoPrefsGadget(ed,&EG[21],ed->ed_LocalRealmList,
                                                                  GTLV_Labels,      ~0);
                                      Remove((struct Node *)ed->ed_CurrentLocRealm);
                                      FreeMem(ed->ed_CurrentLocRealm,sizeof(struct NIPCRealm));
                                      ed->ed_CurrentLocRealm = NULL;
                                      refresh = TRUE;
                                  }
                                  break;

        case EC_LOCREALM_NAME   : if(ed->ed_CurrentLocRealm)
                                  {
	                              ed->ed_LocalRealmList = DoPrefsGadget(ed,&EG[22],ed->ed_LocalRealmList,
                                                                   GTLV_Labels,      ~0);

                                      Remove((struct Node *)ed->ed_CurrentLocRealm);

                                      strcpy(ed->ed_CurrentLocRealm->nz_Prefs.nzp_RealmName,((struct StringInfo *)ed->ed_LocalRealmName->SpecialInfo)->Buffer);

                                      InsertRealmSorted(ed,&ed->ed_PrefsWork.ep_NIPCLocalRealms,ed->ed_CurrentLocRealm);

                                  }
                                  if(!act)
                                      act = ed->ed_LocalRealmAddress;
                                  refresh = TRUE;
                                  break;

        case EC_LOCREALM_NET    : if(ed->ed_CurrentLocRealm)
                                  {
                                      if(!StrtoIP(((struct StringInfo *)ed->ed_LocalRealmAddress->SpecialInfo)->Buffer,&ed->ed_CurrentLocRealm->nz_Prefs.nzp_RealmAddr))
                                      {
                                          act = ed->ed_LocalRealmAddress;
                                      }
                                  }
                                  refresh = TRUE;
                                  break;

        case EC_LOCREALM_LIST   : node = ed->ed_PrefsWork.ep_NIPCLocalRealms.lh_Head;
                                  while(icode--)
                                  {
                                      node = node->ln_Succ;
                                  }
                                  ed->ed_CurrentLocRealm = (struct NIPCRealm *)node;
                                  refresh = TRUE;
                                  break;

        case EC_REMREALM_ADD    : old_realm = ed->ed_CurrentRemRealm;
                                  if(ed->ed_CurrentRemRealm = AllocMem(sizeof(struct NIPCRealm),MEMF_PUBLIC|MEMF_CLEAR))
                                  {
                                      ed->ed_CurrentRemRealm->nz_Node.ln_Name = (STRPTR) &ed->ed_CurrentRemRealm->nz_String;
                                      strcpy(ed->ed_CurrentRemRealm->nz_Prefs.nzp_RealmName,"(new)");
	                              ed->ed_RemoteRealmList = DoPrefsGadget(ed,&EG[22],ed->ed_RemoteRealmList,
                                                                   GTLV_Labels,      ~0);
                                      InsertRealmSorted(ed,&ed->ed_PrefsWork.ep_NIPCRemoteRealms,ed->ed_CurrentRemRealm);
                                      refresh = TRUE;
                                      act = ed->ed_RemoteRealmName;
                                  }
                                  else
                                      ed->ed_CurrentRemRealm = old_realm;
                                  break;

        case EC_REMREALM_DELETE : if(ed->ed_CurrentRemRealm)
                                  {
	                              ed->ed_RemoteRealmList = DoPrefsGadget(ed,&EG[22],ed->ed_RemoteRealmList,
                                                                   GTLV_Labels,      ~0);
                                      Remove((struct Node *)ed->ed_CurrentRemRealm);
                                      FreeMem(ed->ed_CurrentRemRealm,sizeof(struct NIPCRealm));
                                      ed->ed_CurrentRemRealm = NULL;
                                  }
                                  refresh = TRUE;
                                  break;

        case EC_REMREALM_NAME   : if(ed->ed_CurrentRemRealm)
                                  {
	                              ed->ed_RemoteRealmList = DoPrefsGadget(ed,&EG[22],ed->ed_RemoteRealmList,
                                                                   GTLV_Labels,      ~0);
                                      Remove((struct Node *)ed->ed_CurrentRemRealm);
                                      strcpy(ed->ed_CurrentRemRealm->nz_Prefs.nzp_RealmName,((struct StringInfo *)ed->ed_RemoteRealmName->SpecialInfo)->Buffer);
				      InsertRealmSorted(ed,&ed->ed_PrefsWork.ep_NIPCRemoteRealms,ed->ed_CurrentRemRealm);
                                  }
                                  act = ed->ed_RemoteRealmAddress;
                                  refresh = TRUE;
                                  break;

        case EC_REMREALM_NET    : if(ed->ed_CurrentRemRealm)
                                  {
                                      if(!StrtoIP(((struct StringInfo *)ed->ed_RemoteRealmAddress->SpecialInfo)->Buffer,&ed->ed_CurrentRemRealm->nz_Prefs.nzp_RealmAddr))
                                      {
                                          act = ed->ed_RemoteRealmAddress;
                                      }
                                  }
                                  refresh = TRUE;
                                  break;

        case EC_REMREALM_LIST   : node = ed->ed_PrefsWork.ep_NIPCRemoteRealms.lh_Head;
                                  while(icode--)
                                  {
                                      node = node->ln_Succ;
                                  }
                                  ed->ed_CurrentRemRealm = (struct NIPCRealm *)node;
                                  refresh = TRUE;
                                  break;

        /* Routes Screen */

        case EC_ROUTE_ADD       : old_route = ed->ed_CurrentRoute;
                                  if(ed->ed_CurrentRoute = AllocMem(sizeof(struct NIPCRoute),MEMF_PUBLIC|MEMF_CLEAR))
                                  {
	                              ed->ed_RouteList = DoPrefsGadget(ed,&EG[30],ed->ed_RouteList,
                                                             GTLV_Labels,	~0);
                                      AddTail(&ed->ed_PrefsWork.ep_NIPCRoutes,(struct Node *)ed->ed_CurrentRoute);
                                  }
                                  else
                                      ed->ed_CurrentRoute = old_route;
                                  refresh = TRUE;
                                  break;

        case EC_ROUTE_DELETE    : if(ed->ed_CurrentRoute)
                                  {
	                              ed->ed_RouteList = DoPrefsGadget(ed,&EG[30],ed->ed_RouteList,
                                                             GTLV_Labels,	~0);
                                      Remove((struct Node *)ed->ed_CurrentRoute);
                                      FreeMem(ed->ed_CurrentRoute,sizeof(struct NIPCRoute));
                                      ed->ed_CurrentRoute = NULL;
                                      refresh = TRUE;
                                  }
                                  break;

        case EC_ROUTE_DEST      : if(ed->ed_CurrentRoute)
                                  {
                                      if(!Stricmp(((struct StringInfo *)ed->ed_RouteDestination->SpecialInfo)->Buffer,"default"))
                                          ed->ed_CurrentRoute->nr_Prefs.nrp_DestNetwork = 0L;
                                      else if(!StrtoIP(((struct StringInfo *)ed->ed_RouteDestination->SpecialInfo)->Buffer,&ed->ed_CurrentRoute->nr_Prefs.nrp_DestNetwork))
                                      {
                                          act = ed->ed_RouteDestination;
                                      }
                                  }
                                  if(!act)
                                      act = ed->ed_RouteGateway;

                                  refresh = TRUE;
                                  break;

        case EC_ROUTE_GATEWAY   : if(ed->ed_CurrentRoute)
                                  {
                                      if(!StrtoIP(((struct StringInfo *)ed->ed_RouteGateway->SpecialInfo)->Buffer,&ed->ed_CurrentRoute->nr_Prefs.nrp_Gateway))
                                      {
                                          act = ed->ed_RouteGateway;
                                      }
                                  }
                                  if(!act)
                                      act = ed->ed_RouteHops;

                                  refresh = TRUE;
                                  break;

        case EC_ROUTE_HOPS      : if(ed->ed_CurrentRoute)
                                  {
                                      num = ((struct StringInfo *)ed->ed_RouteHops->SpecialInfo)->LongInt;
                                      if(num < 0)
                                      {
                                          num = 0;
                                          act = ed->ed_RouteHops;
                                          refresh = TRUE;
                                      }
                                      ed->ed_CurrentRoute->nr_Prefs.nrp_Hops = num;
                                  }
                                  refresh = TRUE;
                                  break;

        case EC_ROUTE_LIST      : node = ed->ed_PrefsWork.ep_NIPCRoutes.lh_Head;
                                  while(icode--)
                                  {
                                      node = node->ln_Succ;
                                  }
                                  ed->ed_CurrentRoute = (struct NIPCRoute *)node;
                                  refresh = TRUE;
                                  break;

        /* Devices Panel */

        case EC_DEVICE_UNIT     : if(ed->ed_CurrentDevice)
                                  {
                                      num = ((struct StringInfo *)ed->ed_DeviceUnit->SpecialInfo)->LongInt;
                                      if(num < 0)
                                      {
                                          num = 0;
                                          act = ed->ed_DeviceUnit;
                                          refresh = TRUE;
                                      }
                                      ed->ed_CurrentDevice->nd_Prefs.ndp_Unit = num;
                                  }
                                  if(!act)
                                      curr = ed->ed_DeviceUnit;
                                  break;

        case EC_DEVICE_IPTYPE   : if(ed->ed_CurrentDevice)
                                  {
                                      num = ((struct StringInfo *)ed->ed_DeviceIPType->SpecialInfo)->LongInt;
                                      if(num < 0)
                                      {
                                          num = 0;
                                          act = ed->ed_DeviceIPType;
                                          refresh = TRUE;
                                      }
                                      ed->ed_CurrentDevice->nd_Prefs.ndp_IPType = num;
                                  }
                                  if(!act)
                                      curr = ed->ed_DeviceIPType;
                                  break;

        case EC_DEVICE_ARPTYPE  : if(ed->ed_CurrentDevice)
                                  {
                                      num = ((struct StringInfo *)ed->ed_DeviceARPType->SpecialInfo)->LongInt;
                                      if(num < 0)
                                      {
                                         num = 0;
                                         act = ed->ed_DeviceARPType;
                                         refresh = TRUE;
                                      }
                                      ed->ed_CurrentDevice->nd_Prefs.ndp_ARPType = num;
                                  }
                                  if(!act)
                                      curr = ed->ed_DeviceARPTypeAble;
                                  break;

        case EC_DEVICE_IP       : if(ed->ed_CurrentDevice)
                                  {
                                      if(!StrtoIP(((struct StringInfo *)ed->ed_DeviceIP->SpecialInfo)->Buffer,&ed->ed_CurrentDevice->nd_Prefs.ndp_IPAddress))
                                      {
                                         act = ed->ed_DeviceIP;
                                         refresh = TRUE;
                                      }
                                  }
                                  if(!act)
                                      curr = ed->ed_DeviceIP;
                                  break;

        case EC_DEVICE_MASK     : if(ed->ed_CurrentDevice)
                                  {
                                      if(!StrtoIP(((struct StringInfo *)ed->ed_DeviceMask->SpecialInfo)->Buffer,&ed->ed_CurrentDevice->nd_Prefs.ndp_IPSubnet))
                                      {
                                         act = ed->ed_DeviceMask;
                                         refresh = TRUE;
                                      }
                                  }
                                  if(!act)
                                      curr = ed->ed_DeviceMask;
                                  break;

        case EC_DEVICE_ADDR     : if(ed->ed_CurrentDevice)
                                  {
                                      hardstring =((struct StringInfo *)ed->ed_DeviceAddress->SpecialInfo)->Buffer;
                                      strncpy(ed->ed_CurrentDevice->nd_Prefs.ndp_HardString,hardstring,38);
                                      hardaddr = ed->ed_CurrentDevice->nd_Prefs.ndp_HardAddress;

                                      if (!Strnicmp("0x", hardstring, 2))
                                      {
                                          parselength = strlen(hardstring);
                                          if ((parselength & 1L) == 1)
                                              strcpy(hardstring + 1, hardstring + 2);
                                          else
                                              strcpy(hardstring, hardstring + 2);
                                          for (x = 0; x < strlen(hardstring); x += 2)
                                              hardaddr[x / 2] = ((nibvert(hardstring[x]) << 4) | (nibvert(hardstring[x + 1])));
                                      }
                                  }
                                  curr = ed->ed_DeviceAddress;
                                  break;

        case EC_DEVICE_LIST     : node = ed->ed_PrefsWork.ep_NIPCDevices.lh_Head;
                                  while (icode--)
                                  {
                                      node = node->ln_Succ;
                                  }
                                  ed->ed_CurrentDevice = (struct NIPCDevice *)node;
                                  refresh = TRUE;
                                  break;

        case EC_DEVICE_NAME     : strcpy((STRPTR)&ed->ed_CurrentDevice->nd_Prefs.ndp_DevPathName,((struct StringInfo *)ed->ed_DevicePathName->SpecialInfo)->Buffer);
                                  refresh = TRUE;
                                  curr = ed->ed_DevicePathName;
                                  break;

        case EC_DEVICE_STATUS   : if(ed->ed_CurrentDevice)
                                  {
                                      if(icode)
                                          ed->ed_CurrentDevice->nd_Prefs.ndp_Flags |= NDPFLAGF_ONLINE;
                                      else
                                          ed->ed_CurrentDevice->nd_Prefs.ndp_Flags &= ~NDPFLAGF_ONLINE;
                                  }
                                  break;

        case EC_DEVICE_MASK_ABLE    : if(ed->ed_CurrentDevice)
                                      {
                                          if(ed->ed_DeviceSubnetAble->Flags & GFLG_SELECTED)
                                              ed->ed_CurrentDevice->nd_Prefs.ndp_Flags &= ~NDPFLAGF_SUBNET;
                                          else
                                              ed->ed_CurrentDevice->nd_Prefs.ndp_Flags |= NDPFLAGF_SUBNET;
                                          refresh = TRUE;
                                      }
                                      break;

        case EC_DEVICE_ADDR_ABLE    : if(ed->ed_CurrentDevice)
                                      {
                                          if(ed->ed_DeviceAddressAble->Flags & GFLG_SELECTED)
                                              ed->ed_CurrentDevice->nd_Prefs.ndp_Flags &= ~NDPFLAGF_HARDADDR;
                                          else
                                              ed->ed_CurrentDevice->nd_Prefs.ndp_Flags |= NDPFLAGF_HARDADDR;
                                          refresh = TRUE;
                                      }
                                      break;

        case EC_DEVICE_IPTYPE_ABLE  : if(ed->ed_CurrentDevice)
                                      {
                                          if(ed->ed_DeviceIPTypeAble->Flags & GFLG_SELECTED)
                                              ed->ed_CurrentDevice->nd_Prefs.ndp_Flags |= NDPFLAGF_IPTYPE;
                                          else
                                              ed->ed_CurrentDevice->nd_Prefs.ndp_Flags &= ~NDPFLAGF_IPTYPE;
                                          refresh = TRUE;
                                      }
                                      break;

        case EC_DEVICE_ARPTYPE_ABLE : if(ed->ed_CurrentDevice)
                                      {
                                          if(ed->ed_DeviceARPTypeAble->Flags & GFLG_SELECTED)
                                              ed->ed_CurrentDevice->nd_Prefs.ndp_Flags |= NDPFLAGF_ARPTYPE;
                                          else
                                              ed->ed_CurrentDevice->nd_Prefs.ndp_Flags &= ~NDPFLAGF_ARPTYPE;
                                          refresh = TRUE;
                                      }
                                      break;

        case EC_DEVICE_ADD      : AddDeviceReq(ed);
         			  refresh = TRUE;
         			  break;

        case EC_DEVICE_DEL      :
	                          ed->ed_DeviceList = DoPrefsGadget(ed,&EG[48],ed->ed_DeviceList,
                                                              GTLV_Labels,       ~0);
        			  Remove(ed->ed_CurrentDevice);
                                  FreeMem(ed->ed_CurrentDevice,sizeof(struct NIPCDevice));
                                  ed->ed_CurrentDevice = NULL;
                                  refresh = TRUE;
                                  break;

        default                 : break;
    }

    if (refresh)
        RenderGadgets(ed);

    if(curr)
    {
        act = NULL;
        while(act != curr)
        {
            act = (act) ? act->NextGadget : curr->NextGadget;
            if(!(act->Flags & GFLG_DISABLED))
                break;
            if(act == ed->ed_DeviceARPType)
                act = ed->ed_DeviceARPTypeAble;

        }
    }
    if(act)
        ActivateGadget(act,window,NULL);
}


/*****************************************************************************/

BOOL StrtoIP(STRPTR ipstr, ULONG *ipnum)
{
    char *y;
    int x, tmp;
    ULONG add;

    y = ipstr;
    add = 0L;
    if(ipstr[0])
    {
        for(x = 0; x < 3; x++)
        {
            stcd_l(y,&tmp);
            add = (add << 8) | tmp;
            y = (char *) strchr((char *) (y+1), '.');
            if((!y) || (*y == 0))
                return FALSE;
            y = (char *) (y + 1);
        }
        stcd_l(y, &tmp);
        add = (add << 8) | tmp;
        *ipnum = add;
    }
    else
    	*ipnum=0L;

    return(TRUE);
}

/*****************************************************************************/
void MakeNewDisplay(EdDataPtr ed)
{
    RemoveGList(window, ed->ed_Gadgets, -1);
    SetAPen(window->RPort, ed->ed_DrawInfo->dri_Pens[BACKGROUNDPEN]);
    RectFill(window->RPort, window->BorderLeft, window->BorderTop + 20, window->Width - window->BorderRight - 1, window->Height - window->BorderBottom - 25);
    FreeGadgets(ed->ed_Gadgets);

    ed->ed_Gadgets = NULL;
    ed->ed_LastAdded = NULL;
    RenderGadgets(ed);
    if (ed->ed_LastAdded)
    {
        AddGList (window, ed->ed_Gadgets, -1, -1, NULL);
        RefreshGList (ed->ed_Gadgets, window, NULL, -1);
        GT_RefreshWindow (window, NULL);
    }
}

/*****************************************************************************/

void IPToStr(ULONG ipnum, STRPTR ipstr)
{
    UBYTE num[4];

    if(ipnum)
    {
        num[0] = (ipnum >> 24L);
        num[1] = (ipnum >> 16L) & 0xff;
        num[2] = (ipnum >>  8L) & 0xff;
        num[3] = (ipnum & 0xff);
        sprintf(ipstr,"%.3ld.%.3ld.%.3ld.%.3ld",num[0],num[1],num[2],num[3]);
    }
    else
    	ipstr[0]=0;
}

/*****************************************************************************/

char nibvert(ichar)
char ichar;
{
    if (ichar > 96)
        return ((char) (ichar - 87));
    if ((ichar >= '0') && (ichar <= '9'))
        return ((char) (ichar - '0'));
    else
        return ((char) (ichar - 55));
}

/*****************************************************************************/

VOID GetSpecialCmdState(EdDataPtr ed, EdCommand ec, CmdStatePtr state)
{
    state->cs_Available = TRUE;
    state->cs_Selected  = FALSE;
}

/*****************************************************************************/

VOID CopyPrefs(EdDataPtr ed, struct ExtPrefs *p1, struct ExtPrefs *p2)
{
    struct NIPCDevice *nd1, *nd2;
    struct NIPCRoute *nr1, *nr2;
    struct NIPCRealm *nz1, *nz2;

    while(nd1 = (struct NIPCDevice *)RemHead(&p1->ep_NIPCDevices))
    {
        FreeMem(nd1,sizeof(struct NIPCDevice));
    }
    while(nr1 = (struct NIPCRoute *)RemHead(&p1->ep_NIPCRoutes))
    {
        FreeMem(nr1,sizeof(struct NIPCRoute));
    }
    while(nz1 = (struct NIPCRealm *)RemHead(&p1->ep_NIPCLocalRealms))
    {
        FreeMem(nz1,sizeof(struct NIPCRealm));
    }
    while(nz1 = (struct NIPCRealm *)RemHead(&p1->ep_NIPCRemoteRealms))
    {
        FreeMem(nz1,sizeof(struct NIPCRealm));
    }

    nd1 = (struct NIPCDevice *)p2->ep_NIPCDevices.lh_Head;
    while(nd1->nd_Node.ln_Succ)
    {
        if(nd2 = (struct NIPCDevice *)AllocMem(sizeof(struct NIPCDevice),MEMF_PUBLIC|MEMF_CLEAR))
        {
            CopyMem(&nd1->nd_Prefs,&nd2->nd_Prefs,sizeof(struct NIPCDevicePrefs));
            nd2->nd_Node.ln_Name = (STRPTR) &nd2->nd_Prefs.ndp_DevPathName;
            AddTail(&p1->ep_NIPCDevices,(struct Node *)nd2);
        }
        nd1 = (struct NIPCDevice *) nd1->nd_Node.ln_Succ;
    }

    nz1 = (struct NIPCRealm *)p2->ep_NIPCLocalRealms.lh_Head;
    while(nz1->nz_Node.ln_Succ)
    {
        if(nz2 = (struct NIPCRealm *)AllocMem(sizeof(struct NIPCRealm),MEMF_PUBLIC|MEMF_CLEAR))
        {
            CopyMem(&nz1->nz_Prefs,&nz2->nz_Prefs,sizeof(struct NIPCRealmPrefs));
            nz2->nz_Node.ln_Name = (STRPTR) &nz2->nz_String;
            AddTail(&p1->ep_NIPCLocalRealms,(struct Node *)nz2);
        }
        nz1 = (struct NIPCRealm *) nz1->nz_Node.ln_Succ;
    }

    nz1 = (struct NIPCRealm *)p2->ep_NIPCRemoteRealms.lh_Head;
    while(nz1->nz_Node.ln_Succ)
    {
        if(nz2 = (struct NIPCRealm *)AllocMem(sizeof(struct NIPCRealm),MEMF_PUBLIC|MEMF_CLEAR))
        {
            CopyMem(&nz1->nz_Prefs,&nz2->nz_Prefs,sizeof(struct NIPCRealmPrefs));
            nz2->nz_Node.ln_Name = (STRPTR) &nz2->nz_String;
            AddTail(&p1->ep_NIPCRemoteRealms,(struct Node *)nz2);
        }
        nz1 = (struct NIPCRealm *) nz1->nz_Node.ln_Succ;
    }

    nr1 = (struct NIPCRoute *)p2->ep_NIPCRoutes.lh_Head;
    while(nr1->nr_Node.ln_Succ)
    {
        if(nr2 = (struct NIPCRoute *)AllocMem(sizeof(struct NIPCRoute),MEMF_PUBLIC|MEMF_CLEAR))
        {
            CopyMem(&nr1->nr_Prefs,&nr2->nr_Prefs,sizeof(struct NIPCRoutePrefs));
            nr2->nr_Node.ln_Name = (STRPTR) &nr2->nr_String;
            AddTail(&p1->ep_NIPCRoutes,(struct Node *)nr2);
        }
        nr1 = (struct NIPCRoute *) nr1->nr_Node.ln_Succ;
    }
    CopyMem(&p2->ep_NIPCHostPrefs,&p1->ep_NIPCHostPrefs,sizeof(struct NIPCHostPrefs));
    if((p1 == (struct ExtPrefs *)&ed->ed_PrefsWork) && (!ed->ed_Cancelled))
    {
        ed->ed_CurrentDevice = NULL;
        ed->ed_CurrentLocRealm = NULL;
        ed->ed_CurrentRemRealm = NULL;
        ed->ed_CurrentRoute = NULL;
        RenderDisplay(ed);
    }
}

/*****************************************************************************/

BOOL DeviceRequest(EdDataPtr ed)
{
BOOL        success;
struct Hook hook;

    hook.h_Entry = IntuiHook;    /* what should this be cast to to avoid warnings?? */

    if (!ed->ed_DevReq)
    {
        if (!(ed->ed_DevReq = AllocPrefsRequest(ed, ASL_FileRequest,
                                                 ASLFR_RejectIcons,     TRUE,
                                                 ASL_Pattern,           "#?.device",
                                                 ASLFR_InitialLeftEdge, window->LeftEdge+12,
                                                 ASLFR_InitialTopEdge,  window->TopEdge+12,
                                                 ASLFR_InitialDrawer,   "DEVS:Networks",
                                                 ASLFR_Window,          ed->ed_Window,
                                                 ASLFR_SleepWindow,     TRUE,
                                                 ASLFR_RejectIcons,     TRUE,
                                                 TAG_DONE)))
        {
            ShowError2(ed,ES_NO_MEMORY);
            return(FALSE);
        }
    }

    success = RequestDevice(ed,ASLFR_TitleText,    GetString(&ed->ed_LocaleInfo,MSG_REQ_LOAD),
                                      ASLFR_DoSaveMode,   FALSE,
                                      ASLFR_IntuiMsgFunc, &hook,
                                      TAG_DONE);
    if (success)
    {
        stccpy(ed->ed_NameBuf,ed->ed_DevReq->rf_Dir, NAMEBUFSIZE);
        AddPart(ed->ed_NameBuf,ed->ed_DevReq->rf_File, NAMEBUFSIZE);
        return(TRUE);
    }

    return(FALSE);
}

BOOL RequestDevice(EdDataPtr ed, ULONG tag1, ...)
{
    return(AslRequest(ed->ed_DevReq,(struct TagItem *) &tag1));
}

/********************************************************************/

VOID AddDeviceReq(EdDataPtr ed)
{
struct NIPCDevice *old_dev;
struct Node *node;
struct IOSana2Req *ios2;
struct MsgPort *replyport;
struct Sana2DeviceQuery s2dq;
BPTR lock;

    old_dev = ed->ed_CurrentDevice;
    if(ed->ed_CurrentDevice = AllocMem(sizeof(struct NIPCDevice),MEMF_CLEAR|MEMF_PUBLIC))
    {
    	if(DeviceRequest(ed))
    	{
	    ed->ed_DeviceList = DoPrefsGadget(ed,&EG[48],ed->ed_DeviceList,
                                    GTLV_Labels,       ~0);

	    if(lock = Lock(ed->ed_NameBuf,ACCESS_READ))
	    {
	    	if(NameFromLock(lock,ed->ed_CurrentDevice->nd_Prefs.ndp_DevPathName,256))
	    	{
                    strcpy(ed->ed_CurrentDevice->nd_Prefs.ndp_DevPathName,ed->ed_NameBuf);
                    AddTail(&ed->ed_PrefsWork.ep_NIPCDevices,(struct Node *)ed->ed_CurrentDevice);
                    node = (struct Node *)ed->ed_CurrentDevice;
                    node->ln_Name = (STRPTR) &ed->ed_CurrentDevice->nd_Prefs.ndp_DevPathName[0];
                    if(replyport = CreateMsgPort())
                    {
                        if(ios2 = CreateIORequest(replyport,sizeof(struct IOSana2Req)))
                        {
                            ios2->ios2_BufferManagement = BMTags;

                            if(!OpenDevice(ed->ed_CurrentDevice->nd_Prefs.ndp_DevPathName,0,(struct IORequest *)ios2,0))
                            {
                                ios2->ios2_Req.io_Command = S2_DEVICEQUERY;
                                ios2->ios2_StatData = &s2dq;
                                s2dq.SizeAvailable = sizeof(struct Sana2DeviceQuery);
                                s2dq.DevQueryFormat = 0;
                                s2dq.DeviceLevel = 0;
                                DoIO((struct IORequest *)ios2);
                                switch(s2dq.HardwareType)
                                {
                                        case S2WireType_Ethernet:       ed->ed_CurrentDevice->nd_Prefs.ndp_IPType = 2048;
                                                                        ed->ed_CurrentDevice->nd_Prefs.ndp_ARPType = 2054;
                                                                        ed->ed_CurrentDevice->nd_Prefs.ndp_Flags |= (NDPFLAGF_IPTYPE | NDPFLAGF_ARPTYPE);
                                                                        break;
                                        case S2WireType_Arcnet:         ed->ed_CurrentDevice->nd_Prefs.ndp_IPType = 240;
                                                                        ed->ed_CurrentDevice->nd_Prefs.ndp_ARPType = 241;
                                                                        ed->ed_CurrentDevice->nd_Prefs.ndp_Flags |= (NDPFLAGF_IPTYPE | NDPFLAGF_ARPTYPE);
                                                                        break;
                                        case S2WireType_SLIP:
                                        case S2WireType_CSLIP:          ed->ed_CurrentDevice->nd_Prefs.ndp_Flags |= (NDPFLAGF_IPTYPE | NDPFLAGF_ARPTYPE);
                                                                        break;

                                        default:                        break;
                                }
                                CloseDevice((struct IORequest *)ios2);
                            }
                            DeleteIORequest((struct IORequest *)ios2);
                        }
                        DeleteMsgPort(replyport);
                    }
                }
                UnLock(lock);
            }
            return;
        }
        if(ed->ed_DevReq)
            FreeAslRequest(ed->ed_DevReq);
        ed->ed_DevReq = NULL;
        FreeMem(ed->ed_CurrentDevice,sizeof(struct NIPCDevice));
        ed->ed_CurrentDevice = old_dev;
    }
    ed->ed_CurrentDevice = old_dev;
}

/*****************************************************************************/

#define ASM __asm
#define REG(x) register __## x

ULONG ASM StringHook(REG(a0) struct Hook *hook,
                   REG(a2) struct SGWork *sgw,
                   REG(a1) ULONG *msg)
{
UBYTE *work_ptr;
ULONG return_code;

    return_code = ~0L;

    if(*msg == SGH_KEY)
    {
    	if((sgw->EditOp == EO_REPLACECHAR) ||
    	   (sgw->EditOp == EO_INSERTCHAR))
    	{
    	    if(!IsIPDigit(sgw->Code))
    	    {
    	    	sgw->Actions |= SGA_BEEP;
    	    	sgw->Actions &= ~SGA_USE;
    	    }
    	}
    }
    else
    	return_code = 0;

    return(return_code);
}

BOOL IsIPDigit(UBYTE test_char)
{
    if(((test_char >= '0') && (test_char <= '9')) ||
       (test_char == '.'))
    	return TRUE;
    else
    	return FALSE;
}

/*****************************************************************************/

BOOL DummyBuffFunc(APTR to, APTR from, LONG length)
{
	return TRUE;
};
