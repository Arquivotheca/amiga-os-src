/******************************************************************************
*
*	Source Control
*	--------------
*	$Header: db.c,v 36.68 89/03/17 18:43:58 bart Exp $
*
*	$Locker: bart $
*
******************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfxbase.h>
#include <graphics/gfx.h>
#include <graphics/rastport.h>
#include <graphics/view.h>
#include <graphics/display.h>
#include <graphics/monitor.h>
#include <graphics/displayinfo.h>
#include <graphics/videocontrol.h>
#include <libraries/dos.h>

#ifndef KPRINTF
#define kprintf printf
#endif

#ifdef DEBUG
#define X(x) 	x
#define D(x) 	x
#else
#define X(x) 	;
#define D(x) 	x
#endif

#define MAX(a,b)        ((a)>(b)?(a):(b))
#define MIN(a,b)        ((a)<(b)?(a):(b))

#define STREQ(a,b)	(!strcmp((a),(b)))

#define V1_POINT_4      36

#define EOF 			-1

struct DisplayBase *DisplayBase = NULL;
struct GfxBase  *GfxBase = NULL;

#define TXTOSCAN 1
#define STDOSCAN 2
#define MAXOSCAN 3
#define VIDOSCAN 4

int otype = NULL;

int debug = FALSE;
int test = FALSE;
int all = FALSE;

struct TagItem vc[] =
{
	{ VTAG_ATTACH_CM_SET, NULL },
	{ VTAG_VIEWPORTEXTRA_SET, NULL },
	{ VTAG_NORMAL_DISP_SET, NULL },
	{ VTAG_END_CM, NULL }
};

struct TagItem end_vc[] =
{
	{ VTAG_VIEWPORTEXTRA_GET, NULL },
	{ VTAG_END_CM, NULL }
};

struct DisplayInfo queryinfo;
struct NameInfo queryname;
struct DimensionInfo querydims;
struct MonitorInfo querymonitor;

pow( a, b )
{
	if(b-- <= 0) { return( 1 ); } else { return( a * pow( a, b ) ); }
}

md( ID, matchID )
ULONG ID;
ULONG matchID;
{
	int    success = FALSE;
	int    name = FALSE;
	int    info = FALSE;
	int    norm = FALSE;
	int    mntr = FALSE;

	if(info = 
	GetDisplayInfoData(NULL,&queryinfo,sizeof(queryinfo),DTAG_DISP,ID))
	{
		if(!~matchID) 
		{
			D(kprintf("\nID%08lx ",queryinfo.Header.DisplayID);)
			D(kprintf("%sactive ",queryinfo.NotAvailable?"in":"");)
			if(queryinfo.NotAvailable)
			{
				D(kprintf("\( ");)
				if(queryinfo.NotAvailable & DI_AVAIL_NOCHIPS)
				{
					D(kprintf("no chips ");)
				}
				if(queryinfo.NotAvailable & DI_AVAIL_NOMONITOR)
				{
					D(kprintf("no monitor ");)
				}
				D(kprintf("\) ");)
			}
		}
		else
		{
			if(queryinfo.Header.DisplayID == matchID) 
			{
				if(!(queryinfo.NotAvailable)) success= TRUE; 
			}
		}

		if(name = 
		GetDisplayInfoData(NULL,&queryname,sizeof(queryname),DTAG_NAME,ID))
		{
			if(!~matchID) D(kprintf("%s ",queryname.Name);)
		}

		if(mntr = 
		GetDisplayInfoData(NULL,&querymonitor,sizeof(querymonitor),DTAG_MNTR,ID))
		{
			if(debug) 
			{
				if( querymonitor.Mspc ) 
				D(kprintf(" \(%s\):",querymonitor.Mspc->ms_Node.xln_Name);)
				else 
				D(kprintf(" \(no current mspc\):");)
				D(kprintf("\n      ");)
			}

			if(debug) 
			{ 
				struct Rectangle *range = &querymonitor.ViewPositionRange;

				D(kprintf(" origin x = %02lx",querymonitor.ViewPosition.x);)
				D(kprintf(" y = %02lx",querymonitor.ViewPosition.y );)
				D(kprintf(" \n      ");)

				D(kprintf(" view range");)
				D(kprintf(" %4ld",range->MinX);)
				D(kprintf(" %4ld",range->MinY);)
				D(kprintf(" %4ld",range->MaxX);)
				D(kprintf(" %4ld",range->MaxY);)
				D(kprintf(" \n      ");)
			}
		}
		else
		{
			if(debug) D(kprintf(" \(monitor_not_found\): \n      ");)
		}

		if(debug)
		{
			D(kprintf(" ");)
			if(queryinfo.PropertyFlags)
			{

				if(queryinfo.PropertyFlags & DIPF_IS_LACE)
				{
					D(kprintf("LACE ");)
				}
				if(queryinfo.PropertyFlags & DIPF_IS_DUALPF)
				{
					D(kprintf("DUALPF ");)
				}
				if(queryinfo.PropertyFlags & DIPF_IS_PF2PRI)
				{
					D(kprintf("PF2PRI ");)
				}
				if(queryinfo.PropertyFlags & DIPF_IS_HAM)
				{
					D(kprintf("HAM ");)
				}
				if(queryinfo.PropertyFlags & DIPF_IS_ECS)
				{
					D(kprintf("ECS ");)
				}
				if(queryinfo.PropertyFlags & DIPF_IS_PAL)
				{
					D(kprintf("PAL ");)
				}
				if(queryinfo.PropertyFlags & DIPF_IS_SPRITES)
				{
					D(kprintf("SPRITES ");)
				}
				if(queryinfo.PropertyFlags & DIPF_IS_GENLOCK)
				{
					D(kprintf("GENLOCK ");)
				}
				if(queryinfo.PropertyFlags & DIPF_IS_WB)
				{
					D(kprintf("WB ");)
				}
				if(queryinfo.PropertyFlags & DIPF_IS_DRAGGABLE)
				{
					D(kprintf("DRAGGABLE ");)
				}
				if(queryinfo.PropertyFlags & DIPF_IS_PANELLED)
				{
					D(kprintf("PANELLED ");)
				}
				if(queryinfo.PropertyFlags & DIPF_IS_BEAMSYNC)
				{
					D(kprintf("BEAMSYNC");)
				}
			}
			else
			{
					D(kprintf("NULL ");)
			}
			D(kprintf("\n      ");)
		}
	}
	else
	{
		D(kprintf("\nID%08lx ",ID);)
		D(kprintf(" \(unavailable\): \n      ");)
	}

	if(norm = 
	GetDisplayInfoData(NULL,&querydims,sizeof(querydims),DTAG_DIMS,ID))
	{
		struct Rectangle *oscan;

		if(debug) 
		{ 
			D(kprintf(" minimum rasterwidth  ");)
			D(kprintf(" %4ld",querydims.MinRasterWidth);)
			D(kprintf(" ");)

			D(kprintf(" minimum rasterheight ");)
			D(kprintf(" %4ld",querydims.MinRasterHeight);)
			D(kprintf(" \n      ");)

			D(kprintf(" maximum rasterwidth  ");)
			D(kprintf(" %4ld",querydims.MaxRasterWidth);)
			D(kprintf(" ");)

			D(kprintf(" maximum rasterheight ");)
			D(kprintf(" %4ld",querydims.MaxRasterHeight);)
			D(kprintf(" \n      ");)

			D(kprintf(" maximum depth  %1ld",querydims.MaxDepth );) 
			D(kprintf(" \n      ");)

			oscan = &querydims.Nominal;

			D(kprintf(" nominal rectangle");)
			D(kprintf(" %4ld",oscan->MinX);)
			D(kprintf(" %4ld",oscan->MinY);)
			D(kprintf(" %4ld",oscan->MaxX);)
			D(kprintf(" %4ld",oscan->MaxY);)
			D(kprintf(" \n      ");)

			oscan = &querydims.TxtOScan;

			D(kprintf(" text_oscan");)
			D(kprintf(" %4ld",oscan->MinX);)
			D(kprintf(" %4ld",oscan->MinY);)
			D(kprintf(" %4ld",oscan->MaxX);)
			D(kprintf(" %4ld",oscan->MaxY);)
			D(kprintf(" \n      ");)

			oscan = &querydims.StdOScan;

			D(kprintf(" std__oscan");)
			D(kprintf(" %4ld",oscan->MinX);)
			D(kprintf(" %4ld",oscan->MinY);)
			D(kprintf(" %4ld",oscan->MaxX);)
			D(kprintf(" %4ld",oscan->MaxY);)
			D(kprintf(" \n      ");)

			oscan = &querydims.MaxOScan;

			D(kprintf(" max__oscan");)
			D(kprintf(" %4ld",oscan->MinX);)
			D(kprintf(" %4ld",oscan->MinY);)
			D(kprintf(" %4ld",oscan->MaxX);)
			D(kprintf(" %4ld",oscan->MaxY);)
			D(kprintf(" \n      ");)

			oscan = &querydims.VideoOScan;

			D(kprintf(" videooscan");)
			D(kprintf(" %4ld",oscan->MinX);)
			D(kprintf(" %4ld",oscan->MinY);)
			D(kprintf(" %4ld",oscan->MaxX);)
			D(kprintf(" %4ld",oscan->MaxY);)
			D(kprintf(" \n      ");)
			}
	}

	return( (info && mntr && norm)? success : FALSE );
}

display_db(argv)
UBYTE *argv;
{
	ULONG       ID = INVALID_ID;
	ULONG found_id = INVALID_ID;
	ULONG match_id = INVALID_ID;


	if(argv) 
	{ 
		if(sscanf(argv,"%lx",&match_id) == EOF) return(INVALID_ID); 
	}
	else 
	{ 
		D(kprintf("----- mode information -----\n");) 
	}

	while((ID = NextDisplayInfo( ID )) != INVALID_ID)
	{
		if(all) 
		{
			if(md( ID, ID)) 
			{
				open(ID);
			}
		}
		else
		{
			if(md( ID, match_id)) 
			{
				if(!(~found_id)) found_id = ID; break;
			}
		}
	}

	if(!argv) 
	{ 
		D(kprintf("\n----------------------------\n");) 
	}

	return( found_id );
}


create_display( ID, vp, rp )
ULONG ID;
struct ViewPort *vp;
struct RastPort *rp;
{
    int error = FALSE;
    int i,j,k;
    char text[1];


    if(vp && rp)
    {
        SetAPen(  rp,  1 );

		RectFill( rp, 0, 0, vp->DWidth-1, vp->DHeight-1 );

        SetAPen(  rp,  -1 );
		SetDrMd(  rp,  JAM1 );

		/* circle in the center, radius 1/4 the width of the screen */
		{
			WORD x_center = vp->DWidth  >> 1;
			WORD y_center = vp->DHeight >> 1;
			WORD width  =   vp->DWidth  >> 2;
			WORD height = 
			(((LONG)(width * queryinfo.Resolution.x))/queryinfo.Resolution.y);
			DrawEllipse( rp, x_center, y_center, width, height );
		}

		/* outline the rectangle */
		{
                Move( rp, 0, 			0 );
                Draw( rp, 0, 			vp->DHeight-1);
                Draw( rp, vp->DWidth-1, vp->DHeight-1);
                Draw( rp, vp->DWidth-1, 0);
                Draw( rp, 0, 			0);
		}

		/* indicate diagonals */
		{
                Move( rp, 0, 			0 );
                Draw( rp, vp->DWidth-1, vp->DHeight-1);
                Move( rp, 0, 			vp->DHeight-1);
                Draw( rp, vp->DWidth-1, 0);
		}

		/* dimension counters */
        for(i=0; i < vp->DWidth; i+=(1<<4))
        {
                Move( rp, i, 0 );
                Draw( rp, i, vp->DHeight-1);
                if(i == 0)
                for(k=0; k+12 < vp->DHeight-1; k+=(1<<4))
                {
                        *text = '0'+((k>>4)%10);
                        Move( rp, i+4, k+12 );
                        Text( rp, text, 1);
                }
        }

        for(j=0; j < vp->DHeight; j+=(1<<4))
        {
                Move( rp, 0, j );
                Draw( rp, vp->DWidth-1, j);
                if(j == 0)
                for(k=0; k+12 < vp->DWidth-1; k+=(1<<4))
                {
                        *text = '0'+((k>>4)%10);
                        Move( rp, k+4, j+12 );
                        Text( rp, text, 1);
                }
        }
    }
    else
    {
        error = TRUE;
    }

    return( error );
}

