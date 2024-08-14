
/****************************************************************************/
/*                                                                          */
/*  Externs.c - All external variables are here                             */
/*                                                                          */
/****************************************************************************/

#include "Toolmaker.h"

#define TITLE   "Toolmaker"
#define VERSION "1.19"
#define DATE    "(8.7.92)"

char  title[] = TITLE;
char  date[] = DATE;
char  cbmdate[] = "1992";    /* for © */
char  merdate[] = "1991";    /* for © */
char  extension[] = ".tm";
char  template[] = "FILE,SETTINGS/K";

#ifdef DEMO
char  version[] = VERSION "Demo";
char  verstring[] = "\0$VER: " TITLE "Demo " VERSION " " DATE;
#else
char  version[] = VERSION;
char  verstring[] = "\0$VER: " TITLE " " VERSION " " DATE;
#endif

#ifdef TEST
UBYTE debuglevel;
UBYTE debugflag;
ULONG totalmem;
FILE  *confp;
#endif

LONG valuecode;
LONG availablecode;
LONG selectedcode;
LONG screenmodecode;
LONG ntscpalcode;
LONG oscancode;

ULONG maxdepth;
ULONG F_Main;
ULONG mainleft;
ULONG maintop;
ULONG viewwidth;
ULONG viewheight;
ULONG selectedstringnum;
ULONG selectedmenunum;
ULONG selecteditemnum;
ULONG selectedsubnum;
ULONG selectedobject;
ULONG currentcolor;
ULONG currentred;
ULONG currentgreen;
ULONG currentblue;
ULONG mainsignal;
ULONG editsignal;
ULONG IDCMPsignal;
ULONG menusignal;
ULONG screensignal;
ULONG colorssignal;
ULONG arraysignals;

ULONG defaultcolors[32];
ULONG argarray[4];
ULONG edittagtype;
ULONG positionvalues[]  = {PLACETEXT_LEFT, PLACETEXT_RIGHT, PLACETEXT_ABOVE, PLACETEXT_BELOW, PLACETEXT_IN, NULL};

ULONG constantvalues[] =
  {
  0, /* TITLEBARHEIGHT */
  PLACETEXT_LEFT,
  PLACETEXT_RIGHT,
  PLACETEXT_ABOVE,
  PLACETEXT_BELOW,
  PLACETEXT_IN,
  LORIENT_VERT,
  LORIENT_HORIZ,
  GACT_STRINGCENTER,
  GACT_STRINGLEFT,
  GACT_STRINGRIGHT,
  TRUE,
  FALSE
  };

ULONG mainidcmp =   IDCMP_REFRESHWINDOW |
                    IDCMP_CLOSEWINDOW |
                    IDCMP_MOUSEMOVE |
                    IDCMP_MENUPICK |
                    IDCMP_ACTIVEWINDOW |
                    IDCMP_CHANGEWINDOW |
                    IDCMP_VANILLAKEY |
                    MXIDCMP |
                    CHECKBOXIDCMP;

ULONG editidcmp =   IDCMP_REFRESHWINDOW |
                    IDCMP_VANILLAKEY |
                    LISTVIEWIDCMP |
                    SCROLLERIDCMP |
                    SLIDERIDCMP |
                    STRINGIDCMP |
                    BUTTONIDCMP;

ULONG IDCMPidcmp =  IDCMP_REFRESHWINDOW |
                    IDCMP_VANILLAKEY |
                    CHECKBOXIDCMP |
                    BUTTONIDCMP;

ULONG menuidcmp =   IDCMP_REFRESHWINDOW |
                    IDCMP_VANILLAKEY |
                    LISTVIEWIDCMP |
                    CHECKBOXIDCMP |
                    STRINGIDCMP |
                    BUTTONIDCMP;

ULONG screenidcmp = IDCMP_REFRESHWINDOW |
                    IDCMP_VANILLAKEY |
                    CHECKBOXIDCMP |
                    MXIDCMP |
                    SLIDERIDCMP |
                    BUTTONIDCMP;

ULONG arrayidcmp =  IDCMP_REFRESHWINDOW |
                    IDCMP_MOUSEBUTTONS |
                    IDCMP_CHANGEWINDOW |
                    IDCMP_MENUPICK |
                    IDCMP_INACTIVEWINDOW |
                    IDCMP_VANILLAKEY |
                    IDCMP_ACTIVEWINDOW;

ULONG colorsidcmp = IDCMP_REFRESHWINDOW |
                    IDCMP_VANILLAKEY |
                    SLIDERIDCMP |
                    PALETTEIDCMP |
                    BUTTONIDCMP;

ULONG gadgetidcmp = ARROWIDCMP |
                    BUTTONIDCMP |
                    CHECKBOXIDCMP |
                    INTEGERIDCMP |
                    LISTVIEWIDCMP |
                    MXIDCMP |
                    NUMBERIDCMP |
                    CYCLEIDCMP |
                    PALETTEIDCMP |
                    SCROLLERIDCMP |
                    SLIDERIDCMP |
                    STRINGIDCMP |
                    TEXTIDCMP;

APTR  VisualInfo;
APTR  request;
APTR  oldwindowptr;

UBYTE mainwindowpri;
UBYTE currenttopborder;
UBYTE modified;
UBYTE realgadgets;
UBYTE realmenus;
UBYTE commentflag;
UBYTE autotopborderflag;
UBYTE gridsize;
UBYTE pragmaflag;
UBYTE openflag;
UBYTE iconflag;
UBYTE newprojectflag;
UBYTE usersignalflag;
UBYTE arexxflag;
UBYTE selectflag;
UBYTE onegadgetflag;
UBYTE chipflag;
UBYTE userdataflag;
UBYTE aslflag;
UBYTE menupart;
UBYTE sleepcount;
UBYTE actuallymove;

SHORT topborderadjust;
SHORT textpos;
SHORT stringcount;
SHORT windowcount;
SHORT menucount;
SHORT itemcount;
SHORT subcount;
SHORT availablecount;
SHORT selectedcount;
SHORT xmark, ymark;
SHORT xstart, ystart;
SHORT xend, yend;
SHORT gadgetkind;
SHORT drag=DRAGNONE;
SHORT leftbound;
SHORT topbound;
SHORT rightbound;
SHORT bottombound;
SHORT diskfontcount;

struct Library *IntuitionBase;
struct Library *GfxBase;
struct Library *GadToolsBase;
struct Library *AslBase;
struct Library *IconBase;
struct Library *DiskfontBase;
struct Library *UtilityBase;
struct Library *IFFParseBase;

