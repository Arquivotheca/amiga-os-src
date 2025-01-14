#ifndef PE_CUSTOM_H
#define PE_CUSTOM_H

/* includes */
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/ports.h>
#include <exec/execbase.h>
#include <intuition/intuition.h>
#include <intuition/sghooks.h>
#include <intuition/screens.h>
#include <graphics/text.h>
#include <libraries/asl.h>
#include <workbench/workbench.h>
#include <prefs/prefhdr.h>
#include <string.h>

/* application includes */
#include "nipcconfig_rev.h"
#include "pe_strings.h"


/*****************************************************************************/


/* define the editor's command-line template here
 * OPT_FROM, OPT_EDIT, OPT_USE, OPT_SAVE, OPT_SCREEN and OPT_COUNT must all
 * be defined. If any of these do not apply, artificially bump OPT_COUNT
 * by one, and set the value of the unused arguments to OPT_COUNT-1
 */
#define TEMPLATE     "FROM,EDIT/S,USE/S,SAVE/S,PUBSCREEN/K" VERSTAG
#define OPT_FROM     0
#define OPT_EDIT     1
#define OPT_USE      2
#define OPT_SAVE     3
#define OPT_SCREEN   4
#define OPT_COUNT    5


/*****************************************************************************/


/* You don't generally touch these */
#define NAMEBUFSIZE  512
#define PROGPATHSIZE 256

#define ID_NDEV  MAKE_ID('N','D','E','V')
#define ID_NRRM  MAKE_ID('N','R','R','M')
#define ID_NLRM  MAKE_ID('N','L','R','M')
#define ID_NIRT  MAKE_ID('N','I','R','T')
#define ID_HOST  MAKE_ID('H','O','S','T')
#define ID_EHST	 MAKE_ID('E','H','S','T')

/*****************************************************************************/


/* Fill-in these values for the editor */
#define PORT_NAME   "NIPCConfig_Prefs"
#define ENV_NAME    "ENV:Envoy/nipc.prefs"
#define ARC_NAME    "ENVARC:Envoy/nipc.prefs"
#define PRESET_DIR  "SYS:Prefs/Presets"
#define PRESET_NAME ""


/*****************************************************************************/

struct NIPCPrefs
{
    ULONG   np_RealmServer;
    ULONG   np_RealmName;
    UBYTE   np_UserName[16];
    UBYTE   np_AmigaName[64];
    ULONG   np_RealmServerName[64];
};

struct NIPCDevicePrefs
{
    ULONG   ndp_IPType;
    ULONG   ndp_ARPType;
    ULONG   ndp_Unit;
    UBYTE   ndp_HardAddress[16];
    UBYTE   ndp_HardString[40];
    ULONG   ndp_IPAddress;
    ULONG   ndp_IPSubnet;
    UBYTE   ndp_Flags;
    UBYTE   ndp_DevPathName[256];
};

struct NIPCDevice
{
    struct Node            nd_Node;
    struct NIPCDevicePrefs nd_Prefs;
};


#define NDPFLAGB_SUBNET   0
#define NDPFLAGB_ONLINE   1
#define NDPFLAGB_USEARP   2
#define NDPFLAGB_IPTYPE   3
#define NDPFLAGB_ARPTYPE  4
#define NDPFLAGB_HARDADDR 5

#define NDPFLAGF_SUBNET   (1L << NDPFLAGB_SUBNET)
#define NDPFLAGF_ONLINE   (1L << NDPFLAGB_ONLINE)
#define NDPFLAGF_USEARP   (1L << NDPFLAGB_USEARP)
#define NDPFLAGF_IPTYPE   (1L << NDPFLAGB_IPTYPE)
#define NDPFLAGF_ARPTYPE  (1L << NDPFLAGB_ARPTYPE)
#define NDPFLAGF_HARDADDR (1L << NDPFLAGB_HARDADDR)

struct NIPCRoutePrefs
{
    ULONG   nrp_DestNetwork;
    ULONG   nrp_Gateway;
    UWORD   nrp_Hops;
};

struct NIPCRoute
{
    struct Node             nr_Node;
    UBYTE                   nr_String[64];
    struct NIPCRoutePrefs   nr_Prefs;
};

struct NIPCRealmPrefs
{
    UBYTE   nzp_RealmName[64];
    ULONG   nzp_RealmAddr;
};

struct NIPCRealm
{
    struct Node             nz_Node;
    UBYTE                   nz_String[128];
    struct NIPCRealmPrefs   nz_Prefs;
};

