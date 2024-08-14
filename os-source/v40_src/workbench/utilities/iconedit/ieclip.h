#ifndef IECLIP_H
#define IECLIP_H


/*****************************************************************************/


#include <exec/types.h>
#include <intuition/intuition.h>
#include <exec/types.h>
#include <libraries/iffparse.h>

#include "dynamicimages.h"
#include "iemain.h"
/* #include "ieiff.h" */


/*****************************************************************************/


/* Our errors */
#define	RC_EMPTY_CLIP		-1L	/* Clipboard is empty */
#define	RC_NOT_ENOUGH_MEMORY	-4L	/* not enough memory */
#define	RC_COULDNT_READ		-5L	/* Couldn't read a chunk */
#define	RC_BAD_IFF		-8L	/* Clipboard contains a bad IFF */
#define	RC_INVALID_TYPE		-9L	/* don't understand FORM TYPE */


/*****************************************************************************/


BOOL CutClipFunc(WindowInfoPtr wi);
BOOL CopyClipFunc(WindowInfoPtr wi);
BOOL PasteClipFunc(WindowInfoPtr wi);
BOOL OpenClipFunc(WindowInfoPtr wi);
BOOL SaveClipFunc(WindowInfoPtr wi);
BOOL ShowClipFunc(WindowInfoPtr wi);

struct IFFHandle * OpenClip(LONG unit);
VOID CloseClip(struct IFFHandle *iff);
LONG QueryClip(struct IFFHandle *iff, ULONG *clipType, ULONG *clipID);


/*****************************************************************************/


#endif /* IECLIP_H */