struct Window *W_Main;
struct Window *W_Edit;
struct Window *W_IDCMP;
struct Window *W_Menu;
struct Window *W_Screen;
struct Window *W_Colors;

struct Menu   *M_Main;

struct Gadget *G_Main;
struct Gadget *G_MainContext;
struct Gadget *G_GadgetList;
struct Gadget *G_TestGadgets;
struct Gadget *G_TestMenus;

struct Gadget *G_AvailableTags;
struct Gadget *G_SelectedTags;
struct Gadget *G_String;
struct Gadget *G_TagLabel;
struct Gadget *G_ListView;
struct Gadget *G_Title;
struct Gadget *G_Label;
struct Gadget *G_TextPos;
struct Gadget *G_TagAdd;
struct Gadget *G_TagDel;
struct Gadget *G_TagUp;
struct Gadget *G_TagDown;
struct Gadget *G_TagType;
struct Gadget *G_Use;
struct Gadget *G_Remove;
struct Gadget *G_TopEdge;
struct Gadget *G_LeftEdge;
struct Gadget *G_Width;
struct Gadget *G_Height;
struct Gadget *G_Highlight;

struct Gadget *G_MenuList;
struct Gadget *G_MenuString;
struct Gadget *G_MenuLabel;
struct Gadget *G_MenuAdd;
struct Gadget *G_MenuDel;
struct Gadget *G_MenuUp;
struct Gadget *G_MenuDown;
struct Gadget *G_MenuDisabled;
struct Gadget *G_ItemList;
struct Gadget *G_ItemString;
struct Gadget *G_ItemLabel;
struct Gadget *G_ItemAdd;
struct Gadget *G_ItemDel;
struct Gadget *G_ItemUp;
struct Gadget *G_ItemDown;
struct Gadget *G_ItemDisabled;
struct Gadget *G_SubList;
struct Gadget *G_SubString;
struct Gadget *G_SubLabel;
struct Gadget *G_SubAdd;
struct Gadget *G_SubDel;
struct Gadget *G_SubUp;
struct Gadget *G_SubDown;
struct Gadget *G_SubDisabled;
struct Gadget *G_CheckIt;
struct Gadget *G_Checked;
struct Gadget *G_Toggle;
struct Gadget *G_CommKey;
struct Gadget *G_Exclude;

struct Gadget *G_ScreenMode;
struct Gadget *G_NtscPal;
struct Gadget *G_Overscan;
struct Gadget *G_Interlace;
struct Gadget *G_ScreenPalette;
struct Gadget *G_ScreenRed;
struct Gadget *G_ScreenGreen;
struct Gadget *G_ScreenBlue;
struct Gadget *G_ScreenWidth;
struct Gadget *G_ScreenHeight;
struct Gadget *G_ScreenDepth;
struct Gadget *G_ScreenDWidth;
struct Gadget *G_ScreenDHeight;
struct Gadget *G_ScreenTop;
struct Gadget *G_ScreenLeft;
struct Gadget *G_CustomColors;

struct Gadget *G_SIZEVERIFY;
struct Gadget *G_NEWSIZE;
struct Gadget *G_REFRESHWINDOW;
struct Gadget *G_MOUSEBUTTONS;
struct Gadget *G_MOUSEMOVE;
struct Gadget *G_GADGETDOWN;
struct Gadget *G_GADGETUP;
struct Gadget *G_REQSET;
struct Gadget *G_MENUPICK;
struct Gadget *G_CLOSEWINDOW;
struct Gadget *G_RAWKEY;
struct Gadget *G_REQVERIFY;
struct Gadget *G_REQCLEAR;
struct Gadget *G_MENUVERIFY;
struct Gadget *G_NEWPREFS;
struct Gadget *G_DISKINSERTED;
struct Gadget *G_DISKREMOVED;
struct Gadget *G_ACTIVEWINDOW;
struct Gadget *G_INACTIVEWINDOW;
struct Gadget *G_DELTAMOVE;
struct Gadget *G_VANILLAKEY;
struct Gadget *G_INTUITICKS;
struct Gadget *G_IDCMPUPDATE;
struct Gadget *G_MENUHELP;
struct Gadget *G_CHANGEWINDOW;

struct List WindowList;
struct List AvailableTagsList;
struct List SelectedTagsList;
struct List EditMenuList;
struct List TextAttrList;
struct List FontList;
struct List EmptyList;

struct NewGadgetNode *currentgadget;

struct WindowNode *currentwindow;
struct WindowNode *selectwindow;

struct EditTagNode *availabletag;
struct EditTagNode *selectedtag;

struct MenuNode *selectedmenu;
struct ItemNode *selecteditem;
struct SubNode  *selectedsub;

/*****************************/

USHORT chip SleepPointer[] =
  {
  0x0000, 0x0000,

  0x0400, 0x07C0,
  0x0000, 0x07C0,
  0x0100, 0x0380,
  0x0000, 0x07E0,
  0x07C0, 0x1FF8,
  0x1FF0, 0x3FEC,
  0x3FF8, 0x7FDE,
  0x3FF8, 0x7FBE,
  0x7FFC, 0xFF7F,
  0x7EFC, 0xFFFF,
  0x7FFC, 0xFFFF,
  0x3FF8, 0x7FFE,
  0x3FF8, 0x7FFE,
  0x1FF0, 0x3FFC,
  0x07C0, 0x1FF8,
  0x0000, 0x07E0,

  0x0000, 0x0000
  };

struct List far defaultlist;
struct Node far defaultnode[3];

struct DimensionInfo far dimensioninfo;

struct EasyStruct far easystruct = {sizeof(struct EasyStruct), 0, title, NULL, NULL};

struct Requester far Requester;
struct Requester far R_Main;

struct TextAttr far TOPAZ80 = {(STRPTR)"topaz.font",TOPAZ_EIGHTY,0,0};

struct ScreenNode far screennode;
struct ScreenNode far newscreennode;

WORD far defaultzoom[] = {4, 10,20,200,100};
WORD far defaultpens[] = {1, -1};
char far *defaultstrings[] = {"first", "second", "third"};
struct StringData far defaultlabels = {3, defaultstrings};

/*
extern struct List defaultsourcelabellist;
extern struct Node sourcelabelnode0;
extern struct Node sourcelabelnode1;
extern struct Node sourcelabelnode2;

struct List defaultsourcelabellist = {&sourcelabelnode0, NULL, &sourcelabelnode2, 0, 0};
struct Node sourcelabelnode0 = {&sourcelabelnode1,               &defaultsourcelabellist.lh_Head, 0, 0, ""};
struct Node sourcelabelnode1 = {&sourcelabelnode2,               &sourcelabelnode0,               0, 0, ""};
struct Node sourcelabelnode2 = {&defaultsourcelabellist.lh_Tail, &sourcelabelnode1,               0, 0, ""};
*/

