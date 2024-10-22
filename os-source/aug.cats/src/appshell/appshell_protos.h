#ifndef CLIB_APPSHELL_PROTOS_H
#define	CLIB_APPSHELL_PROTOS_H

/************************************************************************
 *                                                                      *
 *                            Preliminary                               *
 *                        Amiga AppShell (tm)                           *
 *                                                                      *
 *   Copyright (c) 1990  Commodore-Amiga, Inc.  All Rights Reserved.    *
 *                                                                      *
 *   This software and information is proprietary, preliminary, and     *
 *   subject to change without notice.                                  *
 *                                                                      *
 *                            DISCLAIMER                                *
 *                                                                      *
 *   THIS SOFTWARE IS PROVIDED "AS IS".                                 *
 *   NO REPRESENTATIONS OR WARRANTIES ARE MADE WITH RESPECT TO THE      *
 *   ACCURACY, RELIABILITY, PERFORMANCE, CURRENTNESS, OR OPERATION      *
 *   OF THIS SOFTWARE, AND ALL USE IS AT YOUR OWN RISK.                 *
 *   NEITHER COMMODORE NOR THE AUTHORS ASSUME ANY RESPONSIBILITY OR     *
 *   LIABILITY WHATSOEVER WITH RESPECT TO YOUR USE OF THIS SOFTWARE.    *
 *                                                                      *
 *                          Non-Disclosure                              *
 *                                                                      *
 *   This information is not to be disclosed to any other company,      *
 *   individual or party.  Discussion is to be restricted to CBM        *
 *   approved discussion areas, such as a private area on bix           *
 *   dedicated to BETA versions of the Amiga AppShell.                  *
 *                                                                      *
 ************************************************************************
 * appshell_protos.h
 * Copyright (C) 1990,1991 Commodore-Amiga, Inc.
 *
 * C function prototypes for the Amiga AppShell
 *
 */


/* Message handler initialization */
struct MsgHandler *setup_arexxA ( struct AppInfo *ai , struct TagItem *tl );
struct MsgHandler *setup_dosA ( struct AppInfo *ai , struct TagItem *tl );
struct MsgHandler *setup_idcmpA ( struct AppInfo *ai , struct TagItem *tl );
struct MsgHandler *setup_sipcA ( struct AppInfo *ai , struct TagItem *tl );
struct MsgHandler *setup_toolA ( struct AppInfo *ai , struct TagItem *tl );
struct MsgHandler *setup_wbA ( struct AppInfo *ai , struct TagItem *tl );


/* Message handler related */
ULONG APSHSignal (struct AppInfo *ai, LONG bit);
BOOL HandleApp ( int argc , char **argv , struct WBStartup *wbm , struct TagItem *anchor );
BOOL HandleAppAsync ( struct TagItem *anchor , struct MsgPort *sipc );
APTR HandlerDataA ( struct AppInfo *ai , struct TagItem *tl );
BOOL HandlerFuncA ( struct AppInfo *ai , struct TagItem *tl );
VOID RemoveMsgPort ( struct MsgPort *mp );


/* Project related */
BOOL AddProjects ( struct AppInfo *ai , LONG numargs , struct WBArg *args , struct TagItem *tl );
BOOL ExpandPattern (struct AppInfo *,STRPTR,struct TagItem *);
VOID FreeProject ( struct ProjNode *pn );
VOID FreeProjects ( struct AppInfo *ai , struct TagItem *tl );
BOOL GetBaseInfo ( struct AppInfo *ai , int argc , char **argv , struct WBStartup *wbm );
struct ProjNode *GetProjNode ( struct AppInfo *ai , LONG cnt , struct TagItem *tl );
struct DiskObject *IconFromWBArg ( struct WBArg *arg );
struct ProjNode *NewProject ( struct AppInfo *ai , STRPTR anchor , struct TagItem *tl );
BOOL RemoveProject ( struct AppInfo *ai , STRPTR cmd , struct TagItem *tl );
VOID RenumProjects ( struct AppInfo *ai , STRPTR cmd , struct TagItem *tl );
VOID SwapProjNodes ( struct AppInfo *ai , struct ProjNode *sn1 , struct ProjNode *sn2 , struct TagItem *tl );
LONG UpdateProject (struct AppInfo * ai, struct ProjNode * pn, struct WBArg * arg);


/* Function table */
BOOL AddFuncEntries ( struct AppInfo *ai , struct Funcs *fels );
BOOL AddFuncEntry ( struct AppInfo *ai , struct Funcs *sfe );
VOID FreeFuncEntries ( struct AppInfo *ai );
VOID FreeFuncEntry (struct AppInfo * ai, struct FuncEntry * fe);
struct FuncEntry *GetFuncEntry ( struct AppInfo *ai , STRPTR anchor , ULONG fid );
ULONG GetFuncID ( struct AppInfo *ai , STRPTR anchor );
STRPTR GetFuncName ( struct AppInfo *ai , ULONG id );
BOOL PerfFunc ( struct AppInfo *ai , ULONG fid , STRPTR cmd , struct TagItem *tl );


/* Intuition related */
VOID APSHClearPointer ( struct AppInfo *ai , struct TagItem *tl );
BOOL APSHGetGadgetInfo ( struct AppInfo *ai , STRPTR wname , STRPTR gname , ULONG *window , ULONG *gadget );
BOOL APSHGetWindowInfo ( struct AppInfo *ai , STRPTR wname , ULONG *window );
VOID APSHSetWaitPointer ( struct AppInfo *ai , struct TagItem *tl );


/* AppShell object manipulation */
VOID DisposeAPSHObject(struct AppInfo *, APTR);
ULONG GetAPSHAttr(struct AppInfo *, LONG, APTR, APTR);
LONG LockAppInfo (struct AppInfo *ai);
APTR NewAPSHObject(struct AppInfo *, LONG, struct TagItem *);
ULONG SetAPSHAttr(struct AppInfo *, APTR, struct TagItem *);
VOID UnlockAppInfo (LONG key);


/* Simple InterProcess Communication */
APTR CloseSIPC ( APTR ash , struct TagItem *tl );
APTR OpenSIPC ( struct AppInfo *ai , STRPTR name , struct TagItem *tl );
BOOL SendSIPCMessage ( STRPTR name , ULONG type , VOID *data );
BOOL SendSIPCMessageP ( struct MsgPort *port , struct MsgPort *reply , ULONG type , VOID *data );


/* Text table */
STRPTR GetText ( struct AppInfo *ai , ULONG baseid , ULONG tid , STRPTR def );
BOOL NotifyUser ( struct AppInfo *ai , STRPTR msg , struct TagItem *tl );
STRPTR PrepText ( struct AppInfo *ai , ULONG baseid , ULONG tid , int va_alist );


/* Parsing and string manipulation */
STRPTR BuildParseLine ( STRPTR line , ULONG *argc , STRPTR *argv );
struct Node *FindNameI (struct List *, STRPTR);
STRPTR FindType ( STRPTR *array , STRPTR name , STRPTR defvalue );
VOID FreeParseLine ( STRPTR clone );
VOID LMatchEnd (struct AnchorList * al);
LONG LMatchFirst (struct List * list, STRPTR pat, struct AnchorList * al);
LONG LMatchNext (struct AnchorList * al);
BOOL MatchValue ( STRPTR type , STRPTR value );
ULONG ParseLine ( STRPTR line , STRPTR *argv );
BOOL QStrCmpI ( STRPTR str1 , STRPTR str2 );

#endif /* CLIB_APPSHELL_PROTOS_H */
