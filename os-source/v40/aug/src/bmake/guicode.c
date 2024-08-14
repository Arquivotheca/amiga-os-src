/*	guicode.c
 *	(c) Copyright 1991 by Ben Eng, All Rights Reserved
 *
 */

#ifndef NO_GAD_HEADERS
#include <exec/types.h>
#include <exec/lists.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <clib/gadtools_protos.h>
#endif

#define NG_Region0 (NGAry + 0)
#define NG_Region1 (NGAry + 1)
#define NG_Region2 (NGAry + 2)
#define NG_Region3 (NGAry + 3)
#define GAD_Region0 0
#define GAD_Region1 1
#define GAD_Region2 2
#define GAD_Region3 3

static struct NewGadget NGAry[4] = {
	{ 9,24,168,70,"Targets",(void *)0L,GAD_Region0,0x0004,(void *)0L },
	{ 12,110,81,14,"Add",(void *)0L,GAD_Region1,0x0010,(void *)0L },
	{ 94,110,80,14,"Delete",(void *)0L,GAD_Region2,0x0010,(void *)0L },
	{ 10,92,166,13,"ItemSelect",(void *)0L,GAD_Region3,0x0010,(void *)0L }
};

#define WIN_LEFT   427
#define WIN_TOP	0
#define WIN_WIDTH  200
#define WIN_HEIGHT 200

static struct Gadget *Gad_Region0;
static struct Gadget *Gad_Region1;
static struct Gadget *Gad_Region2;
static struct Gadget *Gad_Region3;
static struct MinList List_Region0 = { (struct MinNode *)&List_Region0.mlh_Tail, (void *)0L, (struct MinNode *)&List_Region0.mlh_Head };
static char *Buf_Region0;
static char *DefText_Region0 = (char *)0L;

static struct Gadget *GList;
static struct VisualInfo *VisInfo;
#define SelNo_Region0 0

struct Gadget *
InitGads( struct Screen *scr )
{
	struct Gadget *gad;
	short i;

	GList = (void *)0;
	if ((VisInfo = GetVisualInfo(scr, TAG_END)) == NULL )
		return((void *)0L);

	{
		struct NewGadget *ng;
		for( i = 0, ng = NGAry; i < sizeof(NGAry)/sizeof(NGAry[0]);
			i++, ng++ ) {
			ng->ng_VisualInfo = VisInfo;
			ng->ng_TextAttr = scr->Font;
		}
	}
	if( ( gad = CreateContext(&GList)) == NULL ) return((void *)0L);
	Gad_Region0 = gad = CreateGadget(LISTVIEW_KIND, gad, NGAry + 0,
		GTLV_Labels, &List_Region0,
		GTLV_Selected, SelNo_Region0,
		TAG_END
	);
	if( gad == NULL ) return( NULL );

	Gad_Region1 = gad = CreateGadget(BUTTON_KIND, gad, NGAry + 1, TAG_END );
	if( gad == NULL ) return( NULL );

	Gad_Region2 = gad = CreateGadget(BUTTON_KIND, gad, NGAry + 2, TAG_END );
	if( gad == NULL ) return( NULL );

	Gad_Region3 = gad = CreateGadget(STRING_KIND, gad, NGAry + 3,
		GTST_MaxChars, 108,
		GTST_String, DefText_Region0,
		TAG_END
	);
	if( gad == NULL ) return( NULL );
	Buf_Region0 = ((struct StringInfo *)gad->SpecialInfo)->Buffer;

	return( GList );
}

void
FreeGads(void)
{
	FreeGadgets( GList );
	if( VisInfo ) FreeVisualInfo( VisInfo );
	VisInfo = NULL;
	GList = NULL;
}