char far *gadgettext[] =
  {
  "(S_elect)",
  "_Button",
  "C_heckBox",
  "_Cycle",
  "_Integer",
  "_ListView",
  "M_x",
  "_Number",
  "_Palette",
  "Sc_roller",
  "Sli_der",
  "_String",
  "_Text",
  NULL
  };

char far *constantlabels[] =
  {
  "TITLEBARHEIGHT",
  "PLACETEXT_LEFT",
  "PLACETEXT_RIGHT",
  "PLACETEXT_ABOVE",
  "PLACETEXT_BELOW",
  "PLACETEXT_IN",
  "LORIENT_VERT",
  "LORIENT_HORIZ",
  "GACT_STRINGCENTER",
  "GACT_STRINGLEFT",
  "GACT_STRINGRIGHT",
  "TRUE",
  "FALSE"
  };

char far *positionlabels[] =
  {
  "LEFT",
  "RIGHT",
  "ABOVE",
  "BELOW",
  "IN",
  NULL
  };

char far *modelabels[] =
  {
  "WORK_BENCH",
  "HI_RES",
  "S_UPERHIRES",
  "PRODUCTIVIT_Y",
  "A2024 1_0Hz",
  "A2024 1_5Hz",
  NULL
  };

char far *ntscpallabels[] =
  {
  "DEFAULT",
  "NTSC",
  "PAL",
  NULL
  };

char far *oscanlabels[] =
  {
  "TEXT",
  "STANDARD",
  "MAX",
  "VIDEO",
  NULL
  };

struct EditItem far projectitem[] =
  {
    {"New",          "N", "NEW",     0},
    {"Open...",      "O", "OPEN",    0},
    {"---",          "",  "",        0},
    {"Save",         "S", "SAVE",    0},
    {"SaveAs...",    "A", "SAVEAS",  0},
    {"---",          "",  "",        0},
    {"Print",        "P", "PRINT",   0},
    {"PrintAs...",   "",  "PRINTAS", 0},
    {"---",          "",  "",        0},
    {"Hide",         "",  "HIDE",    0},
    {"Reveal...",    "",  "REVEAL",  0},
    {"Close...",     "",  "CLOSE",   0},
    {"---",          "",  "",        0},
    {"About...",     "",  "ABOUT",   0},
    {"---",          "",  "",        0},
    {"Exit Level",   "E", "EXIT",    0},
    {"---",          "",  "",        0},
    {"Quit Program", "Q", "QUIT",    0}
  };

struct EditItem far edititem[] =
  {
    {"Cut",   "X", "CUT",   0},
    {"Copy",  "C", "COPY",  0},
    {"Paste", "V", "PASTE", 0},
    {"---",   "",  "",      0},
    {"Erase", "",  "ERASE", 0},
    {"---",   "",  "",      0},
    {"Undo",  "Z", "UNDO",  0},
    {"Redo",  "",  "REDO",  0}
  };

struct EditItem far macroitem[] =
  {
    {"Start Learning",  "", "STARTLRN", 0},
    {"Stop Learning",   "", "STOPLRN",  0},
    {"Assign Macro...", "", "ASSIGN",   0},
    {"Load...",         "", "LOADMAC",  0},
    {"Save...",         "", "SAVEMAC",  0}
  };

struct EditItem far settingitem[] =
  {
    {"[option]",            "", "OPTION",   CHECKIT|MENUTOGGLE},
    {"Create Icons",        "", "ICONS",    CHECKIT|MENUTOGGLE},
    {"---",                 "", "",         0},
    {"Load Settings...",    "", "LOADSET",  0},
    {"Save Settings",       "", "SAVSET",   0},
    {"Save Settings As...", "", "SAVSETAS", 0}
  };

struct EditItem far useritem[] =
  {
    {"Macro #1", "", "MACRO1", 0},
    {"Macro #2", "", "MACRO2", 0},
    {"Macro #3", "", "MACRO3", 0},
    {"Macro #4", "", "MACRO4", 0}
  };

char far defaulttool[PATHSIZE+FILESIZE];
char far filename[PATHSIZE+FILESIZE];
char far settingsname[PATHSIZE+FILESIZE];
char far fullname[PATHSIZE+FILESIZE];
char far savefilename[FILESIZE];
char far savepathname[PATHSIZE];
char far settingsfilename[FILESIZE];
char far settingspathname[PATHSIZE];
char far string[1024];
char far string2[1024];
char far tabstring[1024];
char far toolpath[PATHSIZE];

struct TagItem far tagarray[MAXTAGS];

