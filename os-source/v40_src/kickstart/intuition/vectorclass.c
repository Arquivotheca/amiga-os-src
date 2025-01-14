/*
 *	$Id: vectorclass.c,v 38.17 93/05/05 10:44:24 peter Exp $
 *
 *	scaleable images.
 *
 *	(C) Copyright 1989, 1990 Commodore-Amiga, Inc.
 *	    All Rights Reserved Worldwide
 */

#include "intuall.h"
#include <exec/memory.h>
#include "classusr.h"
#include "classes.h"
#include "imageclass.h"
#include "newlook.h"
#include <graphics/gfxmacros.h>
#include "vectorclass.h"

/* Scales a coordinate in source size to dest size.
 * This routine was smaller as a macro, until we added
 * the rounding.
 */

LONG Scale(sourcesize,destsize,coord)
LONG sourcesize;
LONG destsize;
LONG coord;
{
    return( ( ((sourcesize-1) >> 1) + ( (destsize-1) * coord ) ) / (sourcesize-1) );
}


#define OUT printf

#define IM(o)	((struct Image *) (o))

#define	D(x)		;
#define	DS(x)		;
#define	DS5(x)		;
#define	DM(x)		;
#define DO(x)		;
#define	DV(x)		;
#define DPOLY(x)	;

/* No sysiclass image currently uses more than 10 vectors, so double
 * that is a nice safe number.
 * The size of the area-buffer is 5 bytes per vector, but in WORDs.
 */

#define NUM_AREAVECTORS	20
#define AREA_WORDSIZE	( ( ( NUM_AREAVECTORS ) * 5 ) / 2 )

/* sysiclass per-object instance data */
struct SysIData {
    UWORD *si_ImageData[VS_NUMSTATES];/* ImageData for each visual state */
    UWORD si_Depth;		/* Depth of image */
    UBYTE si_States;		/* Visual states available */
};

/* class work area */
struct SysIClassData {
    struct SignalSemaphore scd_Sem;	/* Semaphore for lockout */

    UWORD scd_Width;		/* Width of work area */
    UWORD scd_Height;		/* Height of work area */
    UWORD scd_Depth;		/* Depth of work area */

    struct BitMap scd_BM;	/* Image's BitMap */
    struct RastPort scd_RP;	/* Image's RastPort */

    struct TmpRas scd_TmpRas;	/* Flood Fill TmpRas */
    UBYTE *scd_TmpRasPlane;	/* Flood Fill workspace */

    struct AreaInfo scd_Area;	/* Area Fill AreaInfo */
    WORD scd_Array[ AREA_WORDSIZE ];	/* Area Fill workspace */
};

extern struct Vector vdepth[];
extern struct Vector vsdepth[];
extern struct Vector vzoom[];
extern struct Vector vclose[];
extern struct Vector vsize[];
extern struct Vector upar[];
extern struct Vector dnar[];
extern struct Vector lfar[];
extern struct Vector rtar[];
extern struct Vector ckmk[];
extern struct Vector mutx[];
extern struct Vector mchk[];
extern struct Vector amky[];


/* DEPTH gadget */
#define VDEPTH_COUNT	9
#define VDEPTH_DW	24
#define VDEPTH_DH	11

/* Screen DEPTH gadget */

#define VSDEPTH_COUNT	9

#define VSDEPTH_DW	23
#define VSDEPTH_DH	11

/* ZOOM gadget */

#define VZOOM_COUNT	6
#define VZOOM_DW	24
#define VZOOM_DH	11

/* CLOSE gadget */
#define VCLOSE_COUNT	5
#define VCLOSE_DW	20
#define VCLOSE_DH	11

/* SIZING gadget 05/31/90 */

#define VSIZE_COUNT	4
#define VSIZE_DW	18
#define VSIZE_DH	10

/* UP ARROW gadget 05/31/90 */

#define UPAR_COUNT	2
#define UPAR_DW		18
#define UPAR_DH		11

/* DOWN ARROW gadget 05/31/90 */
/* share the up arrows' background rectangle */
/* reversed the Y of the UP arrow */

#define DNAR_COUNT	2
#define DNAR_DW		18
#define DNAR_DH		11

/* LEFT ARROW gadget 05/31/90 */

#define LFAR_COUNT	2
#define LFAR_DW		16
#define LFAR_DH		10

/* RIGHT ARROW gadget 05/31/90 */
/* share the LEFT arrows' background rectangle */
/* reversed the X of the LEFT arrow */

#define RTAR_COUNT	2
#define RTAR_DW		16
#define RTAR_DH		10


/* CHECKMARK 05/31/90 */

#define CKMK_COUNT	1
#define CKMK_DW		26
#define CKMK_DH		11

/* MUTUAL EXCLUDE 05/31/90 */

#define MUTX_COUNT	9
#define MUTX_DW		17
#define MUTX_DH		9

/* MENU-CHECKMARK 1-May-91 */

