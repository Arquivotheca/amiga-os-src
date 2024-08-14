/*******************

    SaveAs8SVX.c

      930528

********************/


#include <exec/types.h>
#include <dos/dos.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/intuitionbase.h>


#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include <string.h>	// for setmem()

#include "cdxl/pan.h"
#include "cdxl/cdxlob.h"
#include "cdxl/runcdxl.h"
#include "cdxl/blitdef.h"
#include "cdxl/xledit.h"
#include "cdxl/runcdxl_protos.h"


#include "cdxl/debugsoff.h"

// Uncomment to get debug output turned on
#define KPRINTF
//#include "cdxl/debugson.h"


#define MakeID(a,b,c,d) ((LONG)(a)<<24L | (LONG)(b)<<16L | (c)<<8 | (d))

#define ID_FORM MakeID('F','O','R','M')
#define ID_VHDR MakeID('V','H','D','R')
#define ID_BODY MakeID('B','O','D','Y')
#define ID_8SVX MakeID('8','S','V','X')
#define ID_CHAN MakeID('C','H','A','N')

#define	MAXLEN_FILENAME	30
#define	MAXLEN_PATHNAME	255

IMPORT UBYTE	FileNameBuf[256];

IMPORT struct IntuitionBase	* IntuitionBase;
IMPORT struct Library		* GadToolsBase;

struct Gadget * GadCreate( struct NewGadget * ng, struct VisualInfo * vi,
	struct Gadget * last, ULONG kind, ULONG tag, ... );

/* General IFF Structures */
typedef struct
{
	LONG ckID;
	LONG ckSize;
} Chunk_Header;

typedef struct
{
	LONG ckID;
	LONG ckSize;
	LONG formtype;
} FORM_Header;

typedef struct VOICE_HEADER
{
    ULONG oneshot;
	ULONG repeat;
	ULONG samples_per_cycle;
	UWORD samples_per_sec;
	UBYTE octives;
	UBYTE compression;
	LONG  volume;
} VOICE_HEADER;


STATIC	VOICE_HEADER	VHead;
STATIC	FORM_Header	FHead;
STATIC	Chunk_Header	CHead;


/***********************************************************************/
/* This returns a pointer to the filename at the end of the path or NULL
** if there is no filename at the end.
*/

UBYTE *
RootName( UBYTE * path )
{
    UBYTE	*root;

    if ( !path )
	return ( NULL );

    if ( root = strrchr( path, '/' ) ) {
	root++;
    } else if ( root = strrchr( path, ':' ) ) {
	root++;
    } else {
	root = path;
    }

    if ( !(*root) )
	root = NULL;

    return ( root );

}  // RootName()


/***********************************************************************/
/* This appends the filename extension 'ext' to 'name', only if
** it doesn't already exist.
** 'name' is expected to be FILENAME_LENGTH bytes long.
** Returns FALSE if not able to append within size constraints.
** Also returns FALSE if filename will be too long for AmigaDOS.
** 'ext' should not contain a period.
** 'ext' must NOT contain wildcard characters.
*/

BOOL
AppendExtension( UBYTE * name, UBYTE * ext )
{
    UBYTE	* cp;
    UBYTE	* rootname;

    if ( !name || !ext || !(*ext) )
	return ( TRUE );

    if ( *ext == '.' ) // skip over the period, if present
	ext++;

    if ( cp = strrchr( name, '.' ) )  {
	if ( !stricmp( cp+1, ext ) )  {
	    return ( TRUE );
	}
    }

    if ( rootname = RootName(name) )  {
	if ( strlen(rootname) + strlen(ext) + 1 > MAXLEN_FILENAME )
	    return ( FALSE );
    }

    if ( strlen(name) + strlen(ext) + 1 > MAXLEN_PATHNAME )
	return ( FALSE );

    strcat( name, ".");
    strcat( name, ext );
    return ( TRUE );

} // AppendExtension()


WritePad( BPTR fh )
{
    short nil = 0;
    return ( (Write(fh,&nil,1) == 1) ? 1 : 0 );

} // WritePad()


#define RANGE_GAD_LEFT	15
#define	RANGE_GAD_TOP	(Topaz80.ta_YSize + 5)
#define	RANGE_GAD_HT	14