struct NewMenu far NM_Main[] =
  {
   {NM_TITLE,    "Project",                   NULL, NULL,                       NULL, NULL},
   {NM_ITEM,       "New ",                    "N",  NULL,                       NULL, NULL},
   {NM_ITEM,       "Open... ",                "O",  NULL,                       NULL, NULL},
   {NM_ITEM,       NM_BARLABEL,               NULL, NULL,                       NULL, NULL},
#ifdef DEMO
   {NM_ITEM,       "Save ",                   "S",  NM_ITEMDISABLED,            NULL, NULL},
   {NM_ITEM,       "Save As... ",             "A",  NM_ITEMDISABLED,            NULL, NULL},
#else
   {NM_ITEM,       "Save ",                   "S",  NULL,                       NULL, NULL},
   {NM_ITEM,       "Save As... ",             "A",  NULL,                       NULL, NULL},
#endif
   {NM_ITEM,       NM_BARLABEL,               NULL, NULL,                       NULL, NULL},
#ifdef DEMO
   {NM_ITEM,       "Generate Code ",          ">",  NM_ITEMDISABLED,            NULL, NULL},
#else
   {NM_ITEM,       "Generate Code ",          ">",  NULL,                       NULL, NULL},
#endif
   {NM_ITEM,       NM_BARLABEL,               NULL, NULL,                       NULL, NULL},
   {NM_ITEM,       "About... ",               "H",  NULL,                       NULL, NULL},
   {NM_ITEM,       NM_BARLABEL,               NULL, NULL,                       NULL, NULL},
   {NM_ITEM,       "Quit ",                   "Q",  NULL,                       NULL, NULL},

   {NM_TITLE,    "Screen",                    NULL, NULL,                       NULL, NULL},
   {NM_ITEM,       "Mode... ",                "Z",  NULL,                       NULL, NULL},
   {NM_ITEM,       "Attributes... ",          "T",  NM_ITEMDISABLED,            NULL, NULL},
   {NM_ITEM,       "Palette... ",             "P",  NM_ITEMDISABLED,            NULL, NULL},
   {NM_ITEM,       "Font... ",                NULL, NM_ITEMDISABLED,            NULL, NULL},

   {NM_TITLE,    "Window",                    NULL, NULL,                       NULL, NULL},
   {NM_ITEM,       "Attributes... ",          "E",  NM_ITEMDISABLED,            NULL, NULL},
   {NM_ITEM,       "IDCMP... ",               "I",  NM_ITEMDISABLED,            NULL, NULL},
   {NM_ITEM,       "Menus... ",               "M",  NM_ITEMDISABLED,            NULL, NULL},
   {NM_ITEM,       NM_BARLABEL,               NULL, NULL,                       NULL, NULL},
   {NM_ITEM,       "New ",                    "W",  NULL,                       NULL, NULL},
   {NM_ITEM,       "Select Contents ",        "B",  NM_ITEMDISABLED,            NULL, NULL},
   {NM_ITEM,       "Sort Contents ",          "R",  NM_ITEMDISABLED,            NULL, NULL},
   {NM_ITEM,       "Delete... ",              "K",  NM_ITEMDISABLED,            NULL, NULL},
   {NM_ITEM,       NM_BARLABEL,               NULL, NULL,                       NULL, NULL},
   {NM_ITEM,       "Open At Start",           NULL, CHECKIT|MENUTOGGLE|CHECKED, NULL, NULL},

   {NM_TITLE,    "Gadget",                    NULL, NM_MENUDISABLED,            NULL, NULL},
   {NM_ITEM,       "Attributes... ",          "G",  NM_ITEMDISABLED,            NULL, NULL},
   {NM_ITEM,       "Font... ",                "F",  NM_ITEMDISABLED,            NULL, NULL},
   {NM_ITEM,       NM_BARLABEL,               NULL, NULL,                       NULL, NULL},
   {NM_ITEM,       "Copy ",                   "C",  NULL,                       NULL, NULL},
   {NM_ITEM,       "Delete... ",              "D",  NULL,                       NULL, NULL},

   {NM_TITLE,    "Settings",                  NULL, NULL,                       NULL, NULL},
   {NM_ITEM,       "Code Comments ",          NULL, CHECKIT|MENUTOGGLE,         NULL, NULL},
   {NM_ITEM,       "Top Border Adjust ",      NULL, CHECKIT|MENUTOGGLE,         NULL, NULL},
   {NM_ITEM,       "Use Pragmas ",            NULL, CHECKIT|MENUTOGGLE,         NULL, NULL},
   {NM_ITEM,       "Use SimpleRexx ",         NULL, CHECKIT|MENUTOGGLE,         NULL, NULL},
   {NM_ITEM,       "Use __chip Keyword ",     NULL, CHECKIT|MENUTOGGLE,         NULL, NULL},
   {NM_ITEM,       "Use UserData Structures ",NULL, CHECKIT|MENUTOGGLE,         NULL, NULL},
   {NM_ITEM,       "Use ASL File Requester ", NULL, CHECKIT|MENUTOGGLE,         NULL, NULL},
   {NM_ITEM,       "User Wait Signal ",       NULL, CHECKIT|MENUTOGGLE,         NULL, NULL},
   {NM_ITEM,       "Create Icons ",           NULL, CHECKIT|MENUTOGGLE,         NULL, NULL},
   {NM_ITEM,       NM_BARLABEL,               NULL, NULL,                       NULL, NULL},
   {NM_ITEM,       "Grid Snap",               NULL, NULL,                       NULL, NULL},
   {NM_SUB,          "Off ",                  "0",  CHECKIT,                    ~1,   NULL},
   {NM_SUB,          "2 Pixels ",             "2",  CHECKIT,                    ~2,   NULL},
   {NM_SUB,          "4 Pixels ",             "4",  CHECKIT,                    ~4,   NULL},
   {NM_SUB,          "8 Pixels ",             "8",  CHECKIT,                    ~8,   NULL},
   {NM_ITEM,       NM_BARLABEL,               NULL, NULL,                       NULL, NULL},
#ifdef DEMO
   {NM_ITEM,       "Save Settings",           NULL, NM_ITEMDISABLED,            NULL, NULL},
#else
   {NM_ITEM,       "Save Settings",           NULL, NULL,                       NULL, NULL},
#endif
   {NM_END,      NULL,                        NULL, NULL,                       NULL, NULL}
  };

struct AvailableIDCMP far AvailableIDCMP[] =
 {
  {"IDCMP_SIZEVERIFY",     IDCMP_SIZEVERIFY,     TYPE_USER},
  {"IDCMP_NEWSIZE",        IDCMP_NEWSIZE,        TYPE_USER},
  {"IDCMP_REFRESHWINDOW",  IDCMP_REFRESHWINDOW,  TYPE_VOID},
  {"IDCMP_MOUSEBUTTONS",   IDCMP_MOUSEBUTTONS,   TYPE_USER},
  {"IDCMP_MOUSEMOVE",      IDCMP_MOUSEMOVE,      TYPE_USER},
  {"IDCMP_GADGETDOWN",     IDCMP_GADGETDOWN,     TYPE_USER},
  {"IDCMP_GADGETUP",       IDCMP_GADGETUP,       TYPE_USER},
  {"IDCMP_REQSET",         IDCMP_REQSET,         TYPE_USER},
  {"IDCMP_MENUPICK",       IDCMP_MENUPICK,       TYPE_USER},
  {"IDCMP_CLOSEWINDOW",    IDCMP_CLOSEWINDOW,    TYPE_USER},
  {"IDCMP_RAWKEY",         IDCMP_RAWKEY,         TYPE_USER},
  {"IDCMP_REQVERIFY",      IDCMP_REQVERIFY,      TYPE_USER},
  {"IDCMP_REQCLEAR",       IDCMP_REQCLEAR,       TYPE_USER},
  {"IDCMP_MENUVERIFY",     IDCMP_MENUVERIFY,     TYPE_USER},
  {"IDCMP_NEWPREFS",       IDCMP_NEWPREFS,       TYPE_USER},
  {"IDCMP_DISKINSERTED",   IDCMP_DISKINSERTED,   TYPE_USER},
  {"IDCMP_DISKREMOVED",    IDCMP_DISKREMOVED,    TYPE_USER},
  {"IDCMP_ACTIVEWINDOW",   IDCMP_ACTIVEWINDOW,   TYPE_USER},
  {"IDCMP_INACTIVEWINDOW", IDCMP_INACTIVEWINDOW, TYPE_USER},
  {"IDCMP_DELTAMOVE",      IDCMP_DELTAMOVE,      TYPE_USER},
  {"IDCMP_VANILLAKEY",     IDCMP_VANILLAKEY,     TYPE_USER},
  {"IDCMP_INTUITICKS",     IDCMP_INTUITICKS,     TYPE_USER},
  {"IDCMP_IDCMPUPDATE",    IDCMP_IDCMPUPDATE,    TYPE_USER},
  {"IDCMP_MENUHELP",       IDCMP_MENUHELP,       TYPE_USER},
  {"IDCMP_CHANGEWINDOW",   IDCMP_CHANGEWINDOW,   TYPE_USER},
  {NULL,                   NULL,                 NULL}
 };