#define MCHK_COUNT	1
#define MCHK_DW		15
#define MCHK_DH		8

/* AMIGAKEY 1-May-91 */

#define AMKY_COUNT	3
#define AMKY_DW		45
#define AMKY_DH		15

struct VectorInfo VectorImages[] =
{
    /* NB: Depth width (24 or 18) is also coded into ezreq.c in the call
     * to reqTitleLength().
     */
    {	/* DEPTHIMAGE */
	{23+1,17+1,23+1,},	/*  +1 for the part of the groove in the drag */
	{11,11,11},
	vdepth,
	VDEPTH_DW, VDEPTH_DH,
	VDEPTH_COUNT,
	VS_WBORDERIMG,
	VIB_HRIGHT,
	VIF_SPECIFYHEIGHT,
    },
    {	/* ZOOMIMAGE */
	{23+1,17+1,23+1,},
	{11,11,11,},
	vzoom,
	VZOOM_DW, VZOOM_DH,
	VZOOM_COUNT,
	VS_WBORDERIMG,
	VIB_HRIGHT,
	VIF_SPECIFYHEIGHT,
    },
    {	/* SIZEIMAGE */
	{18,13,18,},
	{10,11,10,},
	vsize,
	VSIZE_DW, VSIZE_DH,
	VSIZE_COUNT,
	VS_NORMAL | VS_INANORMAL,
	VIB_BRCORN,
	0,
    },
    {	/* CLOSEIMAGE */
	{19+1,14+1,19+1,},
	{11,11,11,},
	vclose,
	VCLOSE_DW, VCLOSE_DH,
	VCLOSE_COUNT,
	VS_WBORDERIMG,
	VIB_HLEFT,
	VIF_SPECIFYHEIGHT,
    },
    {	/* SDEPTHIMAGE */
	{23,17,23,},
	{11,11,11,},
	vsdepth,
	VSDEPTH_DW, VSDEPTH_DH,
	VSDEPTH_COUNT,
	VS_NORMAL | VS_SELECTED,
	VIB_3D,
	VIF_SPECIFYHEIGHT,
    },
    {	/* LEFTIMAGE */
	{16,16,23,},
	{10,11,22,},
	lfar,
	LFAR_DW, LFAR_DH,
	LFAR_COUNT,
	VS_WBORDERIMG,
	VIB_HORIZ,
	0,
    },
    {	/* UPIMAGE */
	{18,13,23,},
	{11,11,22,},
	upar,
	UPAR_DW, UPAR_DH,
	UPAR_COUNT,
	VS_WBORDERIMG,
	VIB_VERT,
	0,
    },
    {	/* RIGHTIMAGE */
	{16,16,23,},
	{10,11,22,},
	rtar,
	RTAR_DW, RTAR_DH,
	RTAR_COUNT,
	VS_WBORDERIMG,
	VIB_HORIZ,
	0,
    },
    {	/* DOWNIMAGE */
	{18,13,23,},
	{11,11,22,},
	dnar,
	DNAR_DW, DNAR_DH,
	DNAR_COUNT,
	VS_WBORDERIMG,
	VIB_VERT,
	0,
    },
    {	/* CHECKIMAGE */
	{26,26,26,},
	{11,11,11,},
	ckmk,
	CKMK_DW, CKMK_DH,
	CKMK_COUNT,
	VS_NORMAL | VS_SELECTED,
	VIB_THICK3D,
	VIF_SPECIFYWIDTH | VIF_SPECIFYHEIGHT,
    },
    {	/* MXIMAGE */
	{17,17,17,},
	{9,9,9,},
	mutx,
	MUTX_DW, MUTX_DH,
	MUTX_COUNT,
	VS_NORMAL |VS_SELECTED,
	NULL,
	VIF_SPECIFYWIDTH | VIF_SPECIFYHEIGHT,
    },
    {	/* MENUCHECK */
	{15,9,15,},
	{8,8,8,},
	mchk,
	MCHK_DW, MCHK_DH,
	MCHK_COUNT,
	VS_NORMAL,
	VIB_INMENU,
	VIF_REFFONT | VIF_SPECIFYWIDTH | VIF_SPECIFYHEIGHT | 1 /* VIF_WMULTIPLY */,
    },
    {	/* AMIGAKEY */
	{23,14,23,},
	{8,8,8,},
	amky,
	AMKY_DW, AMKY_DH,
	AMKY_COUNT,
	VS_NORMAL,
	VIB_INMENU,
	VIF_REFFONT | VIF_SPECIFYWIDTH | VIF_SPECIFYHEIGHT | 2 /* VIF_WMULTIPLY */,
    },
};

VOID Draw1P3DBorder();
void myRectangle();
VOID drawVectorImage();
BOOL setState();
BOOL allocRenderStuff();
VOID freeRenderStuff();
BOOL allocImageMem();
VOID freeImageMem();

#define myEnd	stubReturn
int	stubReturn();
int	Move();
int	Draw();
int	AreaMove();
int	AreaDraw();
int	RectFill();
int	AreaEnd();

