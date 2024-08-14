/* dump.c -- Intuition debugging scan library
 *  $Id: dump.c,v 38.1 92/03/26 16:25:35 peter Exp $
 */

#include "intuall.h"
#include <graphics/regions.h>

#define DEBUG	1

#if DEBUG

#if 0
jstrlen(s)
char	*s;
{
    int	cc = 0;
    while (*s++) ++cc;
    return (cc);
}
#endif

tabin(num)
{
    while (num--) printf("\t");
}

#if 0
dumpRect(s, r)
UBYTE *s;
struct Rectangle *r;
{
    int	numtabs;
    int	i;

    numtabs = (strlen(s))/8;

    tabin(numtabs + 1); printf("   %ld\n", r->MinY);
    printf("%s\t%ld\t%ld\n", s, r->MinX, r->MinX);
    tabin(numtabs + 1); printf("   %ld\n", r->MaxY);
}
#else
dumpRect(s, r)
UBYTE *s;
struct Rectangle *r;
{
    int numtabs;

    numtabs = (strlen(s))/8;
    printf("%s", s);
    if ((numtabs = (4 - numtabs)) > 0) tabin( numtabs );
    printf("X:[%ld,%ld]\tY:[%ld,%ld]\n",r->MinX,r->MaxX,r->MinY,r->MaxY);
}
#endif

dumpPt(s, p)
UBYTE *s;
struct Point p;
{
    printf(":%s:\t", s);
    printf("\tx/y: %ld, %ld\n", p.X, p.Y);
}

dumpBox(s, b)
UBYTE *s;
struct IBox *b;
{
    printf("[%s]\t", s);
    printf("\tl/t: %ld, %ld; w/h: %ld, %ld\n",
	b->Left, b->Top, b->Width, b->Height);
}

dumpRegion(s, r)
UBYTE *s;
struct Region *r;
{
    struct RegionRectangle *rr;

    printf("||%s||\n", s);
    dumpRect("\tbounds", &r->bounds);
    for (rr = r->RegionRectangle; rr != NULL; rr = rr->Next)
    {
	dumpRect(" -", &rr->bounds);
    }
    printf("\n");
}
#endif

sinfo(si)
struct StringInfo *si;
{
    printf("string Pos=%ld Num=%ld Max=%ld DPos=%ld DCount=%ld ",
	si->BufferPos, si->NumChars, si->MaxChars, si->DispPos, si->DispCount);
    printf("\"%s\"\n", si->Buffer);
}

dumpSex(s, sex)
UBYTE *s;
struct StringExtend *sex;
{
    printf("<%s at %lx>\n", s, sex);
    printf("font: %lx, pens: %ld %ld, activepens: %ld %ld\n",
	sex->Font, sex->Pens[0], sex->Pens[1],
	sex->ActivePens[0], sex->ActivePens[1]);
    printf("InitialModes: %lx\n", sex->InitialModes);
    printf("EditHook: %lx, WorkBuffer: %lx\n", sex->EditHook, sex->WorkBuffer);
    printf("\n");

}

dumpTExtent( str, te )
char	*str;
struct TextExtent *te;
{
     printf( "dTE: %s\n", str );   
     printf("width/height %ld/%ld\n", te->te_Width, te->te_Height);
     dumpRect("terect\n", &te->te_Extent );
}

dumpInputEvent(ie)
struct InputEvent *ie;
{
    printf("IE at %lx, next at %lx\n", ie, ie->ie_NextEvent);
    printf("class %lx, subclass %lx, code %lx\n", ie->ie_Class,
	ie->ie_SubClass, ie->ie_Code);
    printf("qualifier %lx\n", ie->ie_Qualifier);
    printf("x/y %lx/%lx\n",
	ie->ie_position.ie_xy.ie_x, ie->ie_position.ie_xy.ie_y);
    printf("addr %lx\n", ie->ie_position.ie_addr);
}

dumpIText( str, it )
char *str;
struct IntuiText *it;
{
    printf("\nITEXT: %s\n", str);
    while (it)
    {
	printf("\t(0x%lx)\tl/t: [%ld, %ld]\t", it, it->LeftEdge, it->TopEdge);
	printf("\ttext: %s\n", it->IText);
	it = it->NextText;
    }

}

dumpRemember(key)
struct Remember *key;
{
  while (key)
  {
printf("key %lx size %ld loc %lx\n", key, key->RememberSize, key->Memory);
    key = key->NextRemember;
  }
}

