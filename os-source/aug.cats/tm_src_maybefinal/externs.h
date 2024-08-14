
/* Extern header */

extern char  title[];
extern char  date[];
extern char  cbmdate[];
extern char  merdate[];
extern char  extension[];
extern char  template[];
extern char  version[];
extern char  verstring[];

#ifdef TEST
extern UBYTE debuglevel;
extern UBYTE debugflag;
extern ULONG totalmem;
extern FILE  *confp;
#endif

extern LONG valuecode;
extern LONG availablecode;
extern LONG selectedcode;
extern LONG screenmodecode;
extern LONG ntscpalcode;
extern LONG oscancode;

extern ULONG maxdepth;
extern ULONG F_Main;
extern ULONG mainleft;
extern ULONG maintop;
extern ULONG viewwidth;
extern ULONG viewheight;
extern ULONG selectedstringnum;
extern ULONG selectedmenunum;
extern ULONG selecteditemnum;
extern ULONG selectedsubnum;
extern ULONG selectedobject;
extern ULONG currentcolor;
extern ULONG currentred;
extern ULONG currentgreen;
extern ULONG currentblue;
extern ULONG mainsignal;
extern ULONG editsignal;
extern ULONG IDCMPsignal;
extern ULONG menusignal;
extern ULONG screensignal;
extern ULONG colorssignal;
extern ULONG arraysignals;

extern ULONG defaultcolors[];
extern ULONG argarray[];
extern ULONG edittagtype;
extern ULONG positionvalues[];

extern ULONG constantvalues[];

extern ULONG mainidcmp;
extern ULONG editidcmp;
extern ULONG IDCMPidcmp;
extern ULONG menuidcmp;
extern ULONG screenidcmp;
extern ULONG arrayidcmp;
extern ULONG colorsidcmp;
extern ULONG gadgetidcmp;

extern APTR  VisualInfo;
extern APTR  request;
extern APTR  oldwindowptr;

extern UBYTE mainwindowpri;
extern UBYTE currenttopborder;
extern UBYTE modified;
extern UBYTE realgadgets;
extern UBYTE realmenus;
extern UBYTE commentflag;
extern UBYTE autotopborderflag;
extern UBYTE gridsize;
extern UBYTE pragmaflag;
extern UBYTE openflag;
extern UBYTE iconflag;
extern UBYTE newprojectflag;
extern UBYTE usersignalflag;
extern UBYTE arexxflag;
extern UBYTE selectflag;
extern UBYTE onegadgetflag;
extern UBYTE chipflag;
extern UBYTE userdataflag;
extern UBYTE aslflag;
extern UBYTE menupart;
extern UBYTE sleepcount;
extern UBYTE actuallymove;

extern SHORT topborderadjust;
extern SHORT textpos;
extern SHORT stringcount;
extern SHORT windowcount;
extern SHORT menucount;
extern SHORT itemcount;
extern SHORT subcount;
extern SHORT availablecount;
extern SHORT selectedcount;
extern SHORT xmark, ymark;
extern SHORT xstart, ystart;
extern SHORT xend, yend;
extern SHORT gadgetkind;
extern SHORT drag;
extern SHORT leftbound;
extern SHORT topbound;
extern SHORT rightbound;
extern SHORT bottombound;
extern SHORT diskfontcount;

extern struct Library *SysBase;
extern struct Library *DOSBase;
extern struct Library *IntuitionBase;
extern struct Library *GfxBase;
extern struct Library *GadToolsBase;
extern struct Library *AslBase;
extern struct Library *IconBase;
extern struct Library *DiskfontBase;
extern struct Library *UtilityBase;
extern struct Library *IFFParseBase;

extern struct Window *W_Main;
extern struct Window *W_Edit;
extern struct Window *W_IDCMP;
extern struct Window *W_Menu;
extern struct Window *W_Screen;
extern struct Window *W_Colors;

extern struct Menu   *M_Main;