/* add the Vector image class to the system */
Class
*initSysIClass()
{
    ULONG dispatchSysI();
    Class *makePublicClass();
    Class *cl = NULL;
    struct SysIClassData *scd;
    extern UBYTE	*SysIClassName;
    extern UBYTE	*ImageClassName;

    if ( scd = (struct SysIClassData *) 	
      AllocMem( sizeof( struct SysIClassData ), MEMF_CLEAR | MEMF_PUBLIC ) )
    {
    	if ( cl = makePublicClass( SysIClassName, ImageClassName,
			    sizeof(struct SysIData), dispatchSysI ) )
	{
	    /* initialize the semaphore */
	    InitSemaphore( &scd->scd_Sem );

	    /* set up class data */
	    cl->cl_UserData = (ULONG) scd;

	    DO( OUT("added %s\n", SysIClassName));

	    return( cl );
	}
	else
	{
	    /* free the unused work area */
	    FreeMem( scd, sizeof *scd );
	}
    }
    return( cl );
}


/* Class dispatcher for sysimageclass.
 *
 * OM_NEW causes the rendering of the vector description into a bitmap
 *     for each supported state.
 * OM_DRAW causes the appropriate bitmap to be selected as the ImageData,
 *     and then imageclass draws it.
 * OM_DISPOSE throws everything away.
 *
 * No other methods are in use.
 *
 */

ULONG
dispatchSysI( cl, o, msg )
Class	*cl;
Object	*o;
Msg	msg;
{
    struct TagItem *tags;
    ULONG size;
    LONG width, height;
    ULONG which;
    struct VectorInfo *vi;
    Object *newobj = NULL;
    struct DrawInfo *dri;

    BOOL createStateImagery();

    switch ( msg->MethodID )
    {
    case OM_NEW:
	DO ( OUT ("\nvectorclass: OM_NEW\n"));

	tags = ((struct opSet *) msg)->ops_AttrList;

	/* now set the size and whatever else I want	 */
	which = GetUserTagData( SYSIA_Which, ~0, tags );

	dri = (struct DrawInfo *)GetUserTagData0( SYSIA_DrawInfo, tags );
	size = SYSISIZE_MEDRES;
	if ( ( dri ) && ( dri->dri_Resolution.X > MOUSESCALEX ) )
	{
	    size = SYSISIZE_LOWRES;
	}
	size = GetUserTagData( SYSIA_Size, size, tags );

	D( kprintf( "dXSysI: which %ld, size %ld\n", which, size ) );

	/* The SYSIA_Which space is sparse, and must be mapped into an index
	 * into VectorImages.
	 * WHICH       VECTORIMAGES INDEX
	 * 0 to 3   => 0 to 3
	 * 4        => missing
	 * 5        => 4
	 * 6 to 9   => missing
	 * 10 to 17 => 5 to 12
	 */
	if ( (which > 17) || (which == 4) || ( (which > 5) && (which < 10) ) )
	{
	    D( kprintf ("dXSysI: OtherImage\n") );
	    return (NULL);
	}
	else if (which == 5)
	{
	    which--;
	}
	else if (which > 5)
	{
	    which -= 5;
	}

	vi = &VectorImages[ which ];

	/* Defaults come from the table. */
	width = vi->vi_Width[ size ];
	height = vi->vi_Height[ size ];

	if ( TESTFLAG( vi->vi_Flags, VIF_REFFONT ) )
	{
	    struct TextFont *font;
	    LONG widthmul;

	    /* Size is relative to the specified font, so derive it from that.
	     * If SYSIA_ReferenceFont is NULL or missing, then use the DrawInfo
	     * font, if available.
	     */
	    font = (struct TextFont *)GetUserTagData0( SYSIA_ReferenceFont, tags );
	    if ( ( !font ) && ( dri ) )
	    {
		font = dri->dri_Font;
	    }

	    if ( font )
	    {
		struct RastPort tempRP;

		InitRastPort( &tempRP );
		SetFont( &tempRP, font );
		widthmul = vi->vi_Flags & VIF_WMULTIPLY;  /* 1 or 2 * em-space */
		/* These magic scaling values are designed to provide
		 * reasonable appearance combined with roughly compatible
		 * dimensions to V37 and earlier.
		 */
		width = widthmul * TextLength( &tempRP, "m", 1 ) + 7;

		height = imax(8, font->tf_Baseline);
	    }
	}
	if ( TESTFLAG( vi->vi_Flags, VIF_SPECIFYWIDTH ) )
	{
	    /* Allowed to override width */
	    width = GetUserTagData(IA_WIDTH, width, tags);
	}
	if ( TESTFLAG( vi->vi_Flags, VIF_SPECIFYHEIGHT ) )
	{
	    /* Allowed to override height */
	    height = GetUserTagData(IA_HEIGHT, height, tags);
	}

	/* send message to superclass asking for new object */
	if (newobj = (Object *) SSM (cl, o, msg))
	{
	    /* zoom & depth have to be shifted by 1 pixel
	     * for the groove in the drag bar
	     */
	    if ( which < 2 )
	    {
		IM(newobj)->LeftEdge = -1;
	    }
	    /* OK, let's attempt to create the imagery for each
	     * supported state of this image.
	     */
	    if ( !createStateImagery(
		    IM(newobj),			/* Object is an image */
		    INST_DATA(cl, newobj),	/* Its instance data */
		    (struct SysIClassData *)cl->cl_UserData,	/* Class data */
		    vi, width, height, dri ) )
	    {
		/* createStateImagery() cleans up after itself, but now
		 * we must dispose what the class mechanism has
		 * done so far, and fail
		 */
		CoerceMessage(cl, newobj, OM_DISPOSE);
		return( NULL );
	    }
	}

	/* return the new object (an Image) */
	return ((ULONG) newobj);

    case IM_DRAW:
	DO ( OUT ("vectorclass: IM_DRAW\n"));
	return ( (ULONG) drawSysI( cl, o, msg ) );

    case OM_DISPOSE:
	DO ( OUT ("vectorclass: OM_DISPOSE\n"));

	/* get a pointer to the instance data */

	/* free the image data */
	freeImageMem( (struct SysIData *)INST_DATA(cl,o) );

#if 0
	/* LOOK:  We're falling through to the default case! */
	/* tell the super class to dispose of the remainder */
	return( (ULONG) SSM (cl, o, msg) );
#endif

   /* Be careful: OM_DISPOSE relies on falling through to here */
    default:
	/* pass anything else to our superclass */
	DO ( OUT ("vectorclass: MethodID=%ld\n", (LONG) msg->MethodID));
	return( (ULONG) SSM (cl, o, msg) );
    }
}

