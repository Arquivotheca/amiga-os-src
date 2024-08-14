/* "workbench.library" */
/*--- functions in V36 or higher (Release 2.0) ---*/


/*pragma libcall WorkbenchBase UpdateWorkbench 1e 09803*/
/*pragma libcall WorkbenchBase QuoteWorkbench 24 001*/

/*pragma libcall WorkbenchBase StartWorkbench 2a 1002*/


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

/*pragma libcall WorkbenchBase WBConfig 54 1002*/

#pragma libcall WorkbenchBase WBInfo 5a A9803

/*--- (5 function slots reserved here) ---*/