#define START_FRAME_ID		50
#define START_FRAME_DISP_ID	51
#define END_FRAME_ID		52
#define END_FRAME_DISP_ID	53
#define RANGE_OK_ID		54
#define RANGE_CANCEL_ID		55


IMPORT struct TextAttr		  Topaz80;


RangeReq( XLEDIT * xledit, struct Window * win, ULONG * start, ULONG * end )
{
    struct Requester	  sleepReq;
    BOOL		  sleeping;
    struct NewGadget	  ng;
    struct IntuiText	  it;
    struct Window	* rangewin;
    struct IntuiMessage	* imsg;
    struct Gadget	* prev,* glist;
    struct VisualInfo	* vi;
    int			  i,wid,pos,ret = FALSE,panelY;
    BOOL		  quit = FALSE;

    kprintf("\nRangeReq() xledit= 0x%lx, win= 0x%lx, *start= 0x%lx, *end= 0x%lx\n",
	xledit,win,*start,*end);

    if ( !(vi = GetVisualInfo( win->WScreen, TAG_DONE ) ) ) {
	DisplayBeep( NULL );
	return( FALSE );
    }

    kprintf("RangeReq() 1\n");

    InitRequester( &sleepReq );
    sleeping = Request( &sleepReq, win );
    SetWindowPointer( win, WA_BusyPointer, TRUE, TAG_DONE );

    setmem( &ng, sizeof ( ng ), 0 );
    setmem( &it, sizeof ( it ), 0 );

    kprintf("RangeReq() 2\n");

    prev = CreateContext( &glist );

    kprintf("RangeReq() prev= 0x%lx\n",prev);

    ng.ng_LeftEdge = RANGE_GAD_LEFT;
    ng.ng_TopEdge = RANGE_GAD_TOP;
    ng.ng_Height = RANGE_GAD_HT;
    ng.ng_TextAttr = it.ITextFont = &Topaz80;
    ng.ng_VisualInfo = vi;
    ng.ng_Flags = NULL;

    ng.ng_GadgetText = it.IText = "_StartFrame";
    ng.ng_Width = IntuiTextLength( &it ) + 16;
    ng.ng_GadgetID = START_FRAME_ID;
    prev = GadCreate( &ng, vi, prev, BUTTON_KIND, 
	GT_Underscore,	'_', 
	TAG_DONE );


    ng.ng_GadgetText = it.IText = "";
    ng.ng_LeftEdge += ng.ng_Width;
    ng.ng_Width = 32 + 16;
    ng.ng_GadgetID = START_FRAME_DISP_ID;

    prev = GadCreate( &ng, vi, prev, NUMBER_KIND, 
	GTNM_Number,		*start,
	GTNM_Border,		TRUE,
	GTNM_Justification,	GTJ_CENTER,
	GTNM_Format,		"%ld",
	GTNM_FrontPen,		1,
	GTNM_BackPen,		0,
	TAG_DONE );


    ng.ng_LeftEdge += (ng.ng_Width + 8);
    ng.ng_GadgetText = it.IText = "_EndFrame";
    ng.ng_Width = IntuiTextLength( &it ) + 16;
    ng.ng_GadgetID = END_FRAME_ID;
    prev = GadCreate( &ng, vi, prev, BUTTON_KIND, 
	GT_Underscore,	'_', 
	TAG_DONE );


    ng.ng_GadgetText = it.IText = "";
    ng.ng_LeftEdge += ng.ng_Width;
    ng.ng_Width = 32 + 16;
    ng.ng_GadgetID = END_FRAME_DISP_ID;

    prev = GadCreate( &ng, vi, prev, NUMBER_KIND, 
	GTNM_Number,		*end,
	GTNM_Border,		TRUE,
	GTNM_Justification,	GTJ_CENTER,
	GTNM_Format,		"%ld",
	GTNM_FrontPen,		1,
	GTNM_BackPen,		0,
	TAG_DONE );

    wid = (ng.ng_LeftEdge + ng.ng_Width + RANGE_GAD_LEFT); // wind width

    ng.ng_LeftEdge = RANGE_GAD_LEFT;
    ng.ng_TopEdge += ng.ng_Height + 4;
    ng.ng_GadgetText = it.IText = "_OK";
    ng.ng_Width = IntuiTextLength( &it ) + 8;
    ng.ng_GadgetID = RANGE_OK_ID;
    prev = GadCreate( &ng, vi, prev, BUTTON_KIND, GT_Underscore, '_', TAG_DONE );

    ng.ng_GadgetText = it.IText = "_Cancel";
    ng.ng_Width = IntuiTextLength( &it ) + 8;
    ng.ng_LeftEdge = wid - ng.ng_Width - RANGE_GAD_LEFT;
    ng.ng_GadgetID = RANGE_CANCEL_ID;
    prev = GadCreate( &ng, vi, prev, BUTTON_KIND, GT_Underscore, '_', TAG_DONE );


    if ( prev && (rangewin = OpenWindowTags( NULL,
	WA_Width,		wid,
	WA_Height,		ng.ng_TopEdge + ng.ng_Height + 4,
	WA_Left,		win->MouseX - (wid >> 1),
	WA_Top,			20,
	WA_IDCMP,		IDCMP_GADGETUP|IDCMP_CLOSEWINDOW,
	WA_Activate,		TRUE,
	WA_CustomScreen,	win->WScreen,
	WA_Gadgets,		glist,
	WA_DragBar,		TRUE,
	WA_DepthGadget,		TRUE,
	WA_CloseGadget,		TRUE,
	WA_RMBTrap,		TRUE,
	WA_Title,		"Range Requester",
	TAG_DONE)) ) {

	GT_SetGadgetAttrs( GetGadget( rangewin, START_FRAME_DISP_ID ), rangewin, NULL, GTNM_Number, *start, TAG_END  );
	GT_SetGadgetAttrs( GetGadget( rangewin, END_FRAME_DISP_ID ), rangewin, NULL, GTNM_Number, *end, TAG_END  );

	panelY = SetHomePos( rangewin->TopEdge + rangewin->Height );

	HomePanel();


	while ( !quit ) {
	    Wait( 1 << rangewin->UserPort->mp_SigBit );

	    while ( imsg = GT_GetIMsg( rangewin->UserPort ) ) {

		switch ( imsg->Class )  {
		    case IDCMP_CLOSEWINDOW:
			quit = TRUE;
			ret = FALSE;
			break;


		    case IDCMP_GADGETUP:
			D(PRINTF("%ld\n",((struct Gadget *)imsg->IAddress)->GadgetID);)

			switch ( ((struct Gadget *)imsg->IAddress)->GadgetID ) {

			    case RANGE_OK_ID:
				quit = TRUE;
				ret = TRUE;
				break;

			    case RANGE_CANCEL_ID:
				quit = TRUE;
				ret = FALSE;
				break;

			    case START_FRAME_ID:
				if ( CalcReq( rangewin, vi, start ) )
				    GT_SetGadgetAttrs( GetGadget( rangewin, START_FRAME_DISP_ID ), rangewin, NULL, GTNM_Number, *start, TAG_END  );
				break;

			    case END_FRAME_ID:
				if ( CalcReq( rangewin, vi, end ) )
				    GT_SetGadgetAttrs( GetGadget( rangewin, END_FRAME_DISP_ID ), rangewin, NULL, GTNM_Number, *end, TAG_END  );
				break;
			}
			break;
		}

		GT_ReplyIMsg(imsg);

		if (quit)
		    break;
	    }
	}

	Forbid();
	StripIntuiMessages( rangewin->UserPort, rangewin );
	ModifyIDCMP( rangewin, 0L );
	Permit();
	CloseWindow( rangewin );

	SetHomePos( panelY );

    } else {
	DisplayBeep( win->WScreen );
    }

    if ( glist )
	FreeGadgets( glist );

    if ( sleeping )
	EndRequest( &sleepReq, win );

    ClearPointer( win );

    HomePanel();

    FreeVisualInfo( vi );

    return( ret );

} // RangeReq()