struct RastPort *create_rp( ID, bm )
ULONG ID;
struct BitMap *bm;
{
    struct RastPort *rp = NULL;

    if( rp = AllocMem( sizeof(*rp), MEMF_PUBLIC|MEMF_CLEAR ) )
    {
        InitRastPort( rp );
        rp->BitMap = bm;
    }

    return( rp );
}

dispose_rp( rp )
struct RastPort *rp;
{
    int error = FALSE;

    if( rp )
    {
        FreeMem( rp, sizeof(*rp));
    }
    else
    {
        error = TRUE;
    }

    return( error );
}

struct RasInfo *create_ri( ID, bm )
ULONG ID;
struct BitMap *bm;
{
    struct RasInfo *ri = NULL;

    if( ri = AllocMem( sizeof(*ri), MEMF_PUBLIC|MEMF_CLEAR ) )
    {
        ri->BitMap = bm;
    }

    return( ri );
}

dispose_ri( ri )
struct RasInfo *ri;
{
    int error = FALSE;

    if( ri )
    {
        FreeMem( ri, sizeof(*ri));
    }
    else
    {
        error = TRUE;
    }

    return( error );
}

struct BitMap *create_bm( ID, vp )
ULONG ID;
struct ViewPort *vp;
{
    struct BitMap  *bm = NULL;
	ULONG DEPTH = querydims.MaxDepth;