/* drawSysI() is the actual render routine that puts the system image up
 * in the requested state.  The actual bitmaps for each supported state
 * were previously allocated.  All this routine has to do is to pick the
 * correct state, select the corresponding bitmap, and let our superclass
 * (imageclass) just draw the sucker.
 */

drawSysI( cl, o, msg )
Class		*cl;
Object		*o;
struct impDraw	*msg;
{
    struct SysIData *si = INST_DATA(cl,o);
    ULONG state, drawstate = msg->imp_State;
    BOOL super = FALSE;

    /* The supported IDS states are IDS_NORMAL, IDS_SELECTED,
     * and IDS_INACTIVENORMAL.  Anything else, we render as
     * IDS_NORMAL and let the superclass handle the state...
     */

    if ( ( drawstate == IDS_NORMAL ) || ( drawstate == IDS_SELECTED ) )
    {
	/* For NORMAL and SELECTED, the IDS values equate to the VSB values */
	state = drawstate;
    }
    else if ( drawstate == IDS_INACTIVENORMAL )
    {
	state = VSB_INANORMAL;
    }
    else
    {
	state = VSB_NORMAL;
	/* Let superclass try to handle the state */
	super = TRUE;
    }

    /* Set up the image for the requested visual state.  If that
     * state is not supported, we get VSB_NORMAL and let the
     * superclass handle the state...
     */
    super |= setState( IM(o), si, state, NULL );

    /*
     * if it is a supported state, then tell the superclass to draw it
     * normally (i.e. force IDS_NORMAL).  Otherwise, let the superclass
     * attempt to handle the specified state.
     */
    if (!super)
	msg->imp_State = IDS_NORMAL;

    DO( OUT ("drawSysI: state %ld (%ld)\n", msg->imp_State, drawstate));

    /* super away */
    SSM( cl, o, msg );

    /* restore stuff */
    msg->imp_State = drawstate;

    return( 1 );
}


/* Set the image data to the correct visual state.
 * Returns TRUE if the requested state is not supported, else FALSE.
 * NB: if scd is non-NULL, the scd_Sem Semaphore had better be held!
 */
BOOL
setState( im, si, state, scd )
struct Image *im;
struct SysIData *si;
LONG state;
struct SysIClassData *scd;
{
    /* If the state is not supported, we'll set this to TRUE,
     * indicating we expect the superclass to handle this state
     */
    BOOL super = FALSE;

    /* make sure that there is data for the selected state */
    if ( !( si->si_ImageData[state] ) )
    {
	super = TRUE;
	state = VSB_NORMAL;
    }

    /* point the imagedata field to the correct data block */
    im->ImageData = si->si_ImageData[ state ];
    DS( printf("setState, state %lx data %lx\n", state,im->ImageData ) );

    /* map data to bitmap planes? */
    if ( scd )
    {
	LONG image_data;
	WORD i, p;

	image_data = (LONG) im->ImageData;	/* pointer to the image data */
	p = RASSIZE( im->Width, im->Height );	/* plane size */

	/* setup the bitmap planes to point to the image data */
	for ( i = 0; i < si->si_Depth; i++ )
	{
	    scd->scd_BM.Planes[i] = (PLANEPTR) image_data;
	    image_data += p;
	}
    }