SaveAs8svx( XLEDIT * xledit, UBYTE * name, LONG numframes )
{
    BPTR	  fh = NULL;
    LONG	  chan = 4,ret = RC_OK;
    LONG	  size = xledit->cdxlob->AudioSize;
    LONG	  bodysize = size * numframes,seekpos1=-1,seekpos2=-1;
    UBYTE	  file[256];
    BOOL	  Pad = FALSE,FPad = FALSE;
    UBYTE	* body;

    strcpy( file, name );
//    RemoveExtension( file, "*?" );

    kprintf("SaveAs8SVX ENTERED with size= %ld, name= '%ls',numframes= %ld, bodysize= %ld\n",
	size,name,numframes,bodysize);

    if( !AppendExtension( file, "8svx" ) ) {
	ret = RC_FNAME_LONG;
	goto error;
    }

    if( !(fh = Open( file, MODE_NEWFILE )) ) {
	ret = RC_CANT_OPEN;
	printf("Could NOT Open '%ls'\n",file);
	goto error;
    }

		// Body   VoiceHeader	  Chan
    FHead.ckSize = bodysize + sizeof(VHead) + sizeof(LONG) + (3 * 8) + 4;
    if( FHead.ckSize & 1 ) {
	FHead.ckSize++;
	FPad = TRUE;
    }

    if ( (seekpos1 = Seek( fh, 0, OFFSET_CURRENT )) == -1L ) {
	ret = RC_WRITE_ERROR;
	goto error;
    }

    if( Write( fh, &FHead, sizeof(FHead) ) != sizeof(FHead) ) {
	ret = RC_WRITE_ERROR;
	goto error;
    }

    VHead.repeat = bodysize;

    CHead.ckID = ID_VHDR;
    CHead.ckSize = sizeof(VHead);
    if( CHead.ckSize & 1 ) {
	CHead.ckSize++;
	Pad = TRUE;
    }

    if( Write( fh, &CHead, sizeof(CHead) ) != sizeof(CHead) ) {
	ret = RC_WRITE_ERROR;
	goto error;
    }

    if( Write( fh, &VHead, sizeof(VHead) ) != sizeof(VHead) ) {
	ret = RC_WRITE_ERROR;
	goto error;
    }

    if( Pad ) {
	if (!WritePad(fh) ) {
	    ret = RC_WRITE_ERROR;
	    goto error;
	}
	Pad = FALSE;
    }

    CHead.ckID = ID_CHAN;
    CHead.ckSize = sizeof(LONG);
    if( CHead.ckSize & 1 ) {
	CHead.ckSize++;
	Pad = TRUE;
    }

    if( Write( fh, &CHead, sizeof(CHead) ) != sizeof(CHead) ) {
	ret = RC_WRITE_ERROR;
	goto error;
    }

    if( Write( fh, &chan, sizeof(LONG) ) != sizeof(LONG) ) {
	ret = RC_WRITE_ERROR;
	goto error;
    }

    if( Pad ) {
	if (!WritePad(fh) ) {
	    ret = RC_WRITE_ERROR;
	    goto error;
	}
	Pad = FALSE;
    }

    CHead.ckID = ID_BODY;
    CHead.ckSize = bodysize;
    if( CHead.ckSize & 1 ) {
	CHead.ckSize++;
	Pad = TRUE;
    }

    if ( (seekpos2 = Seek( fh, 0, OFFSET_CURRENT )) == -1L ) {
	ret = RC_WRITE_ERROR;
	goto error;
    }

    if( Write( fh, &CHead, sizeof(CHead) ) != sizeof(CHead) ) {
	ret = RC_WRITE_ERROR;
	goto error;
    }

    bodysize = 0;

    do {
	body = xledit->cdxlob->audio[xledit->cdxlob->curVideo];

	if( Write( fh, body, size ) != size ) {
	    ret = RC_WRITE_ERROR;
	    goto error;
	}

	bodysize += size;

	if ( --numframes && (ret = StepFrame( xledit, TRUE )) ) {
	    kprintf("BREAKING numframes= %ld, FrameNum= %ld , NumFrames= %ld\n",
		numframes,xledit->cdxlob->FrameNum,xledit->cdxlob->NumFrames);
	    break;
	}
	UpDateFrame( xledit->cdxlob->FrameNum );

    } while ( numframes && ( xledit->cdxlob->FrameNum < (xledit->cdxlob->NumFrames-1) ) );

    if ( (seekpos2 = Seek( fh, seekpos2, OFFSET_BEGINNING)) != -1L ) {

	CHead.ckID = ID_BODY;
	CHead.ckSize = bodysize;
	if( CHead.ckSize & 1 ) {
	    CHead.ckSize++;
	    Pad = TRUE;
	} else {
	    Pad = FALSE;
	}

	kprintf("ckSize= %ld, bodysize= %ld, pad= %ld\n",
	    CHead.ckSize,bodysize,Pad);


	// Rewrite CHead accounting for actual bodysize written
	if( Write( fh, &CHead, sizeof(CHead) ) != sizeof(CHead) ) {
	    ret = RC_WRITE_ERROR;
	    goto error;
	}


	// Rewrite FHead accounting for actual bodysize written
	if ( Seek( fh, seekpos1, OFFSET_BEGINNING) != -1L ) {
			// Body   VoiceHeader	  Chan
	    FHead.ckSize = bodysize + sizeof(VHead) + sizeof(LONG) + (3 * 8) + 4;
	    if( FHead.ckSize & 1 ) {
		FHead.ckSize++;
		FPad = TRUE;
	    } else {
		FPad = FALSE;
	    }

	    if( Write( fh, &FHead, sizeof(FHead) ) != sizeof(FHead) ) {
		ret = RC_WRITE_ERROR;
		goto error;
	    }
	} else {
	    seekpos2 = -1;
	}
    }

    if ( (seekpos2 !=-1) && (Seek( fh, seekpos2, OFFSET_BEGINNING)) != -1L ) {
	if( Pad ) {
	    if (!WritePad(fh) ) {
		ret = RC_WRITE_ERROR;
		goto error;
	    }
	    Pad = FALSE;
	}

	if( FPad ) {
	    if (!WritePad(fh) ) {
		ret = RC_WRITE_ERROR;
		goto error;
	    }
	    Pad = FALSE;
	}

    } else {
	ret = RC_WRITE_ERROR;
    }

    Close( fh );

    printf("%s  to  %s\n",name,file);

    return( ret );

error:

    if( fh ) {
	Close( fh );
    }

    return( ret );

} /* SaveAs8svx() */