struct NIPCHostPrefs
{
    UBYTE   nhp_HostName[64];
    UBYTE   nhp_RealmName[64];
    ULONG   nhp_RealmServAddr;
    UBYTE   nhp_OwnerName[32];
    ULONG   nhp_HostFlags;
};

#define NHPFLAGB_REALMS		0
#define NHPFLAGB_REALMSERVER	1

#define	NHPFLAGF_REALMS		(1L << NHPFLAGB_REALMS)
#define	NHPFLAGF_REALMSERVER	(1L << NHPFLAGB_REALMSERVER)

/*****************************************************************************/
struct ExtPrefs
{
        struct List ep_NIPCDevices;
        struct List ep_NIPCLocalRealms;
        struct List ep_NIPCRemoteRealms;
        struct List ep_NIPCRoutes;
        struct NIPCHostPrefs ep_NIPCHostPrefs;
};


/* This is the global data structure containing all global variables used by
 * the editor. The first part of the structure should not change from editor to
 * editor, while the second part can be changed to whatever the current editor
 * needs
 *
 * This structure is normally passed to you as a parameter. In exceptional
 * cases, the pointer call also be found in the tc_UserData field of the
 * task structure.
 */
typedef struct EdData
{
    /* all the libraries opened automatically by the common code */
    struct ExecBase      *ed_SysBase;
    struct Library       *ed_IntuitionBase;
    struct Library       *ed_DOSBase;
    struct Library       *ed_GfxBase;
    struct Library       *ed_AslBase;
    struct Library       *ed_IFFParseBase;
    struct Library       *ed_IconBase;
    struct Library       *ed_GadToolsBase;
    struct Library       *ed_UtilityBase;
    struct Library       *ed_LocaleBase;
    struct Library       *ed_WorkbenchBase;

    /* execution environment details, can be looked at anytime */
    struct RDArgs        *ed_RdArgs;
    ULONG                 ed_Args[OPT_COUNT];
    char                  ed_ProgramPath[PROGPATHSIZE];
    STRPTR                ed_PubScreenName;
    char                  ed_ScreenNameBuf[32];
    APTR                  ed_OldWindowPtr;

    /* message port used to detect other running instances of this program.
     * This is used by the common code, do not touch
     */
    struct MsgPort        ed_RendezVous;

    /* data needed for the AppWindow and AppMenu */
    struct MsgPort        ed_AppPort;
    struct AppWindow     *ed_AppWindow;
    struct AppMenu       *ed_AppMenu;

    /* another private field used by the common code's gadget creation
     * function
     */
    struct Gadget        *ed_LastAdded;

    /* display attributes for this editor */
    struct Window        *ed_Window;
    struct Screen        *ed_Screen;
    struct Menu          *ed_Menus;
    struct Gadget        *ed_Gadgets;
    struct Font          *ed_Font;
    struct DrawInfo      *ed_DrawInfo;
    struct FileRequester *ed_FileReq;
    APTR                  ed_VisualInfo;
    struct TextAttr       ed_TextAttr;

    /* when processing a command, this will point to the Intuition message
     * that caused this command to be executed. Do NOT reply this message
     * yourself, the common code does this
     */
    struct IntuiMessage   ed_CurrentMsg;

    /* various read/write state flags */
    BOOL                  ed_Quit;           /* set to TRUE to quit */
    BOOL                  ed_Cancelled;
    BOOL                  ed_InitFail;       /* set to TRUE when InitEdData fails */
    BOOL                  ed_CheckCmdState;  /* tell the main code to validate the state of menus and gadgets */
    BOOL                  ed_SaveIcons;      /* should we save icons? */
    LONG                  ed_SecondaryResult;/* pr_Result2 for this program */
    STRPTR                ed_ErrorFileName;  /* name for IO error displays */

    /* this should be changed to be of the current editor's favorite type */
    struct ExtPrefs       ed_PrefsDefaults;
    struct ExtPrefs       ed_PrefsWork;
    struct ExtPrefs       ed_PrefsInitial;

    /* used by file requester calls */
    char                  ed_NameBuf[NAMEBUFSIZE];

    /* information used for localization */
    struct LocaleInfo     ed_LocaleInfo;

    /* the rest of this structure is editor-specific and is not used by the
     * common code
     */

    UBYTE                 ed_Str1[20];
    UBYTE                 ed_Str2[20];
    UBYTE                 ed_SubnetStr[20];
    UBYTE                 ed_AddressStr[40];
    struct NIPCDevice    *ed_CurrentDevice;
    struct NIPCRealm     *ed_CurrentLocRealm;
    struct NIPCRealm     *ed_CurrentRemRealm;
    struct NIPCRoute     *ed_CurrentRoute;
    struct FileRequester *ed_DevReq;
    struct Hook		  ed_IPGadHook;
    STRPTR                ed_PanelLabels[5];
    STRPTR                ed_StatusLabels[3];
    ULONG		  ed_V39;

    /* NIPC Configuration Main Panel */

    struct Gadget        *ed_MainHostName;
    struct Gadget        *ed_MainRealmName;
    struct Gadget        *ed_MainRealmAddress;
    struct Gadget	 *ed_MainUseRealms;
    struct Gadget	 *ed_MainIsRealmServer;
    struct Gadget	 *ed_MainOwnerName;
    struct Gadget        *ed_PanelGad;

    /* NIPC Routing Panel */

    struct Gadget        *ed_RouteDestination;
    struct Gadget        *ed_RouteGateway;
    struct Gadget        *ed_RouteHops;
    struct Gadget        *ed_RouteList;
    struct Gadget        *ed_RouteDelete;

    /* NIPC Realm Configuration Panel */

    struct Gadget        *ed_LocalRealmList;
    struct Gadget        *ed_LocalRealmName;
    struct Gadget        *ed_LocalRealmAddress;
    struct Gadget        *ed_LocalRealmDelete;
    struct Gadget        *ed_RemoteRealmList;
    struct Gadget        *ed_RemoteRealmName;
    struct Gadget        *ed_RemoteRealmAddress;
    struct Gadget        *ed_RemoteRealmDelete;

    /* NIPC Network Devices Panel */

    struct Gadget        *ed_DeviceStatus;
    struct Gadget        *ed_DeviceUnit;
    struct Gadget        *ed_DeviceIPType;
    struct Gadget        *ed_DeviceARPType;
    struct Gadget        *ed_DeviceSubnetAble;
    struct Gadget        *ed_DeviceAddressAble;
    struct Gadget        *ed_DeviceIPTypeAble;
    struct Gadget        *ed_DeviceARPTypeAble;
    struct Gadget        *ed_DevicePathName;
    struct Gadget        *ed_DeviceIP;
    struct Gadget        *ed_DeviceMask;
    struct Gadget        *ed_DeviceAddress;
    struct Gadget        *ed_DeviceInfo;
    struct Gadget        *ed_DeviceList;
    struct Gadget	 *ed_DeviceAdd;
    struct Gadget        *ed_DeviceDelete;

    /* Misc */

    UWORD                 ed_Panel;
    BOOL                  ed_Write;
} EdData;