extern struct Gadget *G_Main;
extern struct Gadget *G_MainContext;
extern struct Gadget *G_GadgetList;
extern struct Gadget *G_TestGadgets;
extern struct Gadget *G_TestMenus;

extern struct Gadget *G_AvailableTags;
extern struct Gadget *G_SelectedTags;
extern struct Gadget *G_TagLabel;
extern struct Gadget *G_String;
extern struct Gadget *G_ListView;
extern struct Gadget *G_Title;
extern struct Gadget *G_Label;
extern struct Gadget *G_TextPos;
extern struct Gadget *G_TagAdd;
extern struct Gadget *G_TagDel;
extern struct Gadget *G_TagUp;
extern struct Gadget *G_TagDown;
extern struct Gadget *G_TagType;
extern struct Gadget *G_Use;
extern struct Gadget *G_Remove;
extern struct Gadget *G_TopEdge;
extern struct Gadget *G_LeftEdge;
extern struct Gadget *G_Width;
extern struct Gadget *G_Height;
extern struct Gadget *G_Highlight;

extern struct Gadget *G_MenuList;
extern struct Gadget *G_MenuString;
extern struct Gadget *G_MenuLabel;
extern struct Gadget *G_MenuAdd;
extern struct Gadget *G_MenuDel;
extern struct Gadget *G_MenuUp;
extern struct Gadget *G_MenuDown;
extern struct Gadget *G_MenuDisabled;
extern struct Gadget *G_ItemList;
extern struct Gadget *G_ItemString;
extern struct Gadget *G_ItemLabel;
extern struct Gadget *G_ItemAdd;
extern struct Gadget *G_ItemDel;
extern struct Gadget *G_ItemUp;
extern struct Gadget *G_ItemDown;
extern struct Gadget *G_ItemDisabled;
extern struct Gadget *G_ItemCheckIt;
extern struct Gadget *G_ItemChecked;
extern struct Gadget *G_ItemToggle;
extern struct Gadget *G_ItemCommKey;
extern struct Gadget *G_ItemExclude;
extern struct Gadget *G_SubList;
extern struct Gadget *G_SubString;
extern struct Gadget *G_SubLabel;
extern struct Gadget *G_SubAdd;
extern struct Gadget *G_SubDel;
extern struct Gadget *G_SubUp;
extern struct Gadget *G_SubDown;
extern struct Gadget *G_SubDisabled;
extern struct Gadget *G_CheckIt;
extern struct Gadget *G_Checked;
extern struct Gadget *G_Toggle;
extern struct Gadget *G_CommKey;
extern struct Gadget *G_Exclude;

extern struct Gadget *G_ScreenMode;
extern struct Gadget *G_NtscPal;
extern struct Gadget *G_Overscan;
extern struct Gadget *G_Interlace;
extern struct Gadget *G_ScreenPalette;
extern struct Gadget *G_ScreenRed;
extern struct Gadget *G_ScreenGreen;
extern struct Gadget *G_ScreenBlue;
extern struct Gadget *G_ScreenWidth;
extern struct Gadget *G_ScreenHeight;
extern struct Gadget *G_ScreenDWidth;
extern struct Gadget *G_ScreenDHeight;
extern struct Gadget *G_ScreenDepth;
extern struct Gadget *G_ScreenTop;
extern struct Gadget *G_ScreenLeft;
extern struct Gadget *G_CustomColors;

