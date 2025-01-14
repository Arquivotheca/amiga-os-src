#ifndef IEMENUS_H
#define IEMENUS_H


/*****************************************************************************/


#include <exec/types.h>
#include <intuition/intuition.h>
#include <workbench/workbench.h>

#include "iemain.h"
#include "dynamicimages.h"
#include "texttable.h"


/*****************************************************************************/


void CreateWinTitle (WindowInfoPtr wi, UBYTE *name);
BOOL CreateIconMenus(WindowInfoPtr wi);
VOID CheckMenuItem(WindowInfoPtr wi, USHORT menunum, UWORD item);
VOID SetHilite(WindowInfoPtr wi, struct DiskObject *dob, UWORD mhilite);
VOID SetType(WindowInfoPtr wi, struct DiskObject *dob, UWORD mtype);
BOOL HandleMenuEvent(WindowInfoPtr wi, UWORD code);
BOOL NewFunc(WindowInfoPtr wi);
VOID OpenNamedIcon(WindowInfoPtr wi, STRPTR name, BOOL real);
BOOL OpenFunc(WindowInfoPtr wi);
BOOL SaveFunc(WindowInfoPtr wi);
BOOL SaveAsFunc(WindowInfoPtr wi);
BOOL SaveDefFunc(WindowInfoPtr wi);
BOOL QuitFunc(WindowInfoPtr wi);
BOOL ExchangeFunc(WindowInfoPtr wi);
BOOL CopyFunc(WindowInfoPtr wi);
VOID LoadIAI(WindowInfoPtr wi, USHORT lmode, struct DiskObject *dob, USHORT cur);
VOID LoadIconAsImage(WindowInfoPtr wi, USHORT lmode);
BOOL LStdFunc(WindowInfoPtr wi);
BOOL LAltFunc(WindowInfoPtr wi);
BOOL LBthFunc(WindowInfoPtr wi);
BOOL LoadIFFImage(WindowInfoPtr wi, STRPTR name, USHORT cur, BOOL errs);
BOOL LIFFFunc(WindowInfoPtr wi);
BOOL SStdFunc(WindowInfoPtr wi);
BOOL SIFFFunc(WindowInfoPtr wi);
BOOL RestoreFunc(WindowInfoPtr wi);
VOID Recolor(struct Image *im);
VOID AutoTopLeft(DynamicImagePtr si);
VOID FindTopLeft(struct RastPort * rp, struct Rectangle * clip, SHORT mw, SHORT mh);
BOOL CheckForChanges(WindowInfoPtr wi, AppStringsID body, AppStringsID gadgets);
BOOL SaveIFFBrush(WindowInfoPtr wi, STRPTR name, DynamicImagePtr di);


/*****************************************************************************/


#endif /* IEMENUS_H */
