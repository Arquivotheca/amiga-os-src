/* misc.c
 *
 */

#include "datatypesbase.h"

/*****************************************************************************/

void CloseClipIFF (struct DataTypesLib *dtl, struct IFFHandle *iff)
{
#if 0
    struct ClipboardHandle *clip = (struct ClipboardHandle *) iff->iff_Stream;
    UBYTE error = 0;

#if 0
kprintf ("io_Actual=%ld\n", clip->cbh_Req.io_Actual);

    if (clip->cbh_Req.io_Actual)
    {
	clip->cbh_Req.io_Offset = 0x7FFFFFF0;
	clip->cbh_Req.io_Length = 1;
	clip->cbh_Req.io_Data   = NULL;
	clip->cbh_Req.io_Error  = 0;
	clip->cbh_Req.io_ClipID = 0;
	clip->cbh_Req.io_Command = CMD_READ;
	error = DoIO ((struct IORequest *) &clip->cbh_Req);
    }
#endif

kprintf ("CloseClipIFF %08lx, actual=%ld, error=%ld (%ld)\n",
	 iff, clip->cbh_Req.io_Actual, clip->cbh_Req.io_Error, (ULONG)error);
#endif

    CloseIFF (iff);
}

/*****************************************************************************/

void strcpy (char *dst, char *src)
{
    if (src && dst)
    {
	do
	    *dst++ = *src;
	while (*src++);
    }
}

/*****************************************************************************/

int strlen (char *src)
{
    int len = 0;

    if (src)
    {
	do
	    len++;
	while (*src++);
    }
    return (len);
}

/*****************************************************************************/

ULONG notifyAttrChanges (struct DataTypesLib *dtl, Object *o, VOID *ginfo, ULONG flags, ULONG tag1,...)
{
    return DoMethod (o, OM_NOTIFY, &tag1, ginfo, flags);
}

/*****************************************************************************/

ULONG dogadgetmethod (struct DataTypesLib *dtl, struct Gadget *g, struct Window *w, struct Requester *r, ULONG data, ...)
{
    return DoGadgetMethodA ((struct Gadget *)g, w, r, (Msg) &data);
}

/*****************************************************************************/

ULONG notifyGAttrChanges (struct DataTypesLib *dtl, Object *o, struct Window *w, struct Requester *r, ULONG flags, ULONG tag1,...)
{
    return dogadgetmethod (dtl, (struct Gadget *)o, w, r, OM_NOTIFY, &tag1, NULL, flags);
}

/*****************************************************************************/

Object *newobject (struct DataTypesLib *dtl, Class *cl, ULONG data,...)
{
    return (NewObjectA (cl, NULL, (struct TagItem *) & data));
}

/*****************************************************************************/

ULONG setattrs (struct DataTypesLib *dtl, Object *o, Tag tag1, ...)
{
    return (SetAttrsA (o, (struct TagItem *)&tag1));
}
