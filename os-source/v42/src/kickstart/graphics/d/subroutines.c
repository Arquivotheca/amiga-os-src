/******************************************************************************
*
*   $Id: subroutines.c,v 42.0 93/06/16 11:18:15 chrisg Exp $
*
*   $Locker:  $
*
******************************************************************************/

/* subroutines.c -- recipies for cooking and uncooking chunkdata */

#include    "/displayinfo_internal.h"
#include "/macros.h"
#include <pragmas/utility_pragmas.h>
#include	"d.protos"

/*#define DEBUG*/
/*#define GBDEBUG*/

#ifdef GBDEBUG
#define GBUG if (GBASE->Debug)
#else
#define GBUG
#endif

#ifdef DEBUG
#define D(x) {GBUG {x};}
#else
#define D(x)
#endif

#define CLIPOSCAN

ULONG __regargs cook_oscan( flag, dinfo, cook, size, raw, bytes, src, dst )
struct DisplayInfo *dinfo;
ULONG  cook, *size, raw, *bytes;
struct Rect32 **src;
struct Rectangle **dst;
int    flag;
{
    ULONG result = 0;

    if((cook <= (*size)) && (raw <= (*bytes))) 
    {
	if( flag )
	{
	    (*dst)->MinX = 
	    (*src)->MinX / dinfo->Resolution.x;

	    (*dst)->MaxX = 
	    (*src)->MaxX / dinfo->Resolution.x;

	    (*dst)->MinY = 
	    (*src)->MinY / dinfo->Resolution.y;

	    (*dst)->MaxY = 
	    (*src)->MaxY / dinfo->Resolution.y;
	}

	(*size) -= cook;
	(*bytes) -= raw;

	*dst = (struct Rectangle *)((*(UBYTE **)dst)+cook);
	*src = (struct Rect32 *)((*(UBYTE **)src)+raw);

	result = cook;
    }

    return(result);
}

ULONG __regargs uncook_oscan( flag, dinfo, cook, size, raw, bytes, src, dst )
struct DisplayInfo *dinfo;
ULONG  cook, *size, raw, *bytes;
struct Rectangle **src;
struct Rect32 **dst;
int    flag;
{
    ULONG result = 0;

    if((cook <= (*size)) && (raw <= (*bytes))) 
    {
	if( flag )
	{
	      (*dst)->MinX = 
	    (((*src)->MinX)  ) * dinfo->Resolution.x;

	      (*dst)->MaxX = 
	    (((*src)->MaxX)+1) * dinfo->Resolution.x - 1;

	      (*dst)->MinY = 
	    (((*src)->MinY)  ) * dinfo->Resolution.y;

	      (*dst)->MaxY = 
	    (((*src)->MaxY)+1) * dinfo->Resolution.y - 1;
	}

	(*size) -= cook;
	(*bytes) -= raw;

	*dst = (struct Rect32 *)((*(UBYTE **)dst)+raw);
	*src = (struct Rectangle *)((*(UBYTE **)src)+cook);

	result = cook;
    }

    return(result);
}

