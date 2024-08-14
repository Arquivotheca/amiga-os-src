/* "gadtools.library"*/
/*--- functions in V36 or higher (distributed as Release 2.0) ---*/
/**/
/* Gadget Functions*/
/**/
#pragma libcall GadToolsBase CreateGadgetA 1E A98004
#pragma libcall GadToolsBase FreeGadgets 24 801
#pragma libcall GadToolsBase GT_SetGadgetAttrsA 2A BA9804
/**/
/* Menu functions*/
/**/
#pragma libcall GadToolsBase CreateMenusA 30 9802
#pragma libcall GadToolsBase FreeMenus 36 801
#pragma libcall GadToolsBase LayoutMenuItemsA 3C A9803
#pragma libcall GadToolsBase LayoutMenusA 42 A9803
/**/
/* Misc Event-Handling Functions*/
/**/
#pragma libcall GadToolsBase GT_GetIMsg 48 801
#pragma libcall GadToolsBase GT_ReplyIMsg 4E 901
#pragma libcall GadToolsBase GT_RefreshWindow 54 9802
#pragma libcall GadToolsBase GT_BeginRefresh 5A 801
#pragma libcall GadToolsBase GT_EndRefresh 60 0802
#pragma libcall GadToolsBase GT_FilterIMsg 66 901
#pragma libcall GadToolsBase GT_PostFilterIMsg 6C 901
#pragma libcall GadToolsBase CreateContext 72 801
/**/
/* Rendering Functions*/
/**/
#pragma libcall GadToolsBase DrawBevelBoxA 78 93210806
/**/
/* Visuals Functions*/
/**/
#pragma libcall GadToolsBase GetVisualInfoA 7E 9802
#pragma libcall GadToolsBase FreeVisualInfo 84 801
/**/
/* Reserved entries*/
/**/
/*pragma libcall GadToolsBase GTReserved0 8A 0*/
/*pragma libcall GadToolsBase GTReserved1 90 0*/
/*pragma libcall GadToolsBase GTReserved2 96 0*/
/*pragma libcall GadToolsBase GTReserved3 9C 0*/
/*pragma libcall GadToolsBase GTReserved4 A2 0*/
/*pragma libcall GadToolsBase GTReserved5 A8 0*/