dumpLocks( str )
UBYTE	*str;
{
    struct IntuitionBase	*IBase = fetchIBase();
    struct SignalSemaphore	*sem;
    int	semnum;
    struct	Task	*mytask, *FindTask();

    mytask = FindTask(0);

    printf("%s  Holding locks:", str );

    for ( semnum = 0; semnum < NUMILOCKS; ++semnum )
    {
	if (mytask == IBase->ISemaphore[ semnum ].ss_Owner)
	{
	    printf( "%ld ", semnum );
	}
    }
    printf("\n");
}

dumpGI( str, gi )
UBYTE	*str;
struct GadgetInfo *gi;
{
    printf("%s  GInfo at %lx:\n", str, gi );
    printf("screen/window/requester: %lx/%lx/%lx\n", gi->gi_Screen,
	gi->gi_Window, gi->gi_Requester );
    printf("rport/layer %lx/%lx\n", gi->gi_RastPort, gi->gi_Layer );
    printf("draw info %lx\n", gi->gi_DrInfo );
    dumpBox( "Domain", &gi->gi_Domain );

}

dumpGE( str, ge )
UBYTE	*str;
struct GListEnv *ge;
{
    printf("%s  GListEnv:\n", str );
    dumpGI( " ", &ge->ge_GInfo );
    printf("---- GListEnv fields\n");
    printf("layer/gzzlayer %lx/%lx\n", ge->ge_Layer, ge->ge_GZZLayer );
    dumpBox( "normal domain", &ge->ge_Domain );
    dumpBox( "gzz domain", &ge->ge_GZZdims );
}

dumpHook( str, hook )
UBYTE	*str;
struct Hook	*hook;
{
    printf("%s.  hook at %lx\n", str, hook );
    printf("data %lx, entry %lx, subentry %lx, data %lx\n", 
	hook->h_Data,
	hook->h_Entry,
	hook->h_SubEntry,
	hook->h_Data );
}

dumpBitMap( str, bm )
UBYTE	*str;
struct BitMap	*bm;
{
    int	plane;

    printf("%s -- bitmap at %lx\n", str, bm );
    printf("bpr %ld, rows %ld, depth %ld\n",
	bm->BytesPerRow, bm->Rows, bm->Depth );
    printf("flags %lx pad %lx\n", bm->Flags, bm->pad );
    for ( plane = 0; plane < bm->Depth; plane++ )
	printf("bitplane %ld at %lx\n", plane, bm->Planes[plane] );
}
    
dumpCRect( str, cr )
UBYTE	*str;
struct ClipRect	*cr;
{
    printf("%s -- cliprect at %lx\n", str, cr );
    printf("next %lx prev %lx\n", cr->Next, cr->prev );
    printf("layer %lx\n", cr->lobs );
    printf("reserved cr ptrs %lx %lx\n", cr->_p1, cr->_p2 );
    printf("reserved %lx\n", cr->reserved );
    dumpRect("cliprect bounds", &cr->bounds );
    dumpBitMap("cliprect", cr->BitMap );
}

struct TagItem	*map;
dumpTagItems( str, ti )
UBYTE	*str;
struct TagItem	*ti;
{
    struct TagItem	*tstate = ti;
    struct TagItem	*NextTagItem();

    if ( str ) printf("%s >>\n", str );

    while ( ti = NextTagItem( &tstate ) )
    {
	kprintf("%8lx--%8lx\n", ti->ti_Tag, ti->ti_Data );
    }
}

dumpDrawInfo( str, dri )
UBYTE	*str;
struct DrawInfo	*dri;
{
    int pen;

    if ( str ) printf("%s\n", str );
    printf("DrawInfo at %lx\n", dri );
    printf("version %ld, numpens %ld, depth %ld\n",
	dri->dri_Version, dri->dri_NumPens, dri->dri_Depth );
    printf("font at %lx\n", dri->dri_Font );
    printf("resolution: %ld/%ld\n",
	dri->dri_Resolution.X, dri->dri_Resolution.Y );
    printf("flags %lx\n", dri->dri_Flags );

    for ( pen = 0; pen < NUMDRIPENS; ++pen )
    {
	printf("\tpen %ld: %ld\n", pen, dri->dri_Pens[ pen ] );
    }
}