	if( DEPTH )
	{
		if( bm = AllocMem( sizeof(*bm), MEMF_PUBLIC|MEMF_CLEAR ) )
		{
			int i;

			InitBitMap(bm, DEPTH, vp->DWidth, vp->DHeight );

			/* allocate chip memory for bitmap planes */
			for (i = 0; i < bm->Depth; i++)
			{
				if( bm->Planes[i] = 
					AllocMem( RASSIZE(vp->DWidth, vp->DHeight), MEMF_CHIP | MEMF_CLEAR ) )
				{
					continue;
				}
				else
				{
					dispose_bm( bm );
					return( NULL );
				}
			}
		}
	}

    return( bm ); 
}

dispose_bm( bm )
struct BitMap *bm;
{
    int error = FALSE;

    if( bm )
    {
        int i;

        for(i=0; i < bm->Depth; i++)
        {
            if( bm->Planes[i] ) 
            {
                FreeMem( bm->Planes[i], bm->BytesPerRow * bm->Rows );
            }
        }
        FreeMem( bm, sizeof(*bm));
    }
    else
    {
        error = TRUE;
    }

    return( error );
}

struct ViewPort *create_vp( ID, view )
ULONG ID;
struct View *view;
{
    struct ViewPort    *vp     = NULL;
	struct Rectangle   *oscan;
	LONG   DX;
	LONG   DY;
	ULONG  WIDTH;
	ULONG  HEIGHT;

