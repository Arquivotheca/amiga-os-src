drawImageGrunt(rport, image, xoffset, yoffset, minterm )
struct RastPort *rport;
struct Image *image;
int xoffset, yoffset;
{
    struct BitMap BMap;
    int	plane;
    unsigned int planesize;
    UBYTE	mask;
    UWORD	*planedata;
    UWORD	umuls();

    /* jimm: 5/23/90: sanity check on image dims used to
     * be done "implicitly" by AllocMem() of planeonoff planes.
     * Now we must be explicit for compatibility.
     */
    if ( image->Height <= 0 || image->Width <= 0 ) return;

    /* initialize a bitmap */
    InitBitMap( &BMap, rport->BitMap->Depth, image->Width, image->Height );

    mask = ( 1 << 0 );	/* identifies plane 0 */

    planedata = image->ImageData;
    planesize = umuls(BMap.BytesPerRow, BMap.Rows) / 2;	   /* in words */

    /* init plane pointers to one of: data, all ones, all zeroes */
    for ( plane = 0, mask = 1; plane < BMap.Depth; ++plane, mask <<= 1 )
    {
	if ( mask & image->PlanePick )
	{
	    BMap.Planes[ plane ] = planedata;
	    planedata += planesize;
	}
	else if ( mask & image->PlaneOnOff )
	{
	    BMap.Planes[ plane ] = (PLANEPTR) -1;
	}
	else
	{
	    BMap.Planes[ plane ] = NULL;
	}
    }


    BltBitMapRastPort(&BMap, 0, 0, rport, xoffset + image->LeftEdge,
	    yoffset + image->TopEdge, image->Width, image->Height, minterm );
}