typedef EdData *EdDataPtr;

#define DOSBase       ed->ed_DOSBase
#define IntuitionBase ed->ed_IntuitionBase
#define AslBase       ed->ed_AslBase
#define IFFParseBase  ed->ed_IFFParseBase
#define IconBase      ed->ed_IconBase
#define GfxBase       ed->ed_GfxBase
#define GadToolsBase  ed->ed_GadToolsBase
#define UtilityBase   ed->ed_UtilityBase
#define LocaleBase    ed->ed_LocaleBase
#define WorkbenchBase ed->ed_WorkbenchBase
#define window        ed->ed_Window


/* state the editor can be in, ES_NORMAL to ES_DOSERROR are required,
 * everything else is optional
 */
typedef enum EdStatus
{
    ES_NORMAL,
    ES_NO_MEMORY,
    ES_IFF_UNKNOWNCHUNK,
    ES_IFF_UNKNOWNVERSION,
    ES_IFFERROR,
    ES_DOSERROR
} EdStatus;


/* commands this editor supports */
typedef enum EdCommand
{
    /* standard prefs editor commands */
    EC_NOP,
    EC_OPEN,
    EC_SAVE,
    EC_SAVEAS,
    EC_LASTSAVED,
    EC_RESTORE,
    EC_RESETALL,
    EC_SAVEICONS,
    EC_USE,
    EC_CANCEL,
    EC_ACTIVATE,

    /* custom editor commands */

        /* Main Panel */

        EC_MAIN_HOSTNAME,
        EC_MAIN_REALMNAME,
        EC_MAIN_REALMADDR,
        EC_MAIN_DEVICES,
        EC_MAIN_ROUTES,
        EC_MAIN_OWNERNAME,
        EC_MAIN_REALMS,
        EC_MAIN_REALMSERVER,

        /* Devices Panel */

        EC_DEVICE_LIST,
        EC_DEVICE_NAME,
        EC_DEVICE_ADD,
        EC_DEVICE_DEL,
        EC_DEVICE_UNIT,
        EC_DEVICE_IP,
        EC_DEVICE_MASK,
        EC_DEVICE_ADDR,
        EC_DEVICE_IPTYPE,
        EC_DEVICE_ARPTYPE,
        EC_DEVICE_STATUS,
        EC_DEVICE_MASK_ABLE,
        EC_DEVICE_IPTYPE_ABLE,
        EC_DEVICE_ARPTYPE_ABLE,
        EC_DEVICE_ADDR_ABLE,
        EC_DEVICE_ACCEPT,
        EC_DEVICE_HELP,
        EC_DEVICE_CANCEL,

        /* Routing Panel */

        EC_ROUTE_LIST,
        EC_ROUTE_DEST,
        EC_ROUTE_GATEWAY,
        EC_ROUTE_HOPS,
        EC_ROUTE_ADD,
        EC_ROUTE_DELETE,
        EC_ROUTE_ACCEPT,
        EC_ROUTE_HELP,
        EC_ROUTE_CANCEL,

        /* Realms Panel */

        EC_LOCREALM_LIST,
        EC_LOCREALM_NAME,
        EC_LOCREALM_NET,
        EC_LOCREALM_ADD,
        EC_LOCREALM_DELETE,

        EC_REMREALM_LIST,
        EC_REMREALM_NAME,
        EC_REMREALM_NET,
        EC_REMREALM_ADD,
        EC_REMREALM_DELETE,

        EC_REALM_ACCEPT,
        EC_REALM_HELP,
        EC_REALM_CANCEL,
        EC_PANEL

} EdCommand;