	switch( otype )
	{
		case TXTOSCAN: oscan = &querydims.TxtOScan; break;
		case STDOSCAN: oscan = &querydims.StdOScan; break;
		case MAXOSCAN: oscan = &querydims.MaxOScan; break;
		case VIDOSCAN: oscan = &querydims.VideoOScan; break;
		default:       oscan = &querydims.Nominal; break;
	}

	DX     = oscan->MinX;
	DY     = oscan->MinY;
	WIDTH  = oscan->MaxX-oscan->MinX+1;
	HEIGHT = oscan->MaxY-oscan->MinY+1;

	if( WIDTH && HEIGHT )  
	if( vp = AllocMem( sizeof(*vp), MEMF_PUBLIC|MEMF_CLEAR ) )
	{
		int	error = FALSE;
		struct ViewPortExtra *vpx = NULL;

		InitVPort( vp );

		/* wow! -- 1.2 compatibility mode */

		vp->Modes    = (ID & 0xFFFF); 

		/* standard size and position productivity viewport */

		vp->DxOffset = DX;
		vp->DyOffset = DY;
		vp->DWidth   = WIDTH;
		vp->DHeight  = HEIGHT;

		/* determine displayclip rectangle for this viewport */

		if (view->Modes & EXTEND_VSTRUCT)
		{
			struct ViewExtra *vx;

			if( vx = GfxLookUp( view ) )
			{
				struct MonitorSpec *mspc;

				if( mspc = vx->Monitor )
				{
					WORD  SHIFT_X =  0;
					WORD  SHIFT_Y =  0;
					struct Rectangle rect;

					X(kprintf("mspc == %lx\n",mspc);)

					/* specify displayclip in view relative coordinates */

					if( !( mspc->ms_Flags & REQUEST_A2024 ) )
					{
						struct Rectangle *oscan;

						switch( otype )
						{
							case TXTOSCAN: oscan = &querydims.TxtOScan; break;
							case STDOSCAN: oscan = &querydims.StdOScan; break;
							case MAXOSCAN: oscan = &querydims.MaxOScan; break;
							case VIDOSCAN: oscan = &querydims.VideoOScan; break;
							default:       oscan = &querydims.Nominal; break;
						}

						if( vpx = GfxNew( VIEWPORT_EXTRA_TYPE ) )
						{
							vpx->DisplayClip.MinX = oscan->MinX;
							vpx->DisplayClip.MinY = oscan->MinY;
							vpx->DisplayClip.MaxX = oscan->MaxX;
							vpx->DisplayClip.MaxY = oscan->MaxY;
						}
					}
				}
			}
		}

		/* pass new information to the system via colormap structure */

		vc[0].ti_Data = vp;
		vc[1].ti_Data = vpx;

		if(!test) 
		{
			vc[2].ti_Data = (ULONG) FindDisplayInfo( ID );
		}
		else
		{
			vc[2].ti_Data = NULL;
		}

		/* get a full colormap */

		error = VideoControl( GetColorMap(32), vc );

		/* set the global lace */

		if(!error && !test)
		{
			view->Modes |= ((DIPF_IS_LACE & queryinfo.PropertyFlags)? LACE: 0);
		}
		else
		{
			view->Modes |= (LACE & vp->Modes);
		}
	}

