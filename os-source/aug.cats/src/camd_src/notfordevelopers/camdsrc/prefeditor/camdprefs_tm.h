
/* Include file generated by Toolmaker V1.13 */

#include "CAMDPrefs_tm_text.h"

struct TMScreenInfo
  {
  struct Screen *Screen;
  struct TagItem *MoreTags;
  struct VisualInfo *VisualInfo;
  APTR UserData;
  };

struct TMWindowInfo
  {
  struct Window *Window;
  struct TagItem *MoreTags;
  struct IntuiMessage *GT_IMsg;
  struct Menu *Menu;
  struct Requester Requester;
  UBYTE DisableCount;
  ULONG Flags;
  struct Gadget *FirstGadget;
  struct Gadget *ContextGadget;
  APTR UserData;
  };

struct TMGadgetInfo
  {
  struct Gadget *Gadget;
  struct TagItem *MoreTags;
  APTR UserData;
  };

struct TMData
  {
  ULONG Size;
  struct MsgPort *WindowMsgPort;
  LONG ReturnCode;
  struct FileRequester *FileRequester;
  struct FontRequester *FontRequester;
  struct TMScreenInfo TMScreenInfo[1];
  struct TMGadgetInfo TMGadgetInfo_MIDIPREF[13];
  struct TMWindowInfo TMWindowInfo[1];
  UWORD *WaitPointer;
  APTR UserData;
  };

struct TMData *TM_Open(ULONG *);
VOID TM_Close(struct TMData *);
LONG TM_Request(struct Window *, UBYTE *, UBYTE *, UBYTE *, ULONG *, APTR, ...);
VOID TM_EventLoop(struct TMData *);
BOOL OpenScreen_Workbench(struct TMData *);
VOID CloseScreen_Workbench(struct TMData *);
BOOL OpenWindow_MIDIPREF(struct TMData *);
VOID CloseWindow_MIDIPREF(struct TMData *);
VOID DisableWindow_MIDIPREF(struct TMData *);
VOID EnableWindow_MIDIPREF(struct TMData *);
BOOL Window_MIDIPREF_MENUPICK(struct TMData *, struct IntuiMessage *);
BOOL Window_MIDIPREF_GADGETDOWN(struct TMData *, struct IntuiMessage *);
BOOL Window_MIDIPREF_GADGETUP(struct TMData *, struct IntuiMessage *);

extern struct Library *SysBase;
extern struct Library *DOSBase;
extern struct Library *IntuitionBase;
extern struct Library *GadToolsBase;
extern struct Library *UtilityBase;
extern struct Library *AslBase;

#define OPENFILE	0
#define SAVEFILE	1

#define TMWF_OPENED	0x00000001
#define TMWF_DISABLED	0x00000002

#define TMERR_OK	0
#define TMERR_MEMORY	1
#define TMERR_MSGPORT	2

#define screen_Workbench	TMData->TMScreenInfo[0].Screen

#define window_MIDIPREF	TMData->TMWindowInfo[0].Window
#define menu_MIDIPREF	TMData->TMWindowInfo[0].Menu
#define gadget_DESELECT	TMData->TMGadgetInfo_MIDIPREF[0].Gadget
#define gadget_SELECT	TMData->TMGadgetInfo_MIDIPREF[1].Gadget
#define gadget_DRIVERSI	TMData->TMGadgetInfo_MIDIPREF[2].Gadget
#define gadget_SYSEXQUE	TMData->TMGadgetInfo_MIDIPREF[3].Gadget
#define gadget_MSGQUEUS	TMData->TMGadgetInfo_MIDIPREF[4].Gadget
#define gadget_CANCEL	TMData->TMGadgetInfo_MIDIPREF[5].Gadget
#define gadget_USE	TMData->TMGadgetInfo_MIDIPREF[6].Gadget
#define gadget_SAVE	TMData->TMGadgetInfo_MIDIPREF[7].Gadget
#define gadget_COMMENT	TMData->TMGadgetInfo_MIDIPREF[8].Gadget
#define gadget_OUTPUTNA	TMData->TMGadgetInfo_MIDIPREF[9].Gadget
#define gadget_INPUTNAM	TMData->TMGadgetInfo_MIDIPREF[10].Gadget
#define gadget_IDSTRING	TMData->TMGadgetInfo_MIDIPREF[11].Gadget
#define gadget_DRIVER	TMData->TMGadgetInfo_MIDIPREF[12].Gadget

