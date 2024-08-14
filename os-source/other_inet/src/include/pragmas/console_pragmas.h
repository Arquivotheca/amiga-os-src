/*
**	$Id: console_pragmas.h,v 36.6 90/11/07 15:33:36 darren Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "console.device" */
#pragma libcall ConsoleDevice CDInputHandler 2a 9802
#pragma libcall ConsoleDevice RawKeyConvert 30 A19804
/*--- functions in V36 or higher (Release 2.0) ---*/
/* System-private function: GetConSnip */
#pragma libcall ConsoleDevice GetConSnip 36 00
/* System-private function: SetConSnip */
#pragma libcall ConsoleDevice SetConSnip 3c 801
/* System-private function: AddConSnipHook */
#pragma libcall ConsoleDevice AddConSnipHook 42 801
/* System-private function: RemConSnipHook */
#pragma libcall ConsoleDevice RemConSnipHook 48 801