extern struct Gadget *G_SIZEVERIFY;
extern struct Gadget *G_NEWSIZE;
extern struct Gadget *G_REFRESHWINDOW;
extern struct Gadget *G_MOUSEBUTTONS;
extern struct Gadget *G_MOUSEMOVE;
extern struct Gadget *G_GADGETDOWN;
extern struct Gadget *G_GADGETUP;
extern struct Gadget *G_REQSET;
extern struct Gadget *G_MENUPICK;
extern struct Gadget *G_CLOSEWINDOW;
extern struct Gadget *G_RAWKEY;
extern struct Gadget *G_REQVERIFY;
extern struct Gadget *G_REQCLEAR;
extern struct Gadget *G_MENUVERIFY;
extern struct Gadget *G_NEWPREFS;
extern struct Gadget *G_DISKINSERTED;
extern struct Gadget *G_DISKREMOVED;
extern struct Gadget *G_WBENCHMESSAGE;
extern struct Gadget *G_ACTIVEWINDOW;
extern struct Gadget *G_INACTIVEWINDOW;
extern struct Gadget *G_DELTAMOVE;
extern struct Gadget *G_VANILLAKEY;
extern struct Gadget *G_INTUITICKS;
extern struct Gadget *G_IDCMPUPDATE;
extern struct Gadget *G_MENUHELP;
extern struct Gadget *G_CHANGEWINDOW;

extern struct List WindowList;
extern struct List AvailableTagsList;
extern struct List SelectedTagsList;
extern struct List EditMenuList;
extern struct List TextAttrList;
extern struct List FontList;
extern struct List EmptyList;

extern struct NewGadgetNode *currentgadget;

extern struct WindowNode *currentwindow;
extern struct WindowNode *selectwindow;

extern struct EditTagNode *availabletag;
extern struct EditTagNode *selectedtag;

extern struct MenuNode *selectedmenu;
extern struct ItemNode *selecteditem;
extern struct SubNode  *selectedsub;

/***********************************/

extern USHORT chip SleepPointer[];

extern struct List far defaultlist;
extern struct Node far defaultnode[];

extern struct DimensionInfo far dimensioninfo;

extern struct EasyStruct far easystruct;

extern struct Requester far Requester;
extern struct Requester far R_Main;

extern struct TextAttr far TOPAZ80;

extern struct ScreenNode far screennode;
extern struct ScreenNode far newscreennode;

extern WORD far defaultxoom[];
extern WORD far defaultpens[];

extern struct StringData far defaultlabels;

/*
extern struct List defaultsourcelabellist;
extern struct Node sourcelabelnode0;
extern struct Node sourcelabelnode1;
extern struct Node sourcelabelnode2;
*/

extern char far *gadgettext[];

extern char far *constantlabels[];
extern char far *positionlabels[];
extern char far *modelabels[];
extern char far *ntscpallabels[];
extern char far *oscanlabels[];

extern struct EditItem far projectitem[];
extern struct EditItem far edititem[];
extern struct EditItem far macroitem[];
extern struct EditItem far settingitem[];
extern struct EditItem far useritem[];

extern char far defaulttool[];
extern char far filename[];
extern char far settingsname[];
extern char far fullname[];
extern char far savefilename[];
extern char far savepathname[];
extern char far settingsfilename[];
extern char far settingspathname[];
extern char far string[];
extern char far string2[];
extern char far tabstring[];
extern char far toolpath[];

extern struct TagItem far tagarray[];

extern struct NewMenu far NM_Main[];

extern struct AvailableIDCMP far AvailableIDCMP[];
extern struct AvailableTags  far AvailableScreenTags[];
extern struct AvailableTags  far AvailableWindowTags[];
extern struct AvailableTags  far AvailableBUGadgetTags[];
extern struct AvailableTags  far AvailableCBGadgetTags[];
extern struct AvailableTags  far AvailableLVGadgetTags[];
extern struct AvailableTags  far AvailableMXGadgetTags[];
extern struct AvailableTags  far AvailableTXGadgetTags[];
extern struct AvailableTags  far AvailableNMGadgetTags[];
extern struct AvailableTags  far AvailableCYGadgetTags[];
extern struct AvailableTags  far AvailablePAGadgetTags[];
extern struct AvailableTags  far AvailableSCGadgetTags[];
extern struct AvailableTags  far AvailableSLGadgetTags[];
extern struct AvailableTags  far AvailableSTGadgetTags[];
extern struct AvailableTags  far AvailableINGadgetTags[];