    return( super );
}


/* createStateImagery() get memory for each supported state,
 * and draws the appropriate image into that memory.
 */

BOOL createStateImagery( im, si, scd, vi, width, height, dri )
struct Image *im;
struct SysIData *si;
struct SysIClassData *scd;
struct VectorInfo *vi;
LONG width;
LONG height;
struct DrawInfo *dri;
{
    BOOL success = TRUE;

    si->si_States = vi->vi_States;
    si->si_Depth = dri->dri_Depth;
    im->Width = width;
    im->Height = height;
    /* set the unused image fields */
    im->PlanePick = (1 << si->si_Depth) - 1;
    im->PlaneOnOff = 0;
    im->NextImage = NULL;

    /* allocate the image data */
    success = allocImageMem( im, si );

    if ( success )
    {
	/* lock out all else from the SysIClass work area */
	ObtainSemaphore( &scd->scd_Sem );

	/* attempt to allocate the stuff we need for rendering */
	if ( success = allocRenderStuff( im, si, scd ) )
	{
	    LONG i;
	    ULONG state, states;

	    states = si->si_States;

	    /* setup each visual state */
	    for (i = 0; i < VS_NUMSTATES; i++)
	    {
		DS( printf("RID: state %lx\n", i ) );

		/* get the current flag */
		state = (1 << i);

		/* see if the state is supported */
		if ((states & state) && si->si_ImageData[i])
		{
		    DS( printf("states'n'state, idata at %lx\n",
			    si->si_ImageData[i] ));

		    /* set the current visual state */
		    setState( im, si, i, scd );

		    /* Let's actually turn the vector description
		     * for this state into a bitmap
		     */
		    drawVectorImage( &scd->scd_RP, vi,
			im->Width, im->Height, i, dri );
		}
		DS(else
		{
		    printf("RID:  Unsupported state %ld\n",i);
		})
	    }
	    /* free the work area */
	    freeRenderStuff( scd );
	}
	/* release the lock on the class data */
	ReleaseSemaphore( &scd->scd_Sem );
    }

    if ( !success )
    {
	freeImageMem( si );
    }

    return( success );
}

/* allocRenderStuff() sets up the stuff we need in order to render.
 * This consists initializing a BitMap, RastPort, and TmpRas, and
 * allocating a TmpRasPlane.  All this stuff (except the TmpRas plane)
 * is kept in the per-class data (SysIClassData) and arbitrated by a
 * semaphore.  Note that the BitMap planes are per image, thus allocated
 * by allocImageMem() (the pointers are established by setState().)
 */

BOOL
allocRenderStuff( im, si, scd )
struct Image *im;
struct SysIData *si;
struct SysIClassData *scd;
{
    struct BitMap *bm = &(scd->scd_BM);
    WORD width, height, raster_size;

    /* preset the variables */
    width = im->Width;
    height = im->Height;

    /* Initialize the bitmap */
    InitBitMap( bm, si->si_Depth, width, height );

    /* initialize the work rastport */
    InitRastPort( &scd->scd_RP );

    /* Link them up */
    scd->scd_RP.BitMap = &(scd->scd_BM);

    /* initialize the area fill workspace */
    clearWords( scd->scd_Array, AREA_WORDSIZE );
    InitArea( &scd->scd_Area, scd->scd_Array, NUM_AREAVECTORS );
    scd->scd_RP.AreaInfo = &scd->scd_Area;

    /* NB: Nothing above this point can fail! */

    /* obtain area fill resources */

    raster_size = RASSIZE( width, height );

    /* The free code structure basically depends on the fact that
     * TmpRasPlane is the only allocation here
     */
    if (scd->scd_TmpRasPlane = (UBYTE *) AllocVec( raster_size, MEMF_CHIP|MEMF_CLEAR  ) )
    {
	/* Initialize Flood Fill Area */
	InitTmpRas( &scd->scd_TmpRas, scd->scd_TmpRasPlane, raster_size );
	scd->scd_RP.TmpRas = &scd->scd_TmpRas;
	return( TRUE );
    }
    return( FALSE );
}


/* Free the stuff in the SysIClass data area used for rendering,
 */
VOID
freeRenderStuff( scd )
struct SysIClassData *scd;
{
    /* Peter 28-Jan-92: we really need to WaitBlit() here! */
    WaitBlit();
    FreeVec( scd->scd_TmpRasPlane );
    scd->scd_TmpRasPlane = NULL;

}


/* Allocate memory for the ImageData of each supported state */
BOOL
allocImageMem( im, si )
struct Image *im;
struct SysIData *si;
{
    WORD state;
    BOOL retval = TRUE;

    /* allocate imagery for each supported state */

