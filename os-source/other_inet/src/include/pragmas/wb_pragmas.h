/*
**	$Id: wb_pragmas.h,v 38.4 92/05/31 16:34:03 mks Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "workbench.library" */
/*--- functions in V36 or higher (Release 2.0) ---*/


/* System-private function: UpdateWorkbench */
#pragma libcall WorkbenchBase UpdateWorkbench 1e 09803
/* System-private function: QuoteWorkbench */
#pragma libcall WorkbenchBase QuoteWorkbench 24 001

/* System-private function: StartWorkbench */
#pragma libcall WorkbenchBase StartWorkbench 2a 1002


#pragma libcall WorkbenchBase AddAppWindowA 30 A981005
#ifdef __SASC_60
#pragma tagcall WorkbenchBase AddAppWindow 30 A981005
#endif

#pragma libcall WorkbenchBase RemoveAppWindow 36 801

#pragma libcall WorkbenchBase AddAppIconA 3c CBA981007
#ifdef __SASC_60
#pragma tagcall WorkbenchBase AddAppIcon 3c CBA981007
#endif

#pragma libcall WorkbenchBase RemoveAppIcon 42 801

#pragma libcall WorkbenchBase AddAppMenuItemA 48 A981005
#ifdef __SASC_60
#pragma tagcall WorkbenchBase AddAppMenuItem 48 A981005
#endif

#pragma libcall WorkbenchBase RemoveAppMenuItem 4e 801

/*--- functions in V39 or higher (Release 3) ---*/

/* System-private function: WBConfig */
#pragma libcall WorkbenchBase WBConfig 54 1002

#pragma libcall WorkbenchBase WBInfo 5a A9803

/*--- (5 function slots reserved here) ---*/