struct AvailableTags far AvailableScreenTags[] =
 {
  {"SA_Left",         SA_Left,         TYPE_INTEGER,  (LONG) "0"},
  {"SA_Top",          SA_Top,          TYPE_INTEGER,  (LONG) "0"},
  {"SA_AutoScroll",   SA_AutoScroll,   TYPE_INTEGER,  (LONG) "TRUE"},
  {"SA_Behind",       SA_Behind,       TYPE_INTEGER,  (LONG) "TRUE"},
  {"SA_FullPalette",  SA_FullPalette,  TYPE_INTEGER,  (LONG) "TRUE"},
  {"SA_Pens",         SA_Pens,         TYPE_WORDLIST, (LONG) &defaultpens},
  {"SA_BlockPen",     SA_BlockPen,     TYPE_INTEGER,  (LONG) "1"},
  {"SA_DetailPen",    SA_DetailPen,    TYPE_INTEGER,  (LONG) "0"},
  {"SA_PubName",      SA_PubName,      TYPE_STRING,   (LONG) "default pubname"},
  {"SA_Quiet",        SA_Quiet,        TYPE_INTEGER,  (LONG) "TRUE"},
  {"SA_ShowTitle",    SA_ShowTitle,    TYPE_INTEGER,  (LONG) "FALSE"},
  {"SA_SysFont",      SA_SysFont,      TYPE_INTEGER,  (LONG) "1"},
  {"SA_Depth",        SA_Depth,        TYPE_VOID,     NULL}, /* Screen Edit */
  {"SA_Width",        SA_Width,        TYPE_VOID,     NULL}, /* Screen Edit */
  {"SA_Height",       SA_Height,       TYPE_VOID,     NULL}, /* Screen Edit */
  {"SA_Colors",       SA_Colors,       TYPE_VOID,     NULL}, /* Color Edit */
  {"SA_DisplayID",    SA_DisplayID,    TYPE_VOID,     NULL}, /* Screen Edit */
  {"SA_Overscan",     SA_Overscan,     TYPE_VOID,     NULL}, /* Screen Edit */
  {"SA_DClip",        SA_DClip,        TYPE_VOID,     NULL}, /* Screen Edit */
  {"SA_Title",        SA_Title,        TYPE_VOID,     NULL}, /* Tag Edit */
  {"SA_Font",         SA_Font,         TYPE_VOID,     NULL}, /* Font Edit */
  {"SA_Type",         SA_Type,         TYPE_VOID,     NULL}, /* Unneccesary */
  {"SA_PubTask",      SA_PubTask,      TYPE_VOID,     NULL}, /* Unimplemented */
  {"SA_PubSig",       SA_PubSig,       TYPE_VOID,     NULL}, /* Unimplemented */
  {"SA_ErrorCode",    SA_ErrorCode,    TYPE_VOID,     NULL}, /* Unimplemented */
  {"SA_BitMap",       SA_BitMap,       TYPE_VOID,     NULL}, /* Unimplemented */
  {"TAG_DONE",        TAG_DONE,        TYPE_VOID,     NULL}
 };

struct AvailableTags far AvailableWindowTags[] =
 {
  {"WA_Left",               WA_Left,              TYPE_INTEGER,  (LONG) "50"},
  {"WA_Top",                WA_Top,               TYPE_INTEGER,  (LONG) "30"},
  {"WA_Width",              WA_Width,             TYPE_INTEGER,  (LONG) "300"},
  {"WA_Height",             WA_Height,            TYPE_INTEGER,  (LONG) "150"},
  {"WA_InnerWidth",         WA_InnerWidth,        TYPE_INTEGER,  (LONG) "300"},
  {"WA_InnerHeight",        WA_InnerHeight,       TYPE_INTEGER,  (LONG) "150"},
  {"WA_MinWidth",           WA_MinWidth,          TYPE_INTEGER,  (LONG) "40"},
  {"WA_MinHeight",          WA_MinHeight,         TYPE_INTEGER,  (LONG) "30"},
  {"WA_MaxWidth",           WA_MaxWidth,          TYPE_INTEGER,  (LONG) "-1"},
  {"WA_MaxHeight",          WA_MaxHeight,         TYPE_INTEGER,  (LONG) "-1"},
  {"WA_AutoAdjust",         WA_AutoAdjust,        TYPE_INTEGER,  (LONG) "FALSE"},
  {"WA_Zoom",               WA_Zoom,              TYPE_WORDLIST, (LONG) &defaultzoom},
  {"WA_DragBar",            WA_DragBar,           TYPE_INTEGER,  (LONG) "TRUE"},
  {"WA_CloseGadget",        WA_CloseGadget,       TYPE_INTEGER,  (LONG) "TRUE"},
  {"WA_DepthGadget",        WA_DepthGadget,       TYPE_INTEGER,  (LONG) "TRUE"},
  {"WA_SizeGadget",         WA_SizeGadget,        TYPE_INTEGER,  (LONG) "TRUE"},
  {"WA_SizeBBottom",        WA_SizeBBottom,       TYPE_INTEGER,  (LONG) "TRUE"},
  {"WA_SizeBRight",         WA_SizeBRight,        TYPE_INTEGER,  (LONG) "TRUE"},
  {"WA_Activate",           WA_Activate,          TYPE_INTEGER,  (LONG) "TRUE"},
  {"WA_Backdrop",           WA_Backdrop,          TYPE_INTEGER,  (LONG) "TRUE"},
  {"WA_Borderless",         WA_Borderless,        TYPE_INTEGER,  (LONG) "TRUE"},
  {"WA_GimmeZeroZero",      WA_GimmeZeroZero,     TYPE_IGNORE,   (LONG) "TRUE"},
  {"WA_MouseQueue",         WA_MouseQueue,        TYPE_INTEGER,  (LONG) "10"},
  {"WA_ReportMouse",        WA_ReportMouse,       TYPE_INTEGER,  (LONG) "TRUE"},
  {"WA_RptQueue",           WA_RptQueue,          TYPE_INTEGER,  (LONG) "10"},
  {"WA_BlockPen",           WA_BlockPen,          TYPE_INTEGER,  (LONG) "0"},
  {"WA_DetailPen",          WA_DetailPen,         TYPE_INTEGER,  (LONG) "1"},
  {"WA_ScreenTitle",        WA_ScreenTitle,       TYPE_STRING,   (LONG) "Screen title"},
  {"WA_PubScreenFallBack",  WA_PubScreenFallBack, TYPE_INTEGER,  (LONG) "TRUE"},
  {"WA_NoCareRefresh",      WA_NoCareRefresh,     TYPE_IGNORE,   (LONG) "TRUE"},
  {"WA_SimpleRefresh",      WA_SimpleRefresh,     TYPE_IGNORE,   (LONG) "TRUE"},
  {"WA_SmartRefresh",       WA_SmartRefresh,      TYPE_IGNORE,   (LONG) "TRUE"},
  {"WA_RMBTrap",            WA_RMBTrap,           TYPE_IGNORE,   (LONG) "TRUE"},
  {"WA_Title",              WA_Title,             TYPE_VOID,     NULL}, /* Tag Edit */
  {"WA_IDCMP",              WA_IDCMP,             TYPE_VOID,     NULL}, /* IDCMP Edit */
  {"WA_Gadgets",            WA_Gadgets,           TYPE_VOID,     NULL}, /* Automatic */
  {"WA_WBenchWindow",       WA_WBenchWindow,      TYPE_VOID,     NULL}, /* Automatic */
  {"WA_CustomScreen",       WA_CustomScreen,      TYPE_VOID,     NULL}, /* Automatic */
  {"WA_PubScreen",          WA_PubScreen,         TYPE_VOID,     NULL}, /* Automatic */
  {"WA_Flags",              WA_Flags,             TYPE_VOID,     NULL}, /* Automatic */
  {"WA_Checkmark",          WA_Checkmark,         TYPE_VOID,     NULL}, /* Not implemented */
  {"WA_WindowName",         WA_WindowName,        TYPE_VOID,     NULL}, /* Not implemented */
  {"WA_Colors",             WA_Colors,            TYPE_VOID,     NULL}, /* Not implemented */
  {"WA_PubScreenName",      WA_PubScreenName,     TYPE_VOID,     NULL}, /* Not implemented */
  {"WA_SuperBitMap",        WA_SuperBitMap,       TYPE_VOID,     NULL}, /* Not implemented */
  {"WA_BackFill",           WA_BackFill,          TYPE_VOID,     NULL}, /* Not implemented */
  {"TAG_DONE",              TAG_DONE,             TYPE_VOID,     NULL}
 };