    return( vp ); 
}

dispose_vp( vp)
struct ViewPort *vp;
{
    int error = FALSE;

    if( vp )
    {
		struct ViewPortExtra *vpx;

		error = VideoControl( vp->ColorMap, end_vc );

		if(!error)
		{
			if(vpx = end_vc[0].ti_Data)
			{
				GfxFree( vpx );
			}
		}

        if ( vp->ColorMap )
        {
            FreeColorMap( vp->ColorMap );
        }
        FreeVPortCopLists( vp );
        FreeMem( vp, sizeof(*vp) );
    }
    else
    {
        error = TRUE;
    }

    return( error );
}

struct View *create_view( ID, mspc )
ULONG ID;
struct MonitorSpec *mspc;
{
    struct View *view;

    if( view = AllocMem( sizeof(*view), MEMF_PUBLIC|MEMF_CLEAR ) )
    {
        struct ViewExtra *vx;

        InitView( view );

        if( vx = GfxNew( VIEW_EXTRA_TYPE ) )
        {
            vx->Monitor = mspc; 
            GfxAssociate( view, vx );
            view->Modes |= EXTEND_VSTRUCT;
        }

        view->DxOffset = querymonitor.ViewPosition.x ;
        view->DyOffset = querymonitor.ViewPosition.y ;
    }

    return( view );
}

dispose_view( view)
struct View *view;
{
    int error = FALSE;