    for ( state = 0; state < VS_NUMSTATES; state++ )
    {
	/* see if they asked for this state */
	if ( si->si_States & ( 1 << state ) )
	{
	    /* allocate memory for visual state. */
	    if (! ( si->si_ImageData[ state ] = (UWORD *) AllocVec(
		si->si_Depth * RASSIZE( im->Width, im->Height ),
		MEMF_CHIP | MEMF_CLEAR ) ) )
	    {
		retval = FALSE;
	    }
	}
    }
    return (retval);
}


/* Free ImageData for each state */
VOID
freeImageMem( si )
struct SysIData	*si;
{
    WORD state;

    /* Peter 28-Jan-92: we really need to WaitBlit() here! */
    WaitBlit();
    /* step thru each visual state */
    for ( state = 0; state < VS_NUMSTATES; state++ )
    {
	/* free any visual state that exists */
	FreeVec( si->si_ImageData[ state ] );
	si->si_ImageData[ state ] = NULL;
    }

}


/* drawVectorImage() draws an image based on a VectorInfo description.
 * The image is drawn into the specified rastport, in the specified
 * width and height, using the supplied DrawInfo.
 */

VOID
drawVectorImage( rport, vi, Width, Height, state, dri )
struct RastPort	*rport;
struct VectorInfo *vi;
LONG		Width;
LONG		Height;
ULONG		state;
struct DrawInfo *dri;
{
    WORD x1, y1, x2, y2;	/* current points */
    UBYTE *data;	/* vector point data */

    LONG direction;		/* direction of 3D box */
    ULONG border = 0L;		/* tack on the system 3D? */
    ULONG state_flag;		/* flags for Visual State */
    UWORD *penspec;		/* DrawInfo pens */
    UWORD penindex;		/* remapped pens */
    BOOL mono = TRUE;		/* doing a monochrome screen? */
    UBYTE vectorcount;
    UBYTE designWidth;
    UBYTE designHeight;
    UBYTE opcount;		/* operation counter */
    struct Vector *v;


    extern UBYTE Vbase;		/* Base for vector-data */


    void (*movef)();
    void (*drawf)();
    void (*rectf)();
    void (*aendf)();

    DV(printf("DVA:rp at $%lx, W/H: <%ld,%ld>, state $%lx\n",
	rport, Width, Height, state));

    /* determine direction of 3D border based on visual state */
    direction = 0;
    if (state != VSB_SELECTED)
    {
	direction++;
    }
    
    vectorcount = vi->vi_VCount;
    designWidth = vi->vi_DesignWidth;
    designHeight = vi->vi_DesignHeight;
    v = vi->vi_VList;

    /* get information for determining MONO handling */
    state_flag = (ULONG)(1 << state);
    DS5( if ( state == 5 ) printf("DV state 5, flag %lx\n", state_flag ) );

    {
	if ( dri )
	{
	    penspec = dri->dri_Pens;
	    /*  Use monochrome mode in 1 bitplane or when explicitly requested
		(for non-NewLook custom screens) */
	    if ( ( dri->dri_Flags & DRIF_NEWLOOK ) && ( dri->dri_Depth > 1 ) )
	    {
		mono = FALSE;
	    }
	}
    }

    DV( printf("screen mono:  %lx\n", mono ));
    border = vi->vi_Border;

    /* Fill background to the correct color, which defaults to BACKGROUNDPEN: */
    penindex = BACKGROUNDPEN;

    /* If it is in the window border, then it may need to be in FILLPEN: */
    if ( ( border ) & ( VIB_HLEFT | VIB_HRIGHT | VIB_VERT | VIB_HORIZ | VIB_BRCORN ) )
    {
	/* Inactive-normal and monochrome-selected gadgets stay in BACKGROUNDPEN,
	 * all others need to be in FILLPEN
	 */
	if ( !( (state == VSB_INANORMAL) || ( ( mono ) && ( state == VSB_SELECTED ) ) ) )
	{
	    penindex = FILLPEN;
	}
    }
    else if ( border & VIB_INMENU )
    {
	penindex = BARBLOCKPEN;
    }

    /* blank out the area */
    SetRast( rport, penspec[ penindex ] );

    DV ( OUT ("List 0x%lx  VState 0x%lx (%ld)", v, state_flag, (LONG)state) );