ULONG __regargs get_dims(record,src,size,tagID,ID,dst,bytes)
struct DisplayInfoRecord **record;
UBYTE **src;
ULONG *size;
ULONG *tagID;
ULONG *ID;
UBYTE **dst;
ULONG *bytes;
{
    ULONG result;
    ULONG raw;
    ULONG cook;
    struct RawDisplayInfo *dinfo;
    struct DimensionInfo *dims = (struct DimensionInfo *)*dst;
    ULONG *stupidcompiler = 0;

    D(kprintf("get_dims ID = 0x%lx\n", *ID);)
    result = copy_dbstuff(src, size, dst, bytes, ((ULONG)&(((struct RawDimensionInfo *)stupidcompiler)->Nominal)));

    if(dinfo = (struct RawDisplayInfo *)FindTagItem(DTAG_DISP,&((*record)->rec_Tag)))
    {
	if(dinfo->Resolution.x && dinfo->Resolution.y)
	{
	    struct DisplayInfoRecord *mntr= (struct DisplayInfoRecord *)((*record)->rec_Node.rcn_Parent);
	    struct RawMonitorInfo *minfo;
	    ULONG add;

	    raw = ((ULONG)&(((struct RawDimensionInfo *)stupidcompiler)->MaxOScan))-
		       ((ULONG)&(((struct RawDimensionInfo *)stupidcompiler)->Nominal));
	    cook = ((ULONG)&(((struct DimensionInfo *)stupidcompiler)->MaxOScan))-
			((ULONG)&(((struct DimensionInfo *)stupidcompiler)->Nominal));

	    if(add = cook_oscan(TRUE,(struct DisplayInfo *)dinfo,cook,size,raw,bytes,(struct Rect32 **)src, (struct Rectangle **)dst))
	    { result += add; } else { return(result); }

	    raw = ((ULONG)&(((struct RawDimensionInfo *)stupidcompiler)->VideoOScan))-
		  ((ULONG)&(((struct RawDimensionInfo *)stupidcompiler)->MaxOScan));
	    cook = ((ULONG)&(((struct DimensionInfo *)stupidcompiler)->VideoOScan))-
		   ((ULONG)&(((struct DimensionInfo *)stupidcompiler)->MaxOScan));

	    if(add = cook_oscan(TRUE,(struct DisplayInfo *)dinfo,cook,size,raw,bytes,(struct Rect32 **)src, (struct Rectangle **)dst))
	    { result += add; } else { return(result); }

	    raw = ((ULONG)&(((struct RawDimensionInfo *)stupidcompiler)->pad))-
	    	  ((ULONG)&(((struct RawDimensionInfo *)stupidcompiler)->VideoOScan));
	    cook = ((ULONG)&(((struct DimensionInfo *)stupidcompiler)->TxtOScan))-
		   ((ULONG)&(((struct DimensionInfo *)stupidcompiler)->VideoOScan));

	    if(add = cook_oscan(TRUE,(struct DisplayInfo *)dinfo,cook,size,raw,bytes,(struct Rect32 **)src, (struct Rectangle **)dst))
	    { result += add; } else { return(result); }

	    if((mntr) && (minfo = (struct RawMonitorInfo *)(FindTagItem(DTAG_MNTR,&mntr->rec_Tag))))
	    {
		*src = (UBYTE *)(&minfo->TxtOScan);

		raw = ((ULONG)&(((struct RawMonitorInfo *)stupidcompiler)->StdOScan))-
		      ((ULONG)&(((struct RawMonitorInfo *)stupidcompiler)->TxtOScan));
		cook = ((ULONG)&(((struct DimensionInfo *)stupidcompiler)->StdOScan))-
		       ((ULONG)&(((struct DimensionInfo *)stupidcompiler)->TxtOScan));

		if(add = cook_oscan(TRUE,(struct DisplayInfo *)dinfo,cook,size,raw,&raw,(struct Rect32 **)src, (struct Rectangle **)dst))
		{ result += add; } else { return(result); }

		*src = (UBYTE *)(&minfo->StdOScan);

		raw = ((ULONG)&(((struct RawMonitorInfo *)stupidcompiler)->DefaultViewPosition.x))-
		      ((ULONG)&(((struct RawMonitorInfo *)stupidcompiler)->StdOScan));
		cook = ((ULONG)&(((struct DimensionInfo *)stupidcompiler)->pad))-
		       ((ULONG)&(((struct DimensionInfo *)stupidcompiler)->StdOScan));

		if (add = cook_oscan(TRUE,(struct DisplayInfo *)dinfo,cook,size,raw,&raw,(struct Rect32 **)src, (struct Rectangle **)dst))
		{
			result += add;
		}
		/* For SuperHires modes under ECS, the MaxOScan is 8 pixels less than
		 * twice the Hires MaxOScan, and this is reflected in the database.
		 * However, we have to ensure here that we cannot be setting the
		 * TxtOScan and StdOScan values greater than the MaxOScan values.
		 */
		{
			D(kprintf("Range check. dims = 0x%lx\n", dims);)
			D(kprintf("TxtMaxX = 0x%lx, StdMaxX = 0x%lx, MaxMaxX = 0x%lx\n", dims->TxtOScan.MaxX, dims->StdOScan.MaxX, dims->MaxOScan.MaxX);)
			dims->TxtOScan.MaxX = MIN(dims->TxtOScan.MaxX, dims->MaxOScan.MaxX);
			dims->StdOScan.MaxX = MIN(dims->StdOScan.MaxX, dims->MaxOScan.MaxX);
		}
	    }
	}
    }

    return( result );
}


