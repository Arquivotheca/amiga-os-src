/*
**	$Id: amigaguide_pragmas.h,v 39.2 92/07/23 17:36:58 davidj Exp Locker: davidj $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "amigaguide.library" */
/*--- functions in V39 or higher (Release 3) ---*/

#pragma libcall AmigaGuideBase AGARexxHost 1e 9802
#pragma libcall AmigaGuideBase LockAmigaGuideBase 24 801
#pragma libcall AmigaGuideBase UnlockAmigaGuideBase 2a 001
/* System-private function: ExpungeDataBases */
#pragma libcall AmigaGuideBase ExpungeDataBases 30 001
#pragma libcall AmigaGuideBase OpenAmigaGuideA 36 9802
#ifdef __SASC_60
#pragma tagcall AmigaGuideBase OpenAmigaGuide 36 9802
#endif
#pragma libcall AmigaGuideBase OpenAmigaGuideAsyncA 3c 0802
#ifdef __SASC_60
#pragma tagcall AmigaGuideBase OpenAmigaGuideAsync 3c 0802
#endif
#pragma libcall AmigaGuideBase CloseAmigaGuide 42 801
#pragma libcall AmigaGuideBase AmigaGuideSignal 48 801
#pragma libcall AmigaGuideBase GetAmigaGuideMsg 4e 801
#pragma libcall AmigaGuideBase ReplyAmigaGuideMsg 54 801
#pragma libcall AmigaGuideBase SetAmigaGuideContextA 5a 10803
#ifdef __SASC_60
#pragma tagcall AmigaGuideBase SetAmigaGuideContext 5a 10803
#endif
#pragma libcall AmigaGuideBase SendAmigaGuideContextA 60 0802
#ifdef __SASC_60
#pragma tagcall AmigaGuideBase SendAmigaGuideContext 60 0802
#endif
#pragma libcall AmigaGuideBase SendAmigaGuideCmdA 66 10803
#ifdef __SASC_60
#pragma tagcall AmigaGuideBase SendAmigaGuideCmd 66 10803
#endif
#pragma libcall AmigaGuideBase SetAmigaGuideAttrsA 6c 9802
#ifdef __SASC_60
#pragma tagcall AmigaGuideBase SetAmigaGuideAttrs 6c 9802
#endif
#pragma libcall AmigaGuideBase GetAmigaGuideAttr 72 98003
/* System-private function: SetAmigaGuideHook */
#pragma libcall AmigaGuideBase SetAmigaGuideHook 78 A90804
/* System-private function: LoadXRef */
#pragma libcall AmigaGuideBase LoadXRef 7e 9802
/* System-private function: ExpungeXRef */
#pragma libcall AmigaGuideBase ExpungeXRef 84 00
#pragma libcall AmigaGuideBase AddAmigaGuideHostA 8a 90803
#ifdef __SASC_60
#pragma tagcall AmigaGuideBase AddAmigaGuideHost 8a 90803
#endif
#pragma libcall AmigaGuideBase RemoveAmigaGuideHostA 90 9802
#ifdef __SASC_60
#pragma tagcall AmigaGuideBase RemoveAmigaGuideHost 90 9802
#endif
/* System-private function: OpenE */
#pragma libcall AmigaGuideBase OpenE 96 21803
/* System-private function: LockE */
#pragma libcall AmigaGuideBase LockE 9c 21803
/* System-private function: CopyPathList */
#pragma libcall AmigaGuideBase CopyPathList a2 801
/* System-private function: AddPathEntries */
#pragma libcall AmigaGuideBase AddPathEntries a8 0802
/* System-private function: FreePathList */
#pragma libcall AmigaGuideBase FreePathList ae 801
/* System-private function: ParsePathString */
#pragma libcall AmigaGuideBase ParsePathString b4 18003
/* System-private function: OpenDataBase */
#pragma libcall AmigaGuideBase OpenDataBase ba 9802
/* System-private function: LoadNode */
#pragma libcall AmigaGuideBase LoadNode c0 A9803
/* System-private function: UnloadNode */
#pragma libcall AmigaGuideBase UnloadNode c6 A9803
/* System-private function: CloseDataBase */
#pragma libcall AmigaGuideBase CloseDataBase cc 801
#pragma libcall AmigaGuideBase GetAmigaGuideString d2 001