struct AvailableTags far AvailableBUGadgetTags[] =
 {
  {"GT_Underscore",        GT_Underscore,        TYPE_CHARACTER,  (LONG) '_'},
  {"GA_Disabled",          GA_Disabled,          TYPE_INTEGER,    (LONG) "TRUE"},
  {"TAG_DONE",             TAG_DONE,             TYPE_VOID,       NULL}
 };

struct AvailableTags far AvailableCBGadgetTags[] =
 {
  {"GTCB_Checked",         GTCB_Checked,         TYPE_INTEGER,    (LONG) "TRUE"},
  {"GT_Underscore",        GT_Underscore,        TYPE_CHARACTER,  (LONG) '_'},
  {"GA_Disabled",          GA_Disabled,          TYPE_INTEGER,    (LONG) "TRUE"},
  {"TAG_DONE",             TAG_DONE,             TYPE_VOID,       NULL}
 };

struct AvailableTags far AvailableLVGadgetTags[] =
 {
  {"GTLV_Labels",          GTLV_Labels,          TYPE_LINKEDLIST, (LONG) &defaultlabels},
  {"GTLV_ReadOnly",        GTLV_ReadOnly,        TYPE_INTEGER,    (LONG) "TRUE"},
  {"GTLV_ScrollWidth",     GTLV_ScrollWidth,     TYPE_INTEGER,    (LONG) "10"},
  {"GTLV_Selected",        GTLV_Selected,        TYPE_INTEGER,    (LONG) "0"},
  {"GTLV_ShowSelected",    GTLV_ShowSelected,    TYPE_INTEGER,    (LONG) "0"},
  {"GTLV_Top",             GTLV_Top,             TYPE_INTEGER,    (LONG) "0"},
  {"GT_Underscore",        GT_Underscore,        TYPE_CHARACTER,  (LONG) '_'},
  {"GA_Disabled",          GA_Disabled,          TYPE_INTEGER,    (LONG) "TRUE"},
  {"LAYOUTA_Spacing",      LAYOUTA_Spacing,      TYPE_INTEGER,    (LONG) "1"},
  {"TAG_DONE",             TAG_DONE,             TYPE_VOID,       NULL}
 };

struct AvailableTags far AvailableMXGadgetTags[] =
 {
  {"GTMX_Active",          GTMX_Active,          TYPE_INTEGER,    (LONG) "0"},
  {"GTMX_Labels",          GTMX_Labels,          TYPE_STRINGLIST, (LONG) &defaultlabels},
  {"GTMX_Spacing",         GTMX_Spacing,         TYPE_INTEGER,    (LONG) "1"},
  {"GT_Underscore",        GT_Underscore,        TYPE_CHARACTER,  (LONG) '_'},
  {"TAG_DONE",             TAG_DONE,             TYPE_VOID,       NULL}
 };

struct AvailableTags far AvailableTXGadgetTags[] =
 {
  {"GTTX_Border",          GTTX_Border,          TYPE_INTEGER,    (LONG) "TRUE"},
  {"GTTX_CopyText",        GTTX_CopyText,        TYPE_INTEGER,    (LONG) "TRUE"},
  {"GTTX_Text",            GTTX_Text,            TYPE_STRING,     (LONG) "text"},
  {"GT_Underscore",        GT_Underscore,        TYPE_CHARACTER,  (LONG) '_'},
  {"TAG_DONE",             TAG_DONE,             TYPE_VOID,       NULL}
 };

struct AvailableTags far AvailableNMGadgetTags[] =
 {
  {"GTNM_Border",          GTNM_Border,          TYPE_INTEGER,    (LONG) "TRUE"},
  {"GTNM_Number",          GTNM_Number,          TYPE_INTEGER,    (LONG) "0"},
  {"GT_Underscore",        GT_Underscore,        TYPE_CHARACTER,  (LONG) '_'},
  {"TAG_DONE",             TAG_DONE,             TYPE_VOID,       NULL}
 };

struct AvailableTags far AvailableCYGadgetTags[] =
 {
  {"GTCY_Active",          GTCY_Active,          TYPE_INTEGER,    (LONG) "1"},
  {"GTCY_Labels",          GTCY_Labels,          TYPE_STRINGLIST, (LONG) &defaultlabels},
  {"GT_Underscore",        GT_Underscore,        TYPE_CHARACTER,  (LONG) '_'},
  {"GA_Disabled",          GA_Disabled,          TYPE_INTEGER,    (LONG) "TRUE"},
  {"TAG_DONE",             TAG_DONE,             TYPE_VOID,       NULL}
 };

struct AvailableTags far AvailablePAGadgetTags[] =
 {
  {"GTPA_Color",           GTPA_Color,           TYPE_INTEGER,    (LONG) "1"},
  {"GTPA_ColorOffset",     GTPA_ColorOffset,     TYPE_INTEGER,    (LONG) "0"},
  {"GTPA_Depth",           GTPA_Depth,           TYPE_INTEGER,    (LONG) "1"},
  {"GTPA_IndicatorWidth",  GTPA_IndicatorWidth,  TYPE_INTEGER,    (LONG) "15"},
  {"GTPA_IndicatorHeight", GTPA_IndicatorHeight, TYPE_INTEGER,    (LONG) "15"},
  {"GT_Underscore",        GT_Underscore,        TYPE_CHARACTER,  (LONG) '_'},
  {"GA_Disabled",          GA_Disabled,          TYPE_INTEGER,    (LONG) "TRUE"},
  {"TAG_DONE",             TAG_DONE,             TYPE_VOID,       NULL}
 };

struct AvailableTags far AvailableSCGadgetTags[] =
 {
  {"GTSC_Arrows",          GTSC_Arrows,          TYPE_INTEGER,    (LONG) "15"},
  {"GTSC_Top",             GTSC_Top,             TYPE_INTEGER,    (LONG) "0"},
  {"GTSC_Total",           GTSC_Total,           TYPE_INTEGER,    (LONG) "10"},
  {"GTSC_Visible",         GTSC_Visible,         TYPE_INTEGER,    (LONG) "5"},
  {"GT_Underscore",        GT_Underscore,        TYPE_CHARACTER,  (LONG) '_'},
  {"GA_Disabled",          GA_Disabled,          TYPE_INTEGER,    (LONG) "TRUE"},
  {"GA_Immediate",         GA_Immediate,         TYPE_INTEGER,    (LONG) "TRUE"},
  {"GA_RelVerify",         GA_RelVerify,         TYPE_INTEGER,    (LONG) "TRUE"},
  {"PGA_Freedom",          PGA_Freedom,          TYPE_INTEGER,    (LONG) "LORIENT_VERT"},
  {"TAG_DONE",             TAG_DONE,             TYPE_VOID,       NULL}
 };

struct AvailableTags far AvailableSLGadgetTags[] =
 {
  {"GTSL_Level",           GTSL_Level,           TYPE_INTEGER,    (LONG) "5"},
  {"GTSL_LevelFormat",     GTSL_LevelFormat,     TYPE_STRING,     (LONG) "%2ld"},
  {"GTSL_LevelPlace",      GTSL_LevelPlace,      TYPE_INTEGER,    (LONG) "PLACETEXT_RIGHT"},
  {"GTSL_Max",             GTSL_Max,             TYPE_INTEGER,    (LONG) "15"},
  {"GTSL_MaxLevelLen",     GTSL_MaxLevelLen,     TYPE_INTEGER,    (LONG) "4"},
  {"GTSL_Min",             GTSL_Min,             TYPE_INTEGER,    (LONG) "0"},
  {"GT_Underscore",        GT_Underscore,        TYPE_CHARACTER,  (LONG) '_'},
  {"GA_Disabled",          GA_Disabled,          TYPE_INTEGER,    (LONG) "TRUE"},
  {"GA_Immediate",         GA_Immediate,         TYPE_INTEGER,    (LONG) "TRUE"},
  {"GA_RelVerify",         GA_RelVerify,         TYPE_INTEGER,    (LONG) "TRUE"},
  {"PGA_Freedom",          PGA_Freedom,          TYPE_INTEGER,    (LONG) "LORIENT_VERT"},
  {"TAG_DONE",             TAG_DONE,             TYPE_VOID,       NULL}
 };

struct AvailableTags far AvailableSTGadgetTags[] =
 {
  {"GTST_MaxChars",        GTST_MaxChars,        TYPE_INTEGER,    (LONG) "25"},
  {"GTST_String",          GTST_String,          TYPE_STRING,     (LONG) "string"},
  {"GT_Underscore",        GT_Underscore,        TYPE_CHARACTER,  (LONG) '_'},
  {"GA_Disabled",          GA_Disabled,          TYPE_INTEGER,    (LONG) "TRUE"},
  {"GA_TabCycle",          GA_TabCycle,          TYPE_INTEGER,    (LONG) "FALSE"},
  {"STRINGA_ExitHelp",     STRINGA_ExitHelp,     TYPE_INTEGER,    (LONG) "TRUE"},
  {"STRINGA_Justification",STRINGA_Justification,TYPE_INTEGER,    (LONG) "GACT_STRINGCENTER"},
  {"TAG_DONE",             TAG_DONE,             TYPE_VOID,       NULL}
 };

struct AvailableTags far AvailableINGadgetTags[] =
 {
  {"GTIN_MaxChars",        GTIN_MaxChars,        TYPE_INTEGER,    (LONG) "10"},
  {"GTIN_Number",          GTIN_Number,          TYPE_INTEGER,    (LONG) "0"},
  {"GT_Underscore",        GT_Underscore,        TYPE_CHARACTER,  (LONG) '_'},
  {"GA_Disabled",          GA_Disabled,          TYPE_INTEGER,    (LONG) "TRUE"},
  {"GA_TabCycle",          GA_TabCycle,          TYPE_INTEGER,    (LONG) "FALSE"},
  {"STRINGA_ExitHelp",     STRINGA_ExitHelp,     TYPE_INTEGER,    (LONG) "TRUE"},
  {"STRINGA_Justification",STRINGA_Justification,TYPE_INTEGER,    (LONG) "GACT_STRINGRIGHT"},
  {"TAG_DONE",             TAG_DONE,             TYPE_VOID,       NULL}
 };

/* Unused tags */

