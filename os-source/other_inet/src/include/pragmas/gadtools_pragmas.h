/*
**	$Id: gadtools_pragmas.h,v 39.2 92/03/24 15:14:44 peter Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "gadtools.library" */

/*--- functions in V36 or higher (Release 2.0) ---*/

/* Gadget Functions */

#pragma libcall GadToolsBase CreateGadgetA 1e A98004
#ifdef __SASC_60
#pragma tagcall GadToolsBase CreateGadget 1e A98004
#endif
#pragma libcall GadToolsBase FreeGadgets 24 801
#pragma libcall GadToolsBase GT_SetGadgetAttrsA 2a BA9804
#ifdef __SASC_60
#pragma tagcall GadToolsBase GT_SetGadgetAttrs 2a BA9804
#endif

/* Menu functions */

#pragma libcall GadToolsBase CreateMenusA 30 9802
#ifdef __SASC_60
#pragma tagcall GadToolsBase CreateMenus 30 9802
#endif
#pragma libcall GadToolsBase FreeMenus 36 801
#pragma libcall GadToolsBase LayoutMenuItemsA 3c A9803
#ifdef __SASC_60
#pragma tagcall GadToolsBase LayoutMenuItems 3c A9803
#endif
#pragma libcall GadToolsBase LayoutMenusA 42 A9803
#ifdef __SASC_60
#pragma tagcall GadToolsBase LayoutMenus 42 A9803
#endif

/* Misc Event-Handling Functions */

#pragma libcall GadToolsBase GT_GetIMsg 48 801
#pragma libcall GadToolsBase GT_ReplyIMsg 4e 901
#pragma libcall GadToolsBase GT_RefreshWindow 54 9802
#pragma libcall GadToolsBase GT_BeginRefresh 5a 801
#pragma libcall GadToolsBase GT_EndRefresh 60 0802
#pragma libcall GadToolsBase GT_FilterIMsg 66 901
#pragma libcall GadToolsBase GT_PostFilterIMsg 6c 901
#pragma libcall GadToolsBase CreateContext 72 801

/* Rendering Functions */

#pragma libcall GadToolsBase DrawBevelBoxA 78 93210806
#ifdef __SASC_60
#pragma tagcall GadToolsBase DrawBevelBox 78 93210806
#endif

/* Visuals Functions */

#pragma libcall GadToolsBase GetVisualInfoA 7e 9802
#ifdef __SASC_60
#pragma tagcall GadToolsBase GetVisualInfo 7e 9802
#endif
#pragma libcall GadToolsBase FreeVisualInfo 84 801

/* Reserved entries */

/* System-private function: GTReserved0 */
#pragma libcall GadToolsBase GTReserved0 8a 00
/* System-private function: GTReserved1 */
#pragma libcall GadToolsBase GTReserved1 90 00
/* System-private function: GTReserved2 */
#pragma libcall GadToolsBase GTReserved2 96 00
/* System-private function: GTReserved3 */
#pragma libcall GadToolsBase GTReserved3 9c 00
/* System-private function: GTReserved4 */
#pragma libcall GadToolsBase GTReserved4 a2 00
/* System-private function: GTReserved5 */
#pragma libcall GadToolsBase GTReserved5 a8 00

/*--- functions in V39 or higher (Release 3) ---*/

#pragma libcall GadToolsBase GT_GetGadgetAttrsA ae BA9804
#ifdef __SASC_60
#pragma tagcall GadToolsBase GT_GetGadgetAttrs ae BA9804
#endif

