*******************************************************************************
*
*   $Id: ScrollRaster.asm,v 42.0 93/06/16 11:12:40 chrisg Exp $
*
*******************************************************************************

******* graphics.library/ScrollRaster ***************************************
*
*   NAME
*   ScrollRaster -- Push bits in rectangle in raster around by
*                   dx,dy towards 0,0 inside rectangle.
*   SYNOPSIS
*   ScrollRaster(rp, dx, dy, xmin, ymin, xmax, ymax)
*                A1  D0  D1  D2    D3    D4    D5
*
*   void ScrollRaster
*        (struct RastPort *, WORD, WORD, WORD, WORD, WORD, WORD);
*
*   FUNCTION
*   Move the bits in the raster by (dx,dy) towards (0,0)
*   The space vacated is RectFilled with BGPen.
*   Limit the scroll operation to the rectangle defined
*   by (xmin,ymin)(xmax,ymax). Bits outside will not be
*   affected. If xmax,ymax is outside the rastport then use
*   the lower right corner of the rastport.
*   If you are dealing with a SimpleRefresh layered RastPort you
*   should check rp->Layer->Flags & LAYERREFRESH to see if
*   there is any damage in the damage list.  If there is you should
*   call the appropriate BeginRefresh(Intuition) or BeginUpdate(graphics)
*   routine sequence.
*
*   INPUTS
*   rp - pointer to a RastPort structure
*   dx,dy are integers that may be positive, zero, or negative
*   xmin,ymin - upper left of bounding rectangle
*   xmax,ymax - lower right of bounding rectangle
*
*   EXAMPLE
*   ScrollRaster(rp,0,1,minx,miny,maxx,maxy) /* shift raster up by one row */
*   ScrollRaster(rp,-1,-1,minx,miny,maxx,maxy)
*		  /* shift raster down and to the right by 1 pixel
*
*   BUGS
*   In 1.2/V1.3 if you ScrollRaster a SUPERBITMAP exactly left or 
*   right, and there is no TmpRas attached to the RastPort, the system
*   will allocate one for you, but will never free it or record its 
*   location. This bug has been fixed for V36.  The workaround for
*   1.2/1.3 is to attach a valid TmpRas of size at least 
*   MAXBYTESPERROW to the RastPort before the call.
*
*   Beginning with V36 ScrollRaster adds the shifted areas into the 
*   damage list for SIMPLE_REFRESH windows. Due to unacceptable 
*   system overhead, the decision was made NOT to propagate this 
*   shifted area damage for SMART_REFRESH windows.
*
*   SEE ALSO
*   ScrollRasterBF() graphics/rastport.h
*
*********************************************************************

******* graphics.library/ScrollRasterBF *************************************
*
*   NAME
*   ScrollRasterBF -- Push bits in rectangle in raster around by
*                   dx,dy towards 0,0 inside rectangle. Newly empty areas
*   		will be filled via EraseRect(). (V39)
*
*   SYNOPSIS
*   ScrollRasterBF(rp, dx, dy, xmin, ymin, xmax, ymax)
*                A1  D0  D1  D2    D3    D4    D5
*
*   void ScrollRasterBF
*        (struct RastPort *, WORD, WORD, WORD, WORD, WORD, WORD);
*
*   FUNCTION
*   Move the bits in the raster by (dx,dy) towards (0,0)
*   The space vacated is filled by calling EraseRect().
*   Limit the scroll operation to the rectangle defined
*   by (xmin,ymin)(xmax,ymax). Bits outside will not be
*   affected. If xmax,ymax is outside the rastport then use
*   the lower right corner of the rastport.
*   If you are dealing with a SimpleRefresh layered RastPort you
*   should check rp->Layer->Flags & LAYERREFRESH to see if
*   there is any damage in the damage list.  If there is you should
*   call the appropriate BeginRefresh(Intuition) or BeginUpdate(graphics)
*   routine sequence.
*
*   INPUTS
*   rp - pointer to a RastPort structure
*   dx,dy are integers that may be positive, zero, or negative
*   xmin,ymin - upper left of bounding rectangle
*   xmax,ymax - lower right of bounding rectangle
*
*   NOTES
*
*   This call is exactly the same as ScrollRaster, except that it calls
*   EraseRect() instead of RectFill() when clearing the newly exposed
*   area. This allows use of a custom layer backfill hook.
*
*   BUGS
*
*   SEE ALSO
*   ScrollRaster() EraseRect() intuition.library/ScrollWindowRaster() 
*   graphics/rastport.h
*
*********************************************************************


	end