ULONG __regargs set_dims(record,src,size,tagID,ID,dst,bytes)
struct DisplayInfoRecord **record;
UBYTE **src;
ULONG *size;
ULONG *tagID;
ULONG *ID;
UBYTE **dst;
ULONG *bytes;
{
    ULONG result;
    ULONG raw;
    struct RawDisplayInfo *dinfo;
    ULONG cook;
    ULONG *stupidcompiler = 0;

    result = copy_dbstuff(src, size, dst, bytes, ((ULONG)&(((struct RawDimensionInfo *)stupidcompiler)->Nominal)));

    if(dinfo = (struct RawDisplayInfo *)(FindTagItem(DTAG_DISP,&((*record)->rec_Tag))))
    {
	if(dinfo->Resolution.x && dinfo->Resolution.y)
	{
	    struct DisplayInfoRecord *mntr= (struct DisplayInfoRecord *)((*record)->rec_Node.rcn_Parent);
	    struct RawMonitorInfo *minfo;
	    ULONG add;

	    raw = ((ULONG)&(((struct RawDimensionInfo *)stupidcompiler)->MaxOScan))-
		       ((ULONG)&(((struct RawDimensionInfo *)stupidcompiler)->Nominal));
	    cook = ((ULONG)&(((struct DimensionInfo *)stupidcompiler)->MaxOScan))-
			((ULONG)&(((struct DimensionInfo *)stupidcompiler)->Nominal));

	    if(add = uncook_oscan(TRUE,(struct DisplayInfo *)dinfo,cook,size,raw,bytes,(struct Rectangle **)src, (struct Rect32 **)dst))
	    { result += add; } else { return(result); }

	    raw = ((ULONG)&(((struct RawDimensionInfo *)stupidcompiler)->VideoOScan))-
		  ((ULONG)&(((struct RawDimensionInfo *)stupidcompiler)->MaxOScan));
	    cook = ((ULONG)&(((struct DimensionInfo *)stupidcompiler)->VideoOScan))-
		   ((ULONG)&(((struct DimensionInfo *)stupidcompiler)->MaxOScan));

	    if(add = uncook_oscan(TRUE,(struct DisplayInfo *)dinfo,cook,size,raw,bytes,(struct Rectangle **)src, (struct Rect32 **)dst))
	    { result += add; } else { return(result); }

	    raw = ((ULONG)&(((struct RawDimensionInfo *)stupidcompiler)->pad))-
	    	  ((ULONG)&(((struct RawDimensionInfo *)stupidcompiler)->VideoOScan));
	    cook = ((ULONG)&(((struct DimensionInfo *)stupidcompiler)->TxtOScan))-
		   ((ULONG)&(((struct DimensionInfo *)stupidcompiler)->VideoOScan));

	    if(add = uncook_oscan(TRUE,(struct DisplayInfo *)dinfo,cook,size,raw,bytes,(struct Rectangle **)src, (struct Rect32 **)dst))
	    { result += add; } else { return(result); }

	    if((mntr) && (minfo = (struct RawMonitorInfo *)(FindTagItem(DTAG_MNTR,&mntr->rec_Tag))))
	    {
		*dst = (UBYTE *)(&minfo->TxtOScan);

		raw = ((ULONG)&(((struct RawMonitorInfo *)stupidcompiler)->StdOScan))-
		      ((ULONG)&(((struct RawMonitorInfo *)stupidcompiler)->TxtOScan));
		cook = ((ULONG)&(((struct DimensionInfo *)stupidcompiler)->StdOScan))-
		       ((ULONG)&(((struct DimensionInfo *)stupidcompiler)->TxtOScan));

		if(add = uncook_oscan(TRUE,(struct DisplayInfo *)dinfo,cook,size,raw,&raw,(struct Rectangle **)src, (struct Rect32 **)dst))
		{ result += add; } else { return(result); }

		*dst = (UBYTE *)(&minfo->StdOScan);

		raw = ((ULONG)&(((struct RawMonitorInfo *)stupidcompiler)->DefaultViewPosition.x))-
		      ((ULONG)&(((struct RawMonitorInfo *)stupidcompiler)->StdOScan));
		cook = ((ULONG)&(((struct DimensionInfo *)stupidcompiler)->pad))-
		       ((ULONG)&(((struct DimensionInfo *)stupidcompiler)->StdOScan));

		if(add = uncook_oscan(TRUE,(struct Displayinfo *)dinfo,cook,size,raw,&raw,(struct Rectangel **)src, (struct Rect32 **)dst))
		{ result += add; } else { return(result); }
	    }
	}
    }

    return( result );
}