#define PANEL_MAIN      0
#define PANEL_DEVICES   1
#define PANEL_ROUTES    2
#define PANEL_REALMS    3

/* this describes the state of an editor command */
typedef struct CmdState
{
    BOOL cs_Available;
    BOOL cs_Selected;
} CmdState;

typedef CmdState *CmdStatePtr;


/*****************************************************************************/

#define ASM __asm
#define REG(x) register __## x

EdStatus InitEdData(EdDataPtr ed);
VOID InitPrefs(struct ExtPrefs *prefs);
VOID CleanUpEdData(EdDataPtr ed);
VOID FreePrefs(EdDataPtr ed, struct ExtPrefs *prefs);
VOID FreeList(EdDataPtr ed, struct List *list, ULONG nodeSize);
VOID ClearGadPtrs(EdDataPtr ed);

EdStatus OpenPrefs(EdDataPtr ed, STRPTR name);
EdStatus SavePrefs(EdDataPtr ed, STRPTR name);

BOOL CreateDisplay(EdDataPtr ed);
VOID DisposeDisplay(EdDataPtr ed);
VOID MakeNewDisplay(EdDataPtr ed);
VOID RenderDisplay(EdDataPtr ed);
VOID RenderGadgets(EdDataPtr ed);

VOID ProcessSpecialCommand(EdDataPtr ed, EdCommand ec);
BOOL StrtoIP(STRPTR ipstr, ULONG *ipnum);
VOID IPToStr(ULONG ipnum, STRPTR ipstr);
char nibvert(char ichar);
EdCommand GetCommand(EdDataPtr ed);
VOID GetSpecialCmdState(EdDataPtr ed, EdCommand ec, CmdStatePtr state);
BOOL DeviceRequest(EdDataPtr ed);
BOOL RequestDevice(EdDataPtr ed, ULONG tag1, ...);
VOID AddDeviceReq(EdDataPtr ed);

VOID ASM IntuiHook(REG(a0) struct Hook *hook,
                   REG(a2) struct FileRequester *fr,
                   REG(a1) struct IntuiMessage *intuiMsg);

ULONG ASM StringHook(REG(a0) struct Hook *hook,
                   REG(a2) struct SGWork *sgw,
                   REG(a1) ULONG *msg);

BOOL IsIPDigit(UBYTE test_char);
VOID CopyPrefs(EdDataPtr ed, void *p1, void *p2);

#define ProcessArgs(ed,diskObj) {}
#define InitDisp(ed)            (TRUE)
#define SyncTextGadgets(ed)     {}
#define RenderDisplay(ed)               {}
#define GetCommand(ed)          (EC_NOP)

#endif /* PE_CUSTOM_H */