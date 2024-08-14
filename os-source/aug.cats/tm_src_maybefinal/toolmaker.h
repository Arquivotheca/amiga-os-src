
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <intuition/gadgetclass.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <graphics/displayinfo.h>
#include <libraries/gadtools.h>
#include <libraries/iffparse.h>
#include <dos/dos.h>
#include <stdio.h>

#include <pragmas/asl_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/wb_pragmas.h>
#include <pragmas/diskfont_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/iffparse_pragmas.h>

#define COLOR2RED(x)   (((x) & 0x0F00) >>8)
#define COLOR2GREEN(x) (((x) & 0x00F0) >>4)
#define COLOR2BLUE(x)  (((x) & 0x000F))

#define RGB2COLOR(r,g,b) ((r)<<8 | (g)<<4 | (b))

#define MIN(a,b) ((a)<=(b)?(a):(b))

#ifdef TEST
#define DEBUGTEXT(text,val);      DebugText(text,(ULONG)val);
#define DEBUGENTER(text,arg);     DebugEnter(text,(ULONG)arg);
#define DEBUGEXIT(rflag,retval);  DebugExit((ULONG)rflag,(ULONG)retval);
#define DEBUGALLOC(memsize);      DebugAlloc((ULONG)memsize);
#define DEBUGFREE(memsize);       DebugFree((ULONG)memsize);
#else
#define DEBUGTEXT(string,number); ;
#define DEBUGENTER(funcname,arg); ;
#define DEBUGEXIT(rflag,retval);  ;
#define DEBUGALLOC(memsize);      ;
#define DEBUGFREE(memsize);       ;
#endif

#define NUMCONSTANTS      13
#define NUMPROJECTITEMS   18
#define NUMEDITITEMS       8
#define NUMMACROITEMS      5
#define NUMSETTINGITEMS    6
#define NUMUSERITEMS       4

#define MAXWINDOWS        10
#define MAXTAGS           50
#define MAXMENUS          31 /* 10 */
#define MAXITEMS          63 /* 25 */
#define MAXSUBS           31 /* 15 */

#define MINGADGETWIDTH    16
#define MINGADGETHEIGHT    8
#define CHECKBOXWIDTH     26
#define CHECKBOXHEIGHT    11
#define MXWIDTH           17
#define MXHEIGHT           9
#define MINLVWIDTH        40
#define MINLVHEIGHT       21
#define MININPUTHEIGHT    12

#define SIZEBLOCKWIDTH     5
#define SIZEBLOCKHEIGHT    5

#define ACTUALLYMOVESIZE   5
#define GRIDSIZE           4
#define STRINGSIZE        84
#define LABELSIZE          9
#define PATHSIZE        1024
#define FILESIZE         256

#define TYPE_VOID          0
#define TYPE_WINDOW        1
#define TYPE_GADGET        2
#define TYPE_INTEGER       3
#define TYPE_STRING        4
#define TYPE_STRINGLIST    5
#define TYPE_FUNCTION      6
#define TYPE_CHARACTER     7
#define TYPE_PLACE         8
#define TYPE_FREEDOM       9
#define TYPE_WORDLIST     10
#define TYPE_USER         11
#define TYPE_SCREEN       12
#define TYPE_LINKEDLIST   13
#define TYPE_IGNORE       14

#define ID_TMUI MAKE_ID('T','M','U','I')  /* FORM: Project file */
#define ID_TMSE MAKE_ID('T','M','S','E')  /* FORM: Settings file */

#define ID_THDR MAKE_ID('T','H','D','R')  /* Chunk: Header */
#define ID_ANNO MAKE_ID('A','N','N','O')  /* Chunk: Annotation */

#define ID_SATB MAKE_ID('S','A','T','B')  /* Chunk: Auto Top Border setting */
#define ID_SCOM MAKE_ID('S','C','O','M')  /* Chunk: Comments setting */
#define ID_SGSZ MAKE_ID('S','G','S','Z')  /* Chunk: Grid Size setting */
#define ID_SPRA MAKE_ID('S','P','R','A')  /* Chunk: Pragmas setting */
#define ID_SICN MAKE_ID('S','I','C','N')  /* Chunk: Create Icons setting */
#define ID_SUSG MAKE_ID('S','U','S','G')  /* Chunk: User Signal setting */
#define ID_SARX MAKE_ID('S','A','R','X')  /* Chunk: SimpleRexx setting */
#define ID_SCHP MAKE_ID('S','C','H','P')  /* Chunk: __chip Keyword setting */
#define ID_SUDA MAKE_ID('S','U','D','A')  /* Chunk: UserData setting */
#define ID_SFRQ MAKE_ID('S','F','R','Q')  /* Chunk: ASL File Requester setting */

#define ID_TSCR MAKE_ID('T','S','C','R')  /* FORM: Screen */
#define ID_TSDA MAKE_ID('T','S','D','A')  /* Chunk: Screen data */
#define ID_CMAP MAKE_ID('C','M','A','P')  /* Chunk: Color map */

#define ID_TFON MAKE_ID('T','F','O','N')  /* FORM: Font */
#define ID_TTAT MAKE_ID('T','T','A','T')  /* Chunk: Text Attr */

#define ID_TWIN MAKE_ID('T','W','I','N')  /* FORM: Window */
#define ID_TWDA MAKE_ID('T','W','D','A')  /* Chunk: Window data */

#define ID_TMLS MAKE_ID('T','M','L','S')  /* FORM: Menu list */
#define ID_TMEN MAKE_ID('T','M','E','N')  /* FORM: Menu */
#define ID_TMDA MAKE_ID('T','M','D','A')  /* Chunk: Menu data */
#define ID_TITE MAKE_ID('T','I','T','E')  /* FORM: Item */
#define ID_TIDA MAKE_ID('T','I','D','A')  /* Chunk: Item data */
#define ID_TSID MAKE_ID('T','S','I','D')  /* Chunk: Subitem data */

#define ID_TGAD MAKE_ID('T','G','A','D')  /* FORM: Gadtools Gadget */
#define ID_TGDA MAKE_ID('T','G','D','A')  /* Chunk: Gadtools Gadget data */

#define ID_TTAG MAKE_ID('T','T','A','G')  /* FORM: Toolmaker tags */
#define ID_TNIN MAKE_ID('T','N','I','N')  /* Chunk: Non-interactive tag */
#define ID_TCHA MAKE_ID('T','C','H','A')  /* Chunk: Character tag */
#define ID_TINT MAKE_ID('T','I','N','T')  /* Chunk: Integer tag */
#define ID_TWLS MAKE_ID('T','W','L','S')  /* Chunk: Word List tag */
#define ID_TSTR MAKE_ID('T','S','T','R')  /* Chunk: String tag */
#define ID_TSLS MAKE_ID('T','S','L','S')  /* Chunk: String List tag */
#define ID_TLLS MAKE_ID('T','L','L','S')  /* Chunk: Linked List tag */

#define TMFILEERR_NONE      0
#define TMFILEERR_OPEN      1
#define TMFILEERR_WRONGTYPE 2
#define TMFILEERR_OUTOFMEM  3
#define TMFILEERR_READ      4
#define TMFILEERR_WRITE     5

/*
#define SAVE_WINDOW                1
#define SAVE_GADGET                2
#define SAVE_TAGCHARACTER          3
#define SAVE_TAGINTEGER            4
#define SAVE_TAGSTRINGLIST         5
#define SAVE_TAGSTRING             6
#define SAVE_TAGIGNORE             7
#define SAVE_ENDTAGS_              8
#define SAVE_ENDWINDOW_            9
#define SAVE_OPTIONCOMMENTS       10
#define SAVE_OPTIONREALGADS_      11
#define SAVE_OPTIONGRIDSIZE       12
#define SAVE_MENUTITLE            13
#define SAVE_MENUITEM             14
#define SAVE_MENUSUB              15
#define SAVE_TAGWORDLIST          16
#define SAVE_TAGPLACE_            17
#define SAVE_TAGFREEDOM_          18
#define SAVE_BORDERTOP            19
#define SAVE_OPTIONAUTOTOPBORDER  20
#define SAVE_OPTIONREGPARMS       21
#define SAVE_OPTIONREALMENUS_     22
#define SAVE_SCREEN               23
#define SAVE_ENDSCREEN_           24
#define SAVE_OPTIONCREATEICONS    25
#define SAVE_OPTIONUSERSIGNAL     26
#define SAVE_OPTIONAREXX          27
#define SAVE_OPTIONCHIPKEYWORD    28
#define SAVE_OPTIONUSERDATA       29
#define SAVE_OPTIONASL            30
#define SAVE_TAGLINKEDLIST        31
*/

#define DONE_OK          1
#define DONE_CANCEL      2
#define DONE_ENDPROGRAM  3

#define DRAGNONE     0
#define DRAGSELECT   1
#define DRAGCREATE   2
#define DRAGMOVE     3
#define DRAGSIZEUL   4
#define DRAGSIZEUR   5
#define DRAGSIZELL   6
#define DRAGSIZELR   7
#define DRAGSIZEL    8
#define DRAGSIZER    9
#define DRAGSIZET   10
#define DRAGSIZEB   11

#define MODE_DEFAULT       0x00000000
#define MODE_NTSC          0x00000001
#define MODE_PAL           0x00000002
#define MODE_INTERLACE     0x00000004
#define MODE_HIRES         0x00000008
#define MODE_SUPERHIRES    0x00000010
#define MODE_PRODUCTIVITY  0x00000020
#define MODE_A202410HZ     0x00000040
#define MODE_A202415HZ     0x00000080
#define MODE_OSCANTEXT     0x00000100
#define MODE_OSCANSTANDARD 0x00000200
#define MODE_OSCANMAX      0x00000400
#define MODE_OSCANVIDEO    0x00000800
#define MODE_OSCANNONE     0x00001000
#define MODE_CUSTOMCOLORS  0x00002000
#define MODE_DEFAULTWIDTH  0x00004000
#define MODE_DEFAULTHEIGHT 0x00008000
#define MODE_WORKBENCH     0x80000000

#define STATUS_KILLED       0
#define STATUS_NORMAL       1
#define STATUS_REMOVED      2
#define COMMAND_KILL        3
#define COMMAND_REMOVE      4
#define COMMAND_CHANGE      5

#define ID_NEWGADGET        4
#define ID_TESTGADGETS      5
#define ID_TESTMENUS        6

#define ID_OK              10
#define ID_CANCEL          11
#define ID_KILL            12

#define ID_TEXT           101
#define ID_AVAILABLE      102
#define ID_SELECTED       103
#define ID_TAGUSE         104
#define ID_TAGREMOVE      105
#define ID_TAGLIST        106
#define ID_TAGSTRING      107
#define ID_TAGADD         108
#define ID_TAGDEL         109
#define ID_TAGUP          110
#define ID_TAGDOWN        111
#define ID_EDITMENUS      112
#define ID_EDITIDCMP      113
#define ID_TEXTPOS        114
#define ID_LABEL          115
#define ID_SCREENMODE     116
#define ID_NTSCPAL        117
#define ID_OVERSCAN       118
#define ID_INTERLACE      119
#define ID_SCREENWIDTH    120
#define ID_SCREENHEIGHT   121
#define ID_SCREENDWIDTH   122
#define ID_SCREENDHEIGHT  123
#define ID_CUSTOMCOLORS   124
#define ID_TAGTYPE        125
#define ID_TAGLABEL       126

#define ID_SIZEVERIFY     201
#define ID_NEWSIZE        202
#define ID_REFRESHWINDOW  203
#define ID_MOUSEBUTTONS   204
#define ID_MOUSEMOVE      205
#define ID_GADGETDOWN     206
#define ID_GADGETUP       207
#define ID_REQSET         208
#define ID_MENUPICK       209
#define ID_CLOSEWINDOW    210
#define ID_RAWKEY         211
#define ID_REQVERIFY      212
#define ID_REQCLEAR       213
#define ID_MENUVERIFY     214
#define ID_NEWPREFS       215
#define ID_DISKINSERTED   216
#define ID_DISKREMOVED    217
#define ID_WBENCHMESSAGE  218
#define ID_ACTIVEWINDOW   219
#define ID_INACTIVEWINDOW 220
#define ID_DELTAMOVE      221
#define ID_VANILLAKEY     222
#define ID_INTUITICKS     223
#define ID_IDCMPUPDATE    224
#define ID_MENUHELP       225
#define ID_CHANGEWINDOW   226

#define ID_MENULIST       301
#define ID_MENUSTRING     302
#define ID_MENULABEL      303
#define ID_MENUADD        304
#define ID_MENUDEL        305
#define ID_MENUUP         306
#define ID_MENUDOWN       307
#define ID_MENUDISABLED   308
#define ID_ITEMLIST       309
#define ID_ITEMSTRING     310
#define ID_ITEMLABEL      311
#define ID_ITEMADD        312
#define ID_ITEMDEL        313
#define ID_ITEMUP         314
#define ID_ITEMDOWN       315
#define ID_ITEMDISABLED   316
#define ID_SUBLIST        317
#define ID_SUBSTRING      318
#define ID_SUBLABEL       319
#define ID_SUBADD         320
#define ID_SUBDEL         321
#define ID_SUBUP          322
#define ID_SUBDOWN        323
#define ID_SUBDISABLED    324
#define ID_CHECKIT        325
#define ID_CHECKED        326
#define ID_TOGGLE         327
#define ID_EXCLUDE        328
#define ID_COMMKEY        329

#define ID_SCREENDEPTH    401
#define ID_SCREENPALETTE  402
#define ID_SCREENRED      403
#define ID_SCREENGREEN    404
#define ID_SCREENBLUE     405

#define ID_TOPEDGE        501
#define ID_LEFTEDGE       502
#define ID_WIDTH          503
#define ID_HEIGHT         504
#define ID_HIGHLIGHT      505

#define MENU_PROJECT      0
#define ITEM_NEW            0
#define ITEM_OPEN           1
/*      ITEM_BAR            2  */
#define ITEM_SAVE           3
#define ITEM_SAVEAS         4
/*      ITEM_BAR            5  */
#define ITEM_GENERATE       6
/*      ITEM_BAR            7  */
#define ITEM_ABOUT          8
/*      ITEM_BAR            9  */
#define ITEM_QUIT          10

#define MENU_SCREEN       1
#define ITEM_SCREENMODE     0
#define ITEM_SCREENTAGS     1
#define ITEM_SCREENCOLORS   2
#define ITEM_SCREENFONT     3

#define MENU_WINDOW       2
#define ITEM_WINDOWTAGS     0
#define ITEM_WINDOWIDCMP    1
#define ITEM_WINDOWMENUS    2
/*      ITEM_BAR            3  */
#define ITEM_NEWWINDOW      4
#define ITEM_SELECTALL      5
#define ITEM_SORTCONTENTS   6
#define ITEM_KILLWINDOW     7
/*      ITEM_BAR            8  */
#define ITEM_OPENATSTART    9

#define MENU_GADGET       3
#define ITEM_GADGETTAGS     0
#define ITEM_GADGETFONT     1
/*      ITEM_BAR            2  */
#define ITEM_COPYGADGET     3
#define ITEM_KILLGADGET     4

#define MENU_OPTIONS      4
#define ITEM_COMMENTS       0
#define ITEM_TOPBORDER      1
#define ITEM_REGPARMS       2
#define ITEM_AREXX          3
#define ITEM_CHIPKEYWORD    4
#define ITEM_USERDATA       5
#define ITEM_ASL            6
#define ITEM_USERSIGNAL     7
#define ITEM_CREATEICONS    8
/*      ITEM_BAR            9  */
#define ITEM_GRID          10
#define SUB_OFF               0
#define SUB_2                 1
#define SUB_4                 2
#define SUB_8                 3
/*      ITEM_BAR           11  */
#define ITEM_SAVESETTINGS  12

#define WINDOWFLAG_GZZ          0x0001
#define WINDOWFLAG_DISABLED     0x0002
#define WINDOWFLAG_OPENATSTART  0x0004

#define SCREENFLAG_OPENATSTART  0x0004

#define MENUFLAG_AUTOEXCLUDE    0x0001

struct MainWindow
  {
  struct Window *Window;
  struct Menu *Menu;
  struct Gadget *GadgetList;
  struct Gadget *ContextGadget;
  struct Gadget *TypeGadget;
  struct Gadget *TestGadgetGadget;
  struct Gadget *TestMenuGadget;
  struct Requester Requester;
  SHORT  LeftEdge;
  SHORT  TopEdge;
  UBYTE  Status;
  };

struct ScreenNode
  {
  struct Node Node;
  struct Screen *Screen;
  UBYTE  Workbench;
  ULONG  TMSFlags;
  ULONG  Mode;
  ULONG  Width;
  ULONG  Height;
  ULONG  Depth;
  ULONG  DisplayID;
  ULONG  Overscan;
  ULONG  Color[32];
  SHORT  TagCount;
  struct List TagList;
  struct Rectangle DClip;
  struct TextAttr TextAttr;
  char   Title[STRINGSIZE];
  char   SourceLabel[STRINGSIZE];
  char   FontName[STRINGSIZE];
  };

struct WindowNode
  {
  struct Node Node;
  struct Window *Window;
  struct List TagList;
  struct List NewGadgetList;
  struct List GadgetList;
  struct List MenuList;
  struct Gadget *FirstGadget;
  struct Gadget *ContextGadget;
  struct Menu *Menu;
  struct Requester Requester;
  ULONG  IDCMP;
  ULONG  TMWFlags;
  SHORT  TagCount;
  char   Title[STRINGSIZE];
  char   SourceLabel[STRINGSIZE];
  };

struct NewGadgetNode
  {
  struct Node Node;
  struct NewGadget NewGadget;
  struct TextAttr TextAttr;
  struct List TagList;
  ULONG  Kind;
  ULONG  TMGFlags;
  SHORT  Selected;
  SHORT  TagCount;
  SHORT  xstart, ystart;
  SHORT  xend, yend;
  char   GadgetText[STRINGSIZE];
  char   SourceLabel[STRINGSIZE];
  char   FontName[STRINGSIZE];
  };

struct GadgetNode
  {
  struct Node Node;
  struct Gadget *Gadget;
  };

struct MenuNode
  {
  struct Node Node;
  struct List ItemList;
  UWORD  Flags;
  ULONG  TMMFlags;
  SHORT  ItemCount;
  char   MenuText[STRINGSIZE];
  char   SourceLabel[STRINGSIZE];
  };

struct ItemNode
  {
  struct Node Node;
  struct List SubList;
  UWORD  Flags;
  UBYTE  TMMFlags;
  SHORT  SubCount;
  char   ItemText[STRINGSIZE];
  char   CommKey[2];
  char   SourceLabel[STRINGSIZE];
  };

struct SubNode
  {
  struct Node Node;
  UWORD  Flags;
  ULONG  TMMFlags;
  char   SubText[STRINGSIZE];
  char   CommKey[2];
  char   SourceLabel[STRINGSIZE];
  };

struct TagNode
  {
  struct Node Node;
  struct TagItem TagItem;
  SHORT  DataCount;
  APTR   Data;
  struct List SourceLabelList;
  };

struct EditTagNode
  {
  struct Node Node;
  struct TagItem TagItem;
  struct List EditStringList;
  SHORT  StringCount;
  };

struct EditStringNode
  {
  struct Node Node;
  char   String[STRINGSIZE];
  char   SourceLabel[STRINGSIZE];
  };

struct StringNode
  {
  struct Node Node;
  char   String[STRINGSIZE];
  };

struct AvailableTags
  {
  char  *Name;
  ULONG Tag;
  ULONG Type;
  LONG  Data;
  };

struct AvailableIDCMP
  {
  char  *Name;
  ULONG IDCMP;
  ULONG Type;
  };

struct StringData
  {
  SHORT StringCount;
  char  **StringArray;
  };

struct TextAttrNode
  {
  struct Node Node;
  struct TextAttr TextAttr;
  char   Name[STRINGSIZE];
  char   FontName[STRINGSIZE];
  };

struct FontNode
  {
  struct Node Node;
  struct TextFont *TextFont;
  char   FontName[STRINGSIZE];
  };

struct EditItem
  {
  char ItemText[STRINGSIZE];
  char CommKey[2];
  char SourceLabel[STRINGSIZE];
  ULONG Flags;
  };

/****************************************************************************/

/* Main.c */

void  main(int, char **);
void  CleanExit(char *);
int   HandleArguments(int, char **);
void  EventLoop(void);
void  NewProject(void);
void  DrawDragBox(struct WindowNode *, SHORT, SHORT, SHORT, SHORT);
void  SpinDragBox(struct WindowNode *);
void  SelectSelect(void);
void  MarkGadget(struct WindowNode *, struct NewGadgetNode *);
void  SelectGadget(struct WindowNode *, struct NewGadgetNode *);
void  UnSelectGadget(struct WindowNode *, struct NewGadgetNode *);
void  SelectAllGadgets(struct WindowNode *);
void  UnSelectAllGadgets(struct WindowNode *);
void  DragSelectGadgets(struct WindowNode *, SHORT, SHORT, SHORT, SHORT);
void  RedoWindowSignals(void);
int   SetUpMenus(void);
int   PutError(char *, char *);
void  MoveNodeUp(struct List *, int, struct Window *, struct Gadget *);
void  MoveNodeDown(struct List *, int, struct Window *, struct Gadget *);
int   RequestKill(void);
void  MainWindowSleep(void);
void  MainWindowWakeUp(void);
void  GetViewSize(void);
void  SortGadgets(struct WindowNode *);
void  SwapNodes(struct List *, struct Node *, struct Node *);
void  TestGadgets(void);
void  TestMenus(void);
int   FileRequest(char *, char *, char *, char);
void  DebugEnter(char *, ULONG);
void  DebugExit(ULONG, ULONG);
void  DebugAlloc(ULONG);
void  DebugFree(ULONG);
void  DebugText(char *, ULONG);

/* Convert.c */

SHORT ConvertGridX(SHORT);
SHORT ConvertGridY(SHORT);
char  *IDCMP2String(ULONG);
char  *MenuFlags2String(ULONG);
char  *ItemFlags2String(ULONG);
char  *NGFlags2String(ULONG);
char  *Mode2String(ULONG);
char  *Overscan2String(ULONG);
ULONG StringList2Data(struct StringList *, int);
void  FreeData(ULONG, int);
char  *GadgetIDCMP(struct WindowNode *);
ULONG CenterHoriz(ULONG);
ULONG CenterVert(ULONG);
char  *TextAttrName(struct TextAttr *);
char  *TextAttrStyle(struct TextAttr *);
char  *TextAttrFlags(struct TextAttr *);
char  *Text2Label(char *);
char  *UserDataName(char *);

/* EditTags.c */

void  EditTags(int, struct WindowNode *, struct NewGadgetNode *);
void  HandleTitleGadget(void);
void  HandleAvailableGadget(ULONG);
void  HandleSelectedGadget(ULONG);
void  HandleTagListGadget(ULONG);
void  HandleTagUseGadget(void);
void  HandleTagRemoveGadget(void);
void  HandleTagStringGadget(struct Gadget *);
void  HandleTagLabelGadget(struct Gadget *);
void  HandleTagAddGadget(void);
void  HandleTagDelGadget(void);
void  HandleTagUpGadget(void);
void  HandleTagDownGadget(void);
void  HandleTextPosGadget(ULONG);
void  RemakeScreen(void);
void  RemakeWindow(struct WindowNode *);
void  RemakeGadget(struct NewGadgetNode *, struct WindowNode *);

/* EditIDCMP.c */

void  EditIDCMP(void);

/* EditScreen.c */

void  EditScreen(void);
void  HandleScreenModeGadget(int);
void  HandleNtscPalGadget(int);
void  HandleOverscanGadget(int);
void  HandleInterlaceGadget(void);
void  HandleCustomColorsGadget(void);
void  HandleScreenDepthGadget(ULONG);
void  HandleScreenWidthGadget(void);
void  HandleScreenHeightGadget(void);
void  HandleScreenDWidthGadget(void);
void  HandleScreenDHeightGadget(void);
void  SetupNewScreen(void);

/* EditMenus.c */

void  EditMenus(void);
void  HandleMenuListGadget(ULONG);
void  HandleMenuStringGadget(void);
void  HandleMenuLabelGadget(void);
void  HandleMenuAddGadget(void);
void  HandleMenuDelGadget(void);
void  HandleMenuUpGadget(void);
void  HandleMenuDownGadget(void);
void  HandleMenuDisabledGadget(void);
void  HandleItemListGadget(ULONG);
void  HandleItemStringGadget(void);
void  HandleItemLabelGadget(void);
void  HandleItemAddGadget(void);
void  HandleItemDelGadget(void);
void  HandleItemUpGadget(void);
void  HandleItemDownGadget(void);
void  HandleItemDisabledGadget(void);
void  HandleSubListGadget(ULONG);
void  HandleSubStringGadget(void);
void  HandleSubLabelGadget(void);
void  HandleSubAddGadget(void);
void  HandleSubDelGadget(void);
void  HandleSubUpGadget(void);
void  HandleSubDownGadget(void);
void  HandleSubDisabledGadget(void);
void  HandleCheckItGadget(void);
void  HandleCheckedGadget(void);
void  HandleToggleGadget(void);
void  HandleExcludeGadget(void);
void  HandleCommKeyGadget(void);
void  ShowCurrentMenuList(int);
struct ItemNode *NextItem(struct MenuNode *);

/* EditColors.c */

void  EditColors(void);
ULONG Depth2Colors(struct Gadget *, WORD);
void  HandleScreenPaletteGadget(int);
void  HandleScreenRedGadget(int);
void  HandleScreenGreenGadget(int);
void  HandleScreenBlueGadget(int);

/* EditFont.c */

int   EditGadgetFont(struct NewGadgetNode *);
int   EditScreenFont(struct ScreenNode *);

/* Add.c */

ULONG  AddScreen(void);
int    AddMainWindow(void);
int    AddNewWindow(void);
int    AddWindow(struct WindowNode *);
int    AddAllWindows(void);
int    AddMenus(struct WindowNode *);
int    AddNewGadget(struct WindowNode *, SHORT, SHORT, SHORT, SHORT);
int    AddGadgets(struct WindowNode *);
int    CopyGadgets(struct WindowNode *);
int    CreateTagData(struct TagNode *, ULONG);
struct TagNode *AddTag(int, struct Node *, ULONG, ULONG);

/* Remove.c */

void  RemoveScreen(void);
void  RemoveWindow(struct WindowNode *);
void  RemoveGadgets(struct WindowNode *);
void  KillScreen(void);
void  KillWindow(struct Windownode *);
void  KillMainWindow(void);
void  KillSelectedGadgets(struct WindowNode *);
void  KillTagList(struct List *);
void  KillTag(struct TagNode *);
void  KillMenuList(struct List *);
void  KillItemList(struct List *);
void  KillSubList(struct List *);
void  KillNewGadgetList(struct List *);
void  KillGadgetList(struct List *);
void  KillWindowList(struct List *);
void  KillEditTagList(struct List *);
void  KillEditStringList(struct List *);
void  KillStringList(struct List *);
void  KillFontList(struct List *);
void  KillTextAttrList(struct List *);

/* Process.c */

void  TagScreen(UBYTE);
void  TagMainWindow(UBYTE);
void  TagWindow(struct WindowNode *, UBYTE);
void  TagWindows(UBYTE);
void  ProcessScreen(void);
void  ProcessWindows(void);

/* Handlers.c */

int   HandleGadgetDown(struct Gadget *, ULONG);
int   HandleGadgetUp(struct Gadget *, ULONG);
int   HandleGadgetMove(struct Gadget *, ULONG);
void  HandleRefresh(struct WindowNode *);
void  HandleChangeWindow(struct WindowNode *);
int   HandleMenuPick(ULONG);

/* HandleMouse.c */

void  HandleMouseButtons(struct WindowNode *, struct IntuiMessage *);
void  HandleMouseMove(struct WindowNode *, struct IntuiMessage *);
void  AbortSelection(void);

/* HandleIDCMP.c */

int   HandleMainIDCMP(struct Window *);
int   HandleRequesterIDCMP(struct Window *);
int   HandleArrayIDCMP(struct WindowNode *);

/* HandleKeys.c */

int   HandleMainVanillaKey(UWORD, UWORD);
int   HandleEditVanillaKey(UWORD, UWORD);
int   HandleMenuVanillaKey(UWORD, UWORD);
int   HandleIDCMPVanillaKey(UWORD, UWORD);
int   HandleScreenVanillaKey(UWORD, UWORD);
int   HandleColorsVanillaKey(UWORD, UWORD);

/* FileInput.c */

BOOL  GetBytes(void *, int, int, struct IFFHandle *, BOOL);
BOOL  GetString(char *, int, struct IFFHandle *, BOOL);
BOOL  OpenFile(void);
BOOL  ActuallyOpenFile(void);
void  OpenSettingsFile(void);
LONG  StdIOStream(struct Hook *, struct IFFHandle *, struct IFFStreamCmd *);
void  InitIFFasStdIO(struct IFFHandle *);

/* FileOutput.c */

BOOL  SaveFile(void);
BOOL  SaveSettingsFile(void);
BOOL  UpdateIcon(char *, char *, BOOL);
BOOL  WriteExeIcon(void);
VOID  DeleteIcon(char *);
BOOL  PutTag(ULONG, struct TagNode *, struct IFFHandle *, BOOL);
BOOL  PutBlock(ULONG, ULONG, struct IFFHandle *, BOOL);
BOOL  PopBlock(struct IFFHandle *, BOOL);
BOOL  PutBytes(void *, int, int, struct IFFHandle *, BOOL);
BOOL  PutString(char *, struct IFFHandle *, BOOL);
VOID  PutIFFError(LONG);

/* Generate.c */

void  GenerateCode(void);
BOOL  GenerateSource(void);
BOOL  GenerateUserHeader(void);
BOOL  GenerateUserTextHeader(void);
BOOL  GenerateTag(ULONG, struct TagNode *, int, int, int, FILE *);
BOOL  GenerateString(char *, FILE *, BOOL, char *);
char  *InvalidLabel(char *);
BOOL  GenerateRevision(void);
BOOL  CallBumpRev(void);
BOOL  BuildTextAttrList(void);
BOOL  GenerateMakefile(void);
BOOL  GenerateMakefileScript(void);
BOOL  CopyDefaultFile(char *);

/* GenerateHeader.c */

BOOL  GenerateHeader(void);

/* GenerateText.c */

BOOL  GenerateText(void);

/* GenerateTemplate.c */

BOOL  GenerateTemplate(void);

/* GenerateEvents.c */

BOOL  GenerateEventsFile(void);

/* Generate Files (each in own file) */

BOOL  GeneratePrecode(FILE *, BOOL);
BOOL  GenerateRequest(FILE *, BOOL);
BOOL  GenerateOpen(FILE *, BOOL);
BOOL  GenerateEventLoop(FILE *, BOOL);
BOOL  GenerateClose(FILE *, BOOL);
BOOL  GenerateScreen(FILE *, BOOL);
BOOL  GenerateWindow(FILE *, BOOL);
BOOL  GenerateFonts(FILE *, BOOL);
BOOL  GenerateWindowSignal(FILE *, BOOL);
BOOL  GenerateWindowIDCMP(FILE *, BOOL);
BOOL  GenerateARexxSignal(FILE *, BOOL);
BOOL  GenerateRemoveWindow(FILE *, BOOL);

