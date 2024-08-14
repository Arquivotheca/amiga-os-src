/*
 * $Id: viewbytext.c,v 38.2 92/02/13 14:22:22 mks Exp $
 *
 * $Log:	viewbytext.c,v $
 * Revision 38.2  92/02/13  14:22:22  mks
 * Fixed the text position problem (it was left by 1 pixel)
 * 
 * Revision 38.1  91/06/24  11:39:18  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include "proto.h"
#include "string.h"
#include <exec/types.h>
#include <dos/datetime.h>
#include "workbench.h"
#include "workbenchbase.h"
#include "wbinternal.h"
#include "global.h"
#include "support.h"
#include "quotes.h"

void FormatObj(obj)
struct WBObject *obj;
{
    char *ptr;
    char datebuff[LEN_DATSTRING*2+1];
    int bits, bitval;

    MP(("FormatObj: enter, obj=$%lx (%s)\n",
	obj, obj->wo_Name));

    datetostring(datebuff, &obj->wo_DateStamp);

    sprintf(obj->wo_NameBuffer, "%10ld hsparwed %s",
	obj->wo_FileSize, datebuff);

    /* size */
    if (CheckForType(obj,ISDRAWER))
    {
    char *c=Quote(Q_WBDRAWER);
    char *t=obj->wo_NameBuffer;

	while (*c) *t++=*c++;
    }

    /* protection */
    ptr = &obj->wo_NameBuffer[11];
    bits = 8; /* # of bits to do */
    bitval = 128; /* bit value */
    do {
    	if (!((obj->wo_Protection ^ 15) & bitval)) {
            *ptr = '-';
    	}
	ptr++;
    	bitval >>= 1;
    } while (--bits);
    MP(("\t             h s p a r w e d\n"));
    MP(("\tProtection = %ld %ld %ld %ld %ld %ld %ld %ld\n\tbuffer='%s'\n",
	obj->wo_Protection & 128, obj->wo_Protection & 64,
	obj->wo_Protection & 32, obj->wo_Protection & 16,
	obj->wo_Protection & 8, obj->wo_Protection & 4,
	obj->wo_Protection & 2, obj->wo_Protection & 1,
	&obj->wo_NameBuffer[11]));
    MP(("FormatObj: exit\n"));
}

void PrepareViewByText(obj)
struct WBObject *obj;
{
    struct WorkbenchBase *wb = getWbBase();
    struct Image *im;
    struct TextFont *tf = wb->wb_TextFont;
    struct RastPort *rp = &wb->wb_TextRast;
    struct BitMap BMap;

    im = &obj->wo_NameImage;

    /* ZZZ: TEXT HACK: call TextExtent() when it's ready */
    im->Width = wb->wb_MaxTextWidth;
/*  im->Width = textLength(obj->wo_Name); /* */
/*  im->Width = WONAME_ACTIVE * tf->tf_XSize; /* */
    im->PlanePick = 1;
    im->PlaneOnOff = 0;
    im->Height = tf->tf_YSize;
    im->Depth = 2;

    BltClear((PLANEPTR)(im->ImageData), obj->wo_ImageSize, NULL); /* clear out any old data */

    /* now render the name into the image data */
    SetRastPort(rp, im, &BMap, im->ImageData);
    rp->Mask = 1; /* only the first plane */

    SetAPen(rp, 1);
    SetBPen(rp, 0);
    SetDrMd(rp, JAM2);
    SetFont(rp, wb->wb_TextFont);
    Move(rp, 0, tf->tf_Baseline);
    Text(rp, obj->wo_Name, strlen(obj->wo_Name));
}

/*
    allocate the NameImage data pointer
    set icon names (Master, Select, Utility and Sibling nodes)
    call PrepareViewByText
*/
growNameImage( obj )
struct WBObject *obj;
{
    struct WorkbenchBase *wb = getWbBase();
    void *ptr;
    int newlen;

    if (obj != wb->wb_RootObject)
    {
	/* ZZZ: we should use TextExtend() here */
	if (!(newlen = wb->wb_MaxTextWidth * wb->wb_TextFont->tf_YSize)) return(0);

	MP(("growNameImage: obj=%lx, name='%s', ImageSize=%ld, newlen=%ld\n",obj, obj->wo_Name, obj->wo_ImageSize, newlen));
	if ((!obj->wo_NameImage.ImageData) || (newlen > obj->wo_ImageSize))
	{
	    DP(("growNameImage: oldsize=%ld, newsize=%ld, calling ObjAlloc for $%lx (%s)\n",obj->wo_ImageSize, newlen, obj, obj->wo_Name));
	    ptr = ObjAllocChip(obj, newlen);
	    if (!ptr)
	    {
		MP(("growNameImage: error, ObjAlloc failed\n"));
		NoMem();
		return(0);
	    }
	    obj->wo_NameImage.ImageData = ptr;
	    obj->wo_ImageSize = newlen;
	}
    }
    /* ZZZ: THIS CALL SHOULD REALLY NOT BE HERE AS IT AS NOTHING TO DO WITH
	    VIEWBYTEXT MODE.  IT SHOULD BE IN THE ROUTINE CALLING THIS ONE.
    */
    SetIconNames(obj);

    if (obj != wb->wb_RootObject) PrepareViewByText(obj);
    return(1);
}

MakeNameGadget(obj, viewmode)
struct WBObject *obj;
int viewmode;
{
    struct WorkbenchBase *wb = getWbBase();
    struct Gadget *gad;
    struct TextFont *tf;
    int len;

    if (obj == wb->wb_RootObject) {
	MP(("MakeNameGadget: passing on root object\n"));
	return(0);
    }

    gad = &obj->wo_Gadget;
    if (viewmode <= DDVM_BYICON) {
	tf = wb->wb_IconFont;
	len = TextLength(&wb->wb_IconRast, obj->wo_Name, strlen(obj->wo_Name));
	obj->wo_NameXOffset = ((gad->Width - len) >> 1)+1;
	obj->wo_NameYOffset = gad->Height + tf->tf_Baseline;
    }
    else { /* all other modes are textual */
	tf = wb->wb_TextFont;
	gad->Height = tf->tf_YSize;
	gad->Width = wb->wb_MaxTextWidth;
	gad->Flags = GADGIMAGE | GADGHCOMP;
	obj->wo_NameXOffset = 0;
	obj->wo_NameYOffset = tf->tf_Baseline;
	FormatObj(obj);
    }
    return(0);
}