#define ID_DESELECT	1
#define ID_SELECT	2
#define ID_DRIVERSI	3
#define ID_SYSEXQUE	4
#define ID_MSGQUEUS	5
#define ID_CANCEL	6
#define ID_USE	7
#define ID_SAVE	8
#define ID_COMMENT	9
#define ID_OUTPUTNA	10
#define ID_INPUTNAM	11
#define ID_IDSTRING	12
#define ID_DRIVER	13

#define MENU_PROJECT	0
#define ITEM_OPEN	0
#define ITEM_SAVEAS	1
#define ITEM_ABOUT	3
#define ITEM_QUIT	5
#define MENU_EDIT	1
#define ITEM_RESET	0
#define ITEM_LASTSAVE	1
#define ITEM_RESTORE	2
#define MENU_SETTINGS	2
#define ITEM_ICONS	0

#define ScreenInfo_Workbench	TMData->TMScreenInfo[0]

#define WindowInfo_MIDIPREF	TMData->TMWindowInfo[0]

#define GadgetInfo_DESELECT	TMData->TMGadgetInfo_MIDIPREF[0]
#define GadgetInfo_SELECT	TMData->TMGadgetInfo_MIDIPREF[1]
#define GadgetInfo_DRIVERSI	TMData->TMGadgetInfo_MIDIPREF[2]
#define GadgetInfo_SYSEXQUE	TMData->TMGadgetInfo_MIDIPREF[3]
#define GadgetInfo_MSGQUEUS	TMData->TMGadgetInfo_MIDIPREF[4]
#define GadgetInfo_CANCEL	TMData->TMGadgetInfo_MIDIPREF[5]
#define GadgetInfo_USE	TMData->TMGadgetInfo_MIDIPREF[6]
#define GadgetInfo_SAVE	TMData->TMGadgetInfo_MIDIPREF[7]
#define GadgetInfo_COMMENT	TMData->TMGadgetInfo_MIDIPREF[8]
#define GadgetInfo_OUTPUTNA	TMData->TMGadgetInfo_MIDIPREF[9]
#define GadgetInfo_INPUTNAM	TMData->TMGadgetInfo_MIDIPREF[10]
#define GadgetInfo_IDSTRING	TMData->TMGadgetInfo_MIDIPREF[11]
#define GadgetInfo_DRIVER	TMData->TMGadgetInfo_MIDIPREF[12]

extern TMOBJECTDATA tmobjectdata_OPEN;
extern TMOBJECTDATA tmobjectdata_SAVEAS;
extern TMOBJECTDATA tmobjectdata_ABOUT;
extern TMOBJECTDATA tmobjectdata_QUIT;
extern TMOBJECTDATA tmobjectdata_RESET;
extern TMOBJECTDATA tmobjectdata_LASTSAVE;
extern TMOBJECTDATA tmobjectdata_RESTORE;
extern TMOBJECTDATA tmobjectdata_ICONS;

extern TMOBJECTDATA tmobjectdata_DESELECT;
extern TMOBJECTDATA tmobjectdata_SELECT;
extern TMOBJECTDATA tmobjectdata_DRIVERSI;
extern TMOBJECTDATA tmobjectdata_SYSEXQUE;
extern TMOBJECTDATA tmobjectdata_MSGQUEUS;
extern TMOBJECTDATA tmobjectdata_CANCEL;
extern TMOBJECTDATA tmobjectdata_USE;
extern TMOBJECTDATA tmobjectdata_SAVE;
extern TMOBJECTDATA tmobjectdata_COMMENT;
extern TMOBJECTDATA tmobjectdata_OUTPUTNA;
extern TMOBJECTDATA tmobjectdata_INPUTNAM;
extern TMOBJECTDATA tmobjectdata_IDSTRING;
extern TMOBJECTDATA tmobjectdata_DRIVER;