/*
  {"GTSL_DispFunc",        GTSL_DispFunc,        TYPE_FUNCTION,   NULL},

  {"GA_Top",               GA_Top,               TYPE_VOID,       NULL},
  {"GA_Left",              GA_Left,              TYPE_VOID,       NULL},
  {"GA_RelRight",          GA_RelRight,          TYPE_INTEGER,    (LONG) "1"},
  {"GA_RelBottom",         GA_RelBottom,         TYPE_INTEGER,    (LONG) "1"},
  {"GA_RelWidth",          GA_RelWidth,          TYPE_INTEGER,    (LONG) "1"},
  {"GA_Width",             GA_Width,             TYPE_VOID,       NULL},
  {"GA_Height",            GA_Height,            TYPE_VOID,       NULL},
  {"GA_RelHeight",         GA_RelHeight,         TYPE_INTEGER,    (LONG) "1"},
  {"GA_Text",              GA_Text,              TYPE_STRING,     (LONG) "text"},
  {"GA_Image",             GA_Image,             TYPE_VOID,       NULL},
  {"GA_Border",            GA_Border,            TYPE_INTEGER,    (LONG) "1"},
  {"GA_SelectRender",      GA_SelectRender,      TYPE_VOID,       NULL},
  {"GA_Highlight",         GA_Highlight,         TYPE_VOID,       NULL},
  {"GA_Disabled",          GA_Disabled,          TYPE_INTEGER,    (LONG) "1"},
  {"GA_GZZGadget",         GA_GZZGadget,         TYPE_INTEGER,    (LONG) "1"},
  {"GA_ID",                GA_ID,                TYPE_VOID,       NULL},
  {"GA_UserData",          GA_UserData,          TYPE_INTEGER,    (LONG) "0"},
  {"GA_SpecialInfo",       GA_SpecialInfo,       TYPE_VOID,       NULL},
  {"GA_Selected",          GA_Selected,          TYPE_INTEGER,    (LONG) "1"},
  {"GA_EndGadget",         GA_EndGadget,         TYPE_INTEGER,    (LONG) "1"},
  {"GA_Immediate",         GA_Immediate,         TYPE_INTEGER,    (LONG) "1"},
  {"GA_RelVerify",         GA_RelVerify,         TYPE_INTEGER,    (LONG) "1"},
  {"GA_FollowMouse",       GA_FollowMouse,       TYPE_INTEGER,    (LONG) "1"},
  {"GA_RightBorder",       GA_RightBorder,       TYPE_INTEGER,    (LONG) "1"},
  {"GA_LeftBorder",        GA_LeftBorder,        TYPE_INTEGER,    (LONG) "1"},
  {"GA_TopBorder",         GA_TopBorder,         TYPE_INTEGER,    (LONG) "1"},
  {"GA_BottomBorder",      GA_BottomBorder,      TYPE_INTEGER,    (LONG) "1"},
  {"GA_ToggleSelect",      GA_ToggleSelect,      TYPE_INTEGER,    (LONG) "1"},
  {"GA_DrawInfo",          GA_DrawInfo,          TYPE_VOID,       NULL},
  {"GA_IntuiText",         GA_IntuiText,         TYPE_VOID,       NULL},
  {"GA_LabelImage",        GA_LabelImage,        TYPE_VOID,       NULL},
  {"GA_TabCycle",          GA_TabCycle,          TYPE_INTEGER,    (LONG) "0"},

  {"PGA_Freedom",          PGA_Freedom,          TYPE_FREEDOM,    (LONG) "LORIENT_VERT"},
  {"PGA_Borderless",       PGA_Borderless,       TYPE_INTEGER,    (LONG) "1"},
  {"PGA_HorizPot",         PGA_HorizPot,         TYPE_INTEGER,    (LONG) "1"},
  {"PGA_HorizBody",        PGA_HorizBody,        TYPE_INTEGER,    (LONG) "1"},
  {"PGA_VertPot",          PGA_VertPot,          TYPE_INTEGER,    (LONG) "1"},
  {"PGA_VertBody",         PGA_VertBody,         TYPE_INTEGER,    (LONG) "1"},
  {"PGA_Total",            PGA_Total,            TYPE_INTEGER,    (LONG) "1"},
  {"PGA_Visible",          PGA_Visible,          TYPE_INTEGER,    (LONG) "1"},
  {"PGA_Top",              PGA_Top,              TYPE_INTEGER,    (LONG) "1"},
  {"PGA_NewLook",          PGA_NewLook,          TYPE_INTEGER,    (LONG) "1"},

  {"STRINGA_MaxChars",       STRINGA_MaxChars,       TYPE_INTEGER,    (LONG) "1"},
  {"STRINGA_Buffer",         STRINGA_Buffer,         TYPE_VOID,       NULL},
  {"STRINGA_UndoBuffer",     STRINGA_UndoBuffer,     TYPE_VOID,       NULL},
  {"STRINGA_WorkBuffer",     STRINGA_WorkBuffer,     TYPE_VOID,       NULL},
  {"STRINGA_BufferPos",      STRINGA_BufferPos,      TYPE_VOID,       NULL},
  {"STRINGA_DispPos",        STRINGA_DispPos,        TYPE_VOID,       NULL},
  {"STRINGA_AltKeyMap",      STRINGA_AltKeyMap,      TYPE_VOID,       NULL},
  {"STRINGA_Font",           STRINGA_Font,           TYPE_VOID,       NULL},
  {"STRINGA_Pens",           STRINGA_Pens,           TYPE_VOID,       NULL},
  {"STRINGA_ActivePens",     STRINGA_ActivePens,     TYPE_VOID,       NULL},
  {"STRINGA_EditHook",       STRINGA_EditHook,       TYPE_VOID,       NULL},
  {"STRINGA_EditModes",      STRINGA_EditModes,      TYPE_VOID,       NULL},
  {"STRINGA_ReplaceMode",    STRINGA_ReplaceMode,    TYPE_VOID,       NULL},
  {"STRINGA_FixedFieldMode", STRINGA_FixedFieldMode, TYPE_VOID,       NULL},
  {"STRINGA_NoFilterMode",   STRINGA_NoFilterMode,   TYPE_VOID,       NULL},
  {"STRINGA_LongVal",        STRINGA_LongVal,        TYPE_VOID,       NULL},
  {"STRINGA_TextVal",        STRINGA_TextVal,        TYPE_VOID,       NULL},
  {"STRINGA_ExitHelp",       STRINGA_ExitHelp,       TYPE_INTEGER,    (LONG) "1"},

  {"LAYOUTA_LayoutObj",    LAYOUTA_LayoutObj,    TYPE_VOID,       NULL},
  {"LAYOUTA_Spacing",      LAYOUTA_Spacing,      TYPE_INTEGER,    (LONG) "1"},
  {"LAYOUTA_Orientation",  LAYOUTA_Orientation,  TYPE_FREEDOM,    (LONG) "LORIENT_VERT"},
*/