    /* loop thru the list of vectors */
    while (vectorcount--)
    {
	DV( printf("vector %lx\n", v ) );
	DS5( if ( state == 5 ) printf("DV5, v %lx\n", v ) );

	/* check if vector data needed for requested Visual State */
	if (
	       (v->v_States & state_flag) &&
	       ((mono && (v->v_Flags & VF_MONO)) ||
		(!mono && (v->v_Flags & VF_COLOR)))
	  )
	{
	    DS5( if(state==5)printf("vector data needed\n"));
	    /* compute the scaling factor */

	    /* set the RastPort information */
	    SetABPenDrMd( rport, penspec[ v->v_Flags & VF_PENMASK ], 0, JAM1 );

	    DV( printf("I'm happy,  looking at flags %lx\n", v->v_Flags  ));

	    if (v->v_Flags & VF_FILLED)
	    {
		/* fill oriented drawing routines */
		movef = AreaMove;
		drawf = AreaDraw;
		rectf = RectFill;
		aendf = AreaEnd;
	    }
	    else
	    {
		/* line oriented drawing routines */
		movef = Move;
		drawf = Draw;
		rectf = myRectangle;
		aendf = myEnd;
	    }

	    /* Calculate vector-data pointer from base/offset */
	    data = (&Vbase + v->v_DataOffset);
	    /* Number of operations is the first thing stored */
	    opcount = *data++;


	    DV( OUT (" %ld", (LONG)v->v_Flags) );

	    /* determine type of vector data */
	    if (v->v_Flags & VF_POLYGON)
	    {
		/* just like DrawBorder */
		x1 = Scale(designWidth, Width, (*data++));
		y1 = Scale(designHeight,Height,(*data++));
		DPOLY( kprintf( "MOVE: %ld,%ld\n", x1, y1 ) );
		( *movef )( rport, x1, y1 );
		/* There are opcount vertices.  So we do one movef,
		 * followed by opcount-1 drawf's.
		 * SUBTLE:  We use PREDECREMENT to give us opcount-1
		 * iterations
		 */
		while (--opcount)
		{
		    x1 = Scale(designWidth, Width, (*data++));
		    y1 = Scale(designHeight,Height,(*data++));
		    DPOLY( kprintf( "DRAW: %ld,%ld\n", x1, y1 ) );
		    ( *drawf )( rport, x1, y1 );
		}
		DPOLY( kprintf( "END\n" ) );
		( *aendf )( rport );
		DPOLY( kprintf( "A-ok\n" ) );
	    }
	    else /* VF_RECTANGLE (i.e. zero) */
	    {
		/*  There are 'opcount' rectangle-quartets */
		while (opcount--)
		{
		    x1 = Scale(designWidth, Width, (*data++));
		    y1 = Scale(designHeight,Height,(*data++));
		    x2 = Scale(designWidth, Width, (*data++));
		    y2 = Scale(designHeight,Height,(*data++));
		    ( *rectf )( rport, x1, y1, x2, y2 );
		}
	    }
	}
	DS5( else if ( state == 5)
		printf("vector data not needed!, vstate %lx\n",v->v_States));
	v++;
    }

    DV ( OUT ("time for borders\n") );

    /*
     * now do 3D border treatment, do it now so that we don't get overwritten
     * by any special backfill treatment that they may do.
     */

    /* see if it needs a 3D border of the size of the image: */
    if ( border & ( VIB_3D | VIB_VERT | VIB_HORIZ | VIB_BRCORN ) )
    {
	DV( printf(" border 3D\n"));
	/* draw 3D border around image */
	Draw1P3DBorder (penspec, rport, direction, 0, Width, Height);
	/*  Top right corner gadget needs to be in shadow: */
	if ( border & ( VIB_BRCORN | VIB_VERT ) )
	{
	    WritePixelPen( rport, penspec[ SHADOWPEN ], Width - 1, 0);
	}

	if ( border & VIB_VERT )
	{
	    WritePixelPen( rport, penspec[ SHINEPEN ], 0, Height - 1 );
	}

	/*  Special monochrome treatment of edges: */
	if ( mono )
	{
	    /*  The pen color that is not filling the frame: */
	    penindex = BACKGROUNDPEN;
	    if ( state == VSB_INANORMAL )
	    {
		penindex = FILLPEN;
	    }

	    if ( border & ( VIB_VERT | VIB_BRCORN ) )
	    {
		/*  Top edge must be visible */
		MoveDrawPen( rport, penspec[ penindex ],
		    1, 0, Width - 3, 0);
	    }

	    if ( border & ( VIB_HORIZ | VIB_BRCORN ) )
	    {
		/*  Left edge must be visible */
		MoveDrawPen( rport, penspec[ penindex ],
		    0, 1, 0, Height - 3 );
	    }

	    /* penindex is either BACKGROUNDPEN or FILLPEN, which is
	     * the color not filling the frame.  Now we want to use
	     * the color which is filling the frame.
	     * This does the trick nicely:
	     */
	    penindex =  (FILLPEN + BACKGROUNDPEN) - penindex;

	    if ( border & VIB_VERT )
	    {
		/*  Bottom edge must be invisible */
		MoveDrawPen( rport, penspec[ penindex ],
		    1, Height - 1, Width - 3, 0 );
	    }

	    if ( border & VIB_HORIZ )
	    {
		/*  Right edge must be invisible */
		MoveDrawPen( rport, penspec[ penindex ],
		    Width - 1, 1, 0, Height - 3 );
	    }
	}
    }
    /* see if it is a border image placed on the left side */
    else if (border & VIB_HLEFT)
    {
	DV( printf(" border HBL\n"));

	/* system border image, left side */
	Draw1P3DBorder( penspec, rport, direction, 0, Width - 1, Height );

	/*  Bottom left corner gadget needs to shine a pixel: */
	WritePixelPen( rport, penspec[ SHINEPEN ], 0, Height - 1 );

	/* draw the indentation on the right side of the box */
	penindex = SHINEPEN;
	if ( ( mono ) && ( state == VSB_INANORMAL ) )
	{
	    penindex = BACKGROUNDPEN;
	}

	MoveDrawPen( rport, penspec[ penindex ],
	    Width - 1, 1, 0, Height - 3 );

	if ( ( mono ) && ( state != VSB_INANORMAL ) )
	{
	    MoveDrawPen( rport, penspec[ BACKGROUNDPEN ],
		Width - 2, 1, 0, Height - 3 );
	}

	/* Continue the window border in the pixel column that is the
	 * indentation.
	 */
	WritePixelPen( rport, penspec[ SHINEPEN ], Width - 1, 0 );
	WritePixelPen( rport, penspec[ SHADOWPEN ], Width - 1, Height - 1 );
    }
    /* see if it is a border image placed on the right side */
    else if (border & VIB_HRIGHT)
    {
	DV( printf(" border HBR\n"));

	/* system border image, right side */
	Draw1P3DBorder( penspec, rport, direction, 1, Width - 1, Height );

	/* draw the indentation on the left side of the box */
	penindex = SHADOWPEN;
	if ( ( mono ) && ( state == VSB_INANORMAL ) )
	{
	    penindex = BACKGROUNDPEN;
	}

	MoveDrawPen( rport, penspec[ penindex ], 0, 1, 0, Height - 3);

	if ( ( mono ) && ( state != VSB_INANORMAL ) )
	{
	    MoveDrawPen( rport, penspec[ BACKGROUNDPEN ],
		1, 1, 0, Height - 3 );
	}


	/* Continue the window border in the pixel column that is the
	 * indentation.
	 */
	WritePixelPen( rport, penspec[ SHINEPEN ], 0, 0 );
	WritePixelPen( rport, penspec[ SHADOWPEN ], 0, Height - 1 );
    }
    /*  Does it need a thick-sided 3D border? */
    else if ( border & VIB_THICK3D )
    {
	struct IBox box;
	box.Left = box.Top = 0;
	box.Width = Width;
	box.Height = Height;
	embossedBoxTrim( rport, &box,
	    penspec[ SHINEPEN ], penspec[ SHADOWPEN ], JOINS_ANGLED );
    }