SaveXLAs8SVX( XLEDIT * xledit, struct Window * win )
{
    int			  ret = RC_OK;
    ULONG		  start,end,numframes;
    struct Requester	  sleepReq;
    BOOL		  sleeping;

    if ( !(xledit && xledit->cdxlob && xledit->cdxlob->AudioSize) ) {
	DisplayBeep( NULL );
	return( RC_FAILED );
    }

    kprintf("SaveXLAs8SVX() ENTERED\n");

    start = end = xledit->cdxlob->FrameNum;

    if ( !RangeReq( xledit, win, &start, &end ) ) {
	return( RC_OK );
    }

    if ( FileReq( win ) ) {

	InitRequester( &sleepReq );
	sleeping = Request( &sleepReq, win );
	SetWindowPointer( win, WA_BusyPointer, TRUE, TAG_DONE );

	start = (start > xledit->cdxlob->NumFrames) ? xledit->cdxlob->NumFrames : start;

	numframes = (end - start) + 1;
	numframes = (numframes < 1) ? 1 : numframes;
	numframes = (numframes > xledit->cdxlob->NumFrames) ? xledit->cdxlob->NumFrames : numframes;

	setmem( &VHead, sizeof(VHead), 0 );
//	VHead.samples_per_sec = 14435;
	VHead.samples_per_sec = INTDIV( (( xledit->cdxlob->ReadXLSpeed * DEFAULT_SECTOR_SIZE) * xledit->cdxlob->AudioSize ), xledit->cdxlob->FrameSize );

	VHead.octives = 1;
	VHead.volume = 65536;

	setmem( &FHead, sizeof(FHead), 0 );
	setmem( &CHead, sizeof(CHead), 0 );

	FHead.ckID = ID_FORM;
	FHead.formtype = ID_8SVX;

	kprintf("SaveXLAs8SVX() 1\n");

	if ( start != xledit->cdxlob->FrameNum ) {
	    GoToFrame( xledit, start );
	    UpDateFrame( xledit->cdxlob->FrameNum );
	}

	ret = SaveAs8svx( xledit, FileNameBuf, numframes );


	if ( sleeping )
	    EndRequest( &sleepReq, win );

	ClearPointer( win );
    }

    kprintf("SaveXLAs8SVX() END ret= %ld\n",ret);

    return( ret );

} // SaveXLAs8SVX()