ULONG __regargs set_mntr(record,src,size,tagID,ID,dst,bytes)
struct DisplayInfoRecord **record;
UBYTE **src;
ULONG *size;
ULONG *tagID;
ULONG *ID;
UBYTE **dst;
ULONG *bytes;
{
    ULONG *stupidcompiler = 0;
    ULONG result;
    ULONG raw = ((ULONG)&(((struct RawMonitorInfo *)stupidcompiler)->ViewPosition));
    Point *op = (Point *)((*dst)+raw);
    Point *np = (Point *)((*src)+raw);

    D(kprintf("set_mntr: enter...\n"););

    /* copy up to viewposition */

    D(kprintf("set_mntr: copy up to viewposition...\n"););
    result = copy_dbstuff(src, size, dst, bytes, raw);

    raw = ( ((ULONG)&(((struct RawMonitorInfo *)stupidcompiler)->reserved))
	  - ((ULONG)&(((struct RawMonitorInfo *)stupidcompiler)->ViewPosition))   );

    D(kprintf("set_mntr: new raw == %08lx\n",raw););

    D(kprintf("set_mntr: op %08lx ",op););
    D(kprintf("set_mntr: old viewposition %08lx \n",*((ULONG *)op)););
    D(kprintf("set_mntr: np %08lx ",np););
    D(kprintf("set_mntr: new viewposition %08lx \n",*((ULONG *)np)););

    /* check viewposition changed */

    if( *(ULONG *)np != *((ULONG *)op ) )
    {
	struct DisplayInfoRecord **mntr = (struct DisplayInfoRecord **)(&((*record)->rec_Node.rcn_Parent));
	struct DisplayInfoRecord **mode = (struct DisplayInfoRecord **)(&((*mntr)->rec_Node.rcn_Child));
	LONG   DX = (op->x - np->x) * (op+1)->x; 	/* ticks DX */
	LONG   DY = (op->y - np->y) * (op+1)->y; 	/* ticks DY */

	Forbid();

	D(kprintf("set_mntr: viewposition changed...\n"););
	D(kprintf("set_mntr: ticks DX == %ld \n",DX););
	D(kprintf("set_mntr: ticks DY == %ld \n",DY););
	D(kprintf("set_mntr: mntr == %08lx id == %08lx\n", \
		*mntr,*(ULONG *)&((*mntr)->rec_MajorKey)););

	if(!mode) D(kprintf("set_mntr: no children...\n"););

	while(*mode)
	{
	    struct RawDimensionInfo *dims = 
				     (struct RawDimensionInfo *)(FindTagItem(DTAG_DIMS,&((*mode)->rec_Tag)));
#ifdef	    CLIPOSCAN
	    struct RawDisplayInfo *dinfo; 
#endif

	    D(kprintf("set_mntr: mode== %08lx id == %08lx\n", \
		    *mode,*(ULONG *)&((*mode)->rec_MajorKey)););

	    if( dims )
	    {
		D(kprintf("set_mntr: dims == %08lx\n",dims););

		dims->MaxOScan.MinX += DX;
		dims->MaxOScan.MinY += DY;
		dims->MaxOScan.MaxX += DX;
		dims->MaxOScan.MaxY += DY;
		dims->VideoOScan.MinX += DX;
		dims->VideoOScan.MinY += DY;
		dims->VideoOScan.MaxX += DX;
		dims->VideoOScan.MaxY += DY;

		D(kprintf("set: max.miny = %08lx\n",dims->MaxOScan.MinY););
		D(kprintf("set: max.maxx = %08lx\n",dims->MaxOScan.MaxX););
		D(kprintf("set: max.maxy = %08lx\n",dims->MaxOScan.MaxY););
		D(kprintf("set: vid.minx = %08lx\n",dims->VideoOScan.MinX););
		D(kprintf("set: vid.miny = %08lx\n",dims->VideoOScan.MinY););
		D(kprintf("set: vid.maxx = %08lx\n",dims->VideoOScan.MaxX););
		D(kprintf("set: vid.maxy = %08lx\n",dims->VideoOScan.MaxY););
	    }

#ifdef	    CLIPOSCAN
	    if(dinfo = (struct RawDisplayInfo *)(FindTagItem(DTAG_DISP,&((*mode)->rec_Tag))))
	    {
		WORD dx = DX/dinfo->Resolution.x;
		WORD dy = DY/dinfo->Resolution.y;

		(*mode)->rec_ClipOScan.MinX += dx;
		(*mode)->rec_ClipOScan.MinY += dy;
		(*mode)->rec_ClipOScan.MaxX += dx;
		(*mode)->rec_ClipOScan.MaxY += dy;
	    }
#endif
	    D(kprintf("set: max.minx = %08lx\n",dims->MaxOScan.MinX););

	    mode = (struct DisplayInfoRecord **)(&((*mode)->rec_Node.rcn_Succ));
	}

	Permit();
    }

    /* copy from viewposition to termination */

    D(kprintf("set_mntr: copy copy from viewposition to termination...\n"););
    result = copy_dbstuff(src, size, dst, bytes, raw);

    D(kprintf("set_mntr: raw = %08lx\n",raw););
    D(kprintf("set_mntr: result = %08lx\n",result););

    return( result );
}