    if( view )
    {
        if (view->Modes & EXTEND_VSTRUCT)
        {
            struct ViewExtra *vx;

            if( vx = GfxLookUp( view ) )
            {
                GfxFree( vx );
            }
        }
        if ( view->LOFCprList ) FreeCprList ( view->LOFCprList );
        if ( view->SHFCprList ) FreeCprList ( view->SHFCprList );

        FreeMem( view, sizeof(*view) );
    }
    else
    {
        error = TRUE;
    }

    return( error );
}

open( ID )
ULONG ID;
{
	int error = FALSE;
	struct MonitorSpec *mspc, *OpenMonitor();

	X(kprintf("open: ID = %lx\n",ID);)

	if(!test)
	{
		mspc = OpenMonitor( NULL, ID ) ;
	}
	else
	{
		mspc = OpenMonitor( NULL, NULL ) ;
	}

	if( mspc )
	{
		struct View *view, *create_view();

		X(kprintf("open: mspc = %lx\n",mspc);)

		if( view = create_view( ID, mspc ) )
		{
			struct ViewPort *vp, *create_vp();

			X(kprintf("open: view = %lx\n",view);)

			if( vp = create_vp( ID, view ) )
			{
				struct BitMap *bm, *create_bm();

				X(kprintf("open: vp = %lx\n",vp);)

				view->ViewPort = vp;

				if( bm = create_bm( ID, vp ) )
				{
					struct RasInfo *ri, create_ri();

					X(kprintf("open: bm = %lx\n",bm);)

					if( ri = create_ri( ID, bm ) )
					{
						struct RastPort *rp, create_rp();

						X(kprintf("open: ri = %lx\n",ri);)

						vp->RasInfo = ri;

						if( rp = create_rp( ID, bm ) )
						{
							/* show the productivity viewport */

							X(kprintf("open: rp = %lx\n",rp);)

							if(!(error = create_display(ID,vp,rp)))
							{
								struct View *restore_view;
								restore_view = GfxBase->ActiView;

								MakeVPort( view, vp );
								MrgCop( view );
								LoadView( view );

								X(kprintf("open: wait for CTRL_C\n");)
								Wait(SIGBREAKF_CTRL_C);

								LoadView( restore_view );
							}
							
							dispose_rp( rp );
						}
						
						dispose_ri( ri );
					}
					
					dispose_bm( bm );
				}
				
				dispose_vp( vp );
			}
			
			dispose_view( view );
		}
		else
		{
			error = TRUE;
		}

		CloseMonitor( mspc );
	}
	else
	{
			error = TRUE;
	}

	return(error);
}

main(argc,argv)
int argc;
UBYTE *argv[];
{
	int error = FALSE;

	if( (argc) && ( GfxBase = OpenLibrary( "graphics.library", V1_POINT_4 )) )
	{
		/* display database by name */

		if(argc == 1)
		{
			display_db(NULL);
		}
		else
		{
			if( STREQ(*(argv+1),"-debug") ) debug = TRUE;
			if( STREQ(*(argv+1),"-all") ) all = TRUE;

			if((argc == 2 && debug)||(argc == 2 && all))
			{
				display_db(NULL);
			}
			else
			{
				debug = FALSE;
				all = FALSE;

				while(--argc) 
				{	
					if( **(++argv) == '-' ) 
					{
						if( STREQ(*argv  ,"-test") ) test = TRUE;
						if( STREQ(*argv,"-txt") ) otype = TXTOSCAN;
						if( STREQ(*argv,"-std") ) otype = STDOSCAN;
						if( STREQ(*argv,"-max") ) otype = MAXOSCAN;
						if( STREQ(*argv,"-vid") ) otype = VIDOSCAN;
						D(kprintf("db: use overscan %s option\n",(*argv)+1);)
						continue;
					}
					else
					{
						ULONG id = display_db(*argv);
						D(kprintf("db: open %s ID == 0x%08lx\n",*argv,id);)
						if(~id) error = open(id);
					}
				}
			}
		}

		CloseLibrary( GfxBase ); /* clean up and exit */
	}
	else
	{
		error = TRUE;
	}

	exit(error);
}