    DS5( if ( state == 5 ) printf("done with a 5'er, rp at %lx", rport ) );
    DS5( if ( state == 5 ) printf("bitmap at %lx", rport->BitMap ) );
    DS5( if ( state == 5 ) Debug() );

    DV( printf("DrawVector all done\n"));
}

/* Rendering routines:  here are a bunch of routines that draw things
 * we need.
 */

/* Draw1P3DBorder() draws a 1-pixel thick 3D border.  We should
 * convert to using embossedBoxTrim() instead.
 */

VOID
Draw1P3DBorder( Pens, rp, dir, tx, w, h )
UWORD		*Pens;
struct RastPort *rp;
LONG		dir;
LONG 		tx;
LONG 		w;
LONG 		h;
{
    LONG lrb, ulb;

    ulb = Pens[ SHINEPEN ];
    lrb = Pens[ SHADOWPEN ];
    if (dir == 0)
    {
	ulb = Pens[ SHADOWPEN ];
	lrb = Pens[ SHINEPEN ];
    }

    /* upper left border */
    MoveDrawPen( rp, ulb, tx + w - 1, 0, 1-w, 0 );
    Draw( rp, tx, h - 2 );

    /* lower right border */
    MoveDrawPen( rp, lrb, tx, h - 1, w-1, 0 );
    Draw( rp, tx + w - 1, 1 );
}

/* myRectangle() draws an outline of a rectangle, with parameters
 * compatible with RectFill().
 */
void
myRectangle( rport, x1, y1, x2, y2 )
struct RastPort	*rport;
LONG		x1;
LONG		y1;
LONG		x2;
LONG		y2;
{

    Move( rport, x1, y1 );	/* upper-left */
    Draw( rport, x2, y1 );	/* upper-right */
    Draw( rport, x2, y2 );	/* lower-right */
    Draw( rport, x1, y2 );	/* lower-left */
    Draw( rport, x1, y1 );	/* upper left */
}


/*
 * Space-savings function that writes a pixel in a given color.
 * SIDE-EFFECT:  Changes your RastPort's APen.
 */

WritePixelPen( rport, pen, x, y )
{
    SetAPen( rport, pen );
    WritePixel( rport, x, y );
}


/*
 * Space-saving function that writes a line in a given color.
 * SIDE-EFFECT:  Affects your RastPort's APen and cursor-position.
 */

MoveDrawPen( rport, pen, x, y, dx, dy )
{
    SetAPen( rport, pen );
    Move( rport, x, y );
    Draw( rport, x+dx, y+dy );
}
