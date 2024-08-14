/* -----------------------------------------------------------------------
 * video.c		handshake_src
 *
 * $Locker:  $
 *
 * $Id: video.c,v 1.1 91/05/09 15:40:47 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/RCS/video.c,v 1.1 91/05/09 15:40:47 bj Exp $
 *
 * $Log:	video.c,v $
 * Revision 1.1  91/05/09  15:40:47  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

/*

        VIDEO.C - Routines to drive Amiga Window

*/

#include "termall.h"

static unsigned short int cursor_on = 0;

/***
*
* Set up the Window
*
***/
int MakeVtWindow ( lines )
unsigned short int lines;
  {
    /***
    *
    * Open up our screen and window. 
    *
    ***/
    
    if ( !ScreenSafe () )
        return ( FALSE );
    
    NewScreen.Height = lines;
    if ( lines > 300 || nvmodes.decinlm == 1 )
        NewScreen.ViewModes |= LACE;
    else
        NewScreen.ViewModes &= ~LACE;
        
    Screen = (struct Screen *) OpenScreen ( &NewScreen );
    if ( Screen == NULL )
        return ( FALSE );
    
    NewWindow.Screen = Screen;
    NewWindow.Height = NewScreen.Height;
    Window = (struct Window *) OpenWindow ( &NewWindow );
    if ( Window == NULL )
      {
        CloseScreen ( Screen );
        return ( FALSE );
      }
        
    RPort = Window->RPort;
    VPort = ViewPortAddress ( Window );
    SaveOrigFgBg ();
    
    SetAPen ( RPort, (long)vcb.fgcolor );
    SetBPen ( RPort, (long)vcb.bgcolor );
    
    return ( TRUE );
  }

/****************************************************************************/

unsigned short int ScreenSafe ()
  {
    struct Window *windowptr;
   
    if ( !Screen || !Window )
        return ( 1 );
        
    for ( windowptr = Screen->FirstWindow;
          windowptr;
          windowptr = windowptr->NextWindow )
        if ( windowptr != Window )
          {
            Error ( "Cannot do this with open windows" );
            break;
          }
    return ( (unsigned short int) ( windowptr == NULL ) );
  }

/****************************************************************************/

void ConfigureScreen ()
  {
    NewScreen.Depth = nvmodes.bitplane;
    switch ( nvmodes.decinlm )
     {
       case 0 : /* OFF */
           SetScreen24 ();
           GfxBase->system_bplcon0 &= ~LACE;
           MoveScreen ( Screen, 0, - Screen->TopEdge );
           break;
       case 1 : /* Half Screen  */
           SetScreen24 ();
           MoveScreen ( Screen, 0, - Screen->TopEdge );
           MoveScreen ( Screen, 0,  200 );
           break;
       case 2 : /* Full Screen  */
           SetScreen24 ();
           MoveScreen ( Screen, 0, - Screen->TopEdge );
           GfxBase->system_bplcon0 |=  LACE;
           break;
       case 3 :
           SetScreen48 ();
           break;
     }

    SetAPen ( RPort, (long)vcb.fgcolor );
    SetBPen ( RPort, (long)vcb.bgcolor );
    
    RemakeDisplay ();
    ScreenToFront ( Screen );
  }
    
/****************************************************************************/

void DrawLogo ()
  {
    RemoveCursor ();
    Vcls ( 1, vcb.screenlen );
    SetFont ( RPort, dwsgcs40font );
    Move    ( RPort, 48,  38 );
    Text    ( RPort, "lqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqk",34 );
    Move    ( RPort, 48,  46 );
    Text    ( RPort, "x",1 );
    Move    ( RPort, 576, 46 );
    Text    ( RPort, "x",1 );
    Move    ( RPort, 48,  54 );
    Text    ( RPort, "x",1 );
    Move    ( RPort, 576, 54 );
    Text    ( RPort, "x",1 );
    Move    ( RPort, 48,  62 );
    Text    ( RPort, "x",1 );
    Move    ( RPort, 576, 62 );
    Text    ( RPort, "x",1 );
    Move    ( RPort, 48,  70 );
    Text    ( RPort, "x",1 );
    Move    ( RPort, 576, 70 );
    Text    ( RPort, "x",1 );
    Move    ( RPort, 48,  78 );
    Text    ( RPort, "tqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqu",34 );
    Move    ( RPort, 48,  86 );
    Text    ( RPort, "x",1 );
    Move    ( RPort, 576, 86 );
    Text    ( RPort, "x",1 );
    Move    ( RPort, 48,  94 );
    Text    ( RPort, "x",1 );
    Move    ( RPort, 576, 94 );
    Text    ( RPort, "x",1 );
    Move    ( RPort, 48, 102 );
    Text    ( RPort, "x",1 );
    Move    ( RPort, 576,102 );
    Text    ( RPort, "x",1 );
    Move    ( RPort, 48, 110 );
    Text    ( RPort, "mqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqj",34 );
    SetDrMd ( RPort, JAM2 | INVERSVID );
    Move    ( RPort, 176, 46 );
    SetFont ( RPort, topdh40font );
    Text    ( RPort, "                   ", 19 );
    Move    ( RPort, 176, 54 );
    Text    ( RPort, " H a n d s h a k e ", 19 );
    Move    ( RPort, 176, 62 );
    SetFont ( RPort, botdh40font );
    Text    ( RPort, " H a n d s h a k e ", 19 );
    Move    ( RPort, 176, 70 );
    Text    ( RPort, "                   ", 19 );
    SetFont ( RPort, dw40font );
    SetDrMd ( RPort, JAM2 );
    Move    ( RPort, 80, 86 );
    Text    ( RPort, "SHAREWARE by Eric Haberfellner", 30 );
    Move    ( RPort, 96, 94 );
    Text    ( RPort, "See the About... item in the", 28 );
    Move    ( RPort, 224, 102 );
    Text    ( RPort, "PROJECT Menu", 12 );
    SetFont ( RPort, Normal80 );
    SetDrMd ( RPort, JAM2 );
    vcb.ln          = 15;
    vcb.col         = 1;
    vcb.cursor_ln   = 15;
    vcb.cursor_col  = 1;
    Vsetcup ();
    cursor_on = 0;
    PlaceCursor ();
  }

/****************************************************************************/

void Beep ()
  {
    MakeBeep ( 750, 32 );
  }

/****************************************************************************/

void VCBInit ( rows, cols )
unsigned short int  rows;
unsigned short int  cols;
  {
    struct ViewPort *ViewPortAddress ();

    NormalFont = Normal80;
    UKFont     = Normal80;
    DSCSFont   = dscs80font;
    SGCSFont   = ansisgcsfont;
    
    if ( nvmodes.ukset == SET )
        g[0]   = &UKFont;
    else
        g[0]   = &NormalFont;
        
    if ( nvmodes.decanm < VT200_7 )
      {
        g[1]   = &SGCSFont;
        g[2]   = &Normal80;
      }
    else
      {
        g[1]   = &SGCSFont;
        g[2]   = &DSCSFont;
      }
    g[3]       = &Normal80;

    Move ( RPort, 0, TOP_OFFSET + 6 );
    vcb.ln          = 1;
    vcb.col         = 1;
    vcb.linelen     = cols;
    vcb.screenlen   = rows;
    vcb.tm          = 1;
    vcb.stm         = 1;
    vcb.bm          = vcb.screenlen;
    vcb.sbm         = vcb.screenlen;
    vcb.bgcolor     = 0;
    vcb.fgcolor     = 1;
    Vsetcup ();
    TabSet ();
    
  }

/****************************************************************************/

void VsetLAttr ( attr )
register int    attr;
  {
    unsigned char *scrptr,
                  *atrptr,
                  tea,
                  tfs,
                  tfont;
    unsigned short int col,
                       lwidth,
                       i;

    if ( attr  < '3' || attr > '6' )
        return;

    LineAttr [ vcb.ln - 1 ] = attr - '0';
    col = (unsigned short int) vcb.col;
    vcb.col = 1;
    Vsetcup ();
    lwidth = FontSize ();
    switch ( lwidth )
      {
        case 5:
            lwidth = 132;
            break;
        case 8:
            lwidth = 80;
            break;
        case 10:
            lwidth = 66;
            break;
        case 16:
            lwidth = 40;
            break;
      }
    
    tfont = tmodes.font;
    tea   = tmodes.eraseattr;
    tfs   = tmodes.fontstyle;
    scrptr = ScreenImage + ((vcb.ln-1) * 132 + (vcb.col-1));
    atrptr = CharAttr    + ((vcb.ln-1) * 132 + (vcb.col-1));
    tmodes.font      = -1;
    tmodes.fontstyle = -1;
    for ( i = 0; i < lwidth; i++ )
      {
        tmodes.eraseattr = *atrptr & 0x80;
        if ( ( tmodes.fontstyle != (*atrptr & 0x78) ) ||
             ( tmodes.font      != (*atrptr & 0x07) ) )
          {
            tmodes.fontstyle = *atrptr & 0x78;
            tmodes.font      = *atrptr & 0x07;
            switch ( tmodes.font )
              {
                case FASCII:
                    ChangeFont ( NormalFont );
                    break;
                case FUK:
                    ChangeFont ( UKFont );
                    break;
                case FSGCS:
                    ChangeFont ( SGCSFont );
                    break;
                case FDSCS:
                    ChangeFont ( DSCSFont );
                    break;
                case FSOFT:
                    ChangeFont ( SoftFont );
              }
          }
        Vtty (*scrptr++);
        atrptr++;
      }
    tmodes.font = tfont;
    tmodes.eraseattr = tea;
    tmodes.fontstyle = tfs;
    vcb.col = col;
    Vsetcup ();
    SetFontAttributes ();
  }

/****************************************************************************/

void VsetCAttr ( ch )
unsigned char ch;
  {
    ScreenImage[(vcb.ln - 1) * 132 + (vcb.col - 1) ] = ch;
    CharAttr   [(vcb.ln - 1) * 132 + (vcb.col - 1) ] =
      tmodes.font | tmodes.fontstyle | tmodes.eraseattr;
    /*
    dispattr ( tmodes.font | tmodes.fontstyle | tmodes.eraseattr );
    */
  }

/****************************************************************************/

void VscrollUpAttr ( rows, top, bottom )
short int rows;
short int top;
short int bottom;
  {
    unsigned short int i;
    
    if ( rows < 0 )
      {
        VscrollDownAttr ( (short)-rows, top, bottom );
        return;
      }
      
    if ( rows > bottom - top + 1 )
        rows = bottom - top + 1;
        
    memcpy ( CharAttr + (top * 132),
             CharAttr + ((top+rows) * 132),
             max ( 0, 132 * (bottom - (top + rows) + 1) ) );
    memset ( CharAttr + ((bottom - rows + 1) * 132),
             FASCII, 132 * rows ); 
    
    memcpy ( ScreenImage + (top * 132),
             ScreenImage + ((top+rows) * 132),
             max ( 0, 132 * (bottom - (top + rows) + 1) ) );
    memset ( ScreenImage + ((bottom - rows + 1) * 132),
             ' ', 132 * rows );
    
    memcpy ( &LineAttr [ top ],
             &LineAttr [ top + rows ],
             max ( 0, bottom - (top+rows) + 1) );

    for (  i = 0; i < rows; i++ )
        LineAttr [ bottom - rows + 1 + i ] = SWSH;
  }

/****************************************************************************/

void VscrollDownAttr ( rows, top, bottom )
short int rows;
short int top;
short int bottom;
  {
    unsigned short int i;
    
    if ( rows < 0 )
      {
        VscrollUpAttr ( (short)-rows, top, bottom );
        return;
      }
      
    if ( rows > bottom - top + 1 )
        rows = bottom - top + 1;
        
    memcpy ( CharAttr + ((top+rows) * 132),
             CharAttr + (top * 132),
             max ( 0, 132 * (bottom - (top+rows) + 1) ) );
    memset ( CharAttr + (top * 132), FASCII, 132 * rows );
    
    
    memcpy ( ScreenImage + ((top+rows) * 132),
             ScreenImage + (top * 132),
             max ( 0, 132 * (bottom - (top+rows) + 1) ) );
    memset ( ScreenImage + (top * 132), ' ', 132 * rows );
    
    
    memcpy ( &LineAttr [ top + rows ],
             &LineAttr [ top ],
             max ( 0, bottom - (top+rows) + 1 ) );

    for (  i = 0; i < rows; i++ )
        LineAttr [ top + i ] = SWSH;
  }

/****************************************************************************/

void VscrollRightAttr ( cols )
short int cols;
  {
    if ( cols < 0 )
      {
        VscrollLeftAttr ( (short)-cols );
        return;
      }
      
    memcpy ( CharAttr + ((vcb.ln - 1) * 132 + (vcb.col - 1 + cols )),
             CharAttr + ((vcb.ln - 1) * 132 + (vcb.col - 1)),
             max( 0, vcb.linelen - ((vcb.col- 1) + cols)));
    memset ( CharAttr + ((vcb.ln - 1) * 132 + (vcb.col - 1)), FASCII, cols );

    memcpy ( ScreenImage + ((vcb.ln - 1) * 132 + (vcb.col - 1 + cols )),
             ScreenImage + ((vcb.ln - 1) * 132 + (vcb.col - 1)),
             max( 0, vcb.linelen - ((vcb.col- 1) + cols)));
    memset ( ScreenImage + ((vcb.ln - 1) * 132 + (vcb.col - 1)), ' ', cols );

  }

/****************************************************************************/

void VscrollLeftAttr ( cols )
short int cols;
  {
    if ( cols < 0 )
      {
        VscrollRightAttr ( (short)-cols );
        return;
      }
      
    memcpy ( CharAttr + ((vcb.ln - 1) * 132 + (vcb.col - 1)),
             CharAttr + ((vcb.ln - 1) * 132 + (vcb.col - 1 + cols)),
             max( 0, vcb.linelen - cols));
    memset ( CharAttr + ((vcb.ln - 1) * 132 + (vcb.linelen - cols)),
             FASCII, cols );

    memcpy ( ScreenImage + ((vcb.ln - 1) * 132 + (vcb.col - 1)),
             ScreenImage + ((vcb.ln - 1) * 132 + (vcb.col - 1 + cols)),
             max( 0, vcb.linelen - cols));
    memset ( ScreenImage + ((vcb.ln - 1) * 132 + (vcb.linelen - cols)),
             ' ', cols );
  }


/****************************************************************************/

/*
void dispattr ( atr )
unsigned char atr;
  {
    unsigned short  old_ln,
                    old_col;
    struct TextFont *font;
    unsigned char   dbuf[20];
                   
    old_ln = vcb.ln;
    old_col= vcb.col;
    vcb.ln = 23;
    vcb.col= 20;
    font = CurrentFont;
    ChangeFont ( NormalFont );
    Vsetcup ();
    sprintf ( dbuf,"%08lx", (int)atr );
    Text ( RPort, dbuf, (unsigned long) strlen ( dbuf ) );
    vcb.ln = old_ln;
    vcb.col= old_col;
    ChangeFont ( font );
    Vsetcup ();
  }
*/

/****************************************************************************/

void Vsetcup ()
  {
    unsigned short int          FontSize ();
    register unsigned short int fs,
                                attr_idx = LineAttr[ vcb.ln - 1 ] - 3;

    if ( scr_out_count )
      {
        Text ( RPort, scr_out_buffer, (unsigned long) scr_out_count );
        scr_out_count = 0;
      }

    fs = FontSize ();
    if ( vcb.linelen == 80 )
      {
        UKFont     =
        NormalFont = *NormFontTab [ attr_idx ];
        SGCSFont   = *SGCSFontTab [ attr_idx ];
        DSCSFont   = *DSCSFontTab [ attr_idx ];
        SoftFont   = *SoftFontTab [ attr_idx ];
      }
    else
      {
        UKFont     =
        NormalFont = *NormFontTab [ attr_idx + 4];
        SGCSFont   = *SGCSFontTab [ attr_idx + 4];
        DSCSFont   = *DSCSFontTab [ attr_idx + 4];
        SoftFont   = *SoftFontTab [ attr_idx + 4];
      }
    if ( fs == 16 && vcb.col > 40 )
        vcb.col = 40;
    else if ( fs == 10 && vcb.col > 66 )
        vcb.col = 66;

    Move ( RPort, ( vcb.col - 1L ) * (long)fs, (vcb.ln - 1L ) * 8L + TOP_OFFSET + 6L );
  }

/****************************************************************************/

unsigned short int FontSize ()
  {
    register unsigned short int fs;

    if ( vcb.linelen == 80 )
      {
        fs = ( LineAttr [ vcb.ln - 1 ] == SWSH ) ? 8 : 16;  
      }
    else
      {
        fs = ( LineAttr [ vcb.ln - 1 ] == SWSH ) ? 5 : 10;  
      }
    return ( fs );
  }

/****************************************************************************/

void PlaceCursor ()
 {
    unsigned short int FontSize ();

    register unsigned short int fs;
    register unsigned short int c_offset,
                                c_height,
                                planesflag;
    register unsigned short int col,
                                rp_c_offset,
                                rp_r_offset;

    if (  cursor_on || tmodes.dectcem == RESET )
        return;

    cursor_on++;

    if ( nvmodes.ctype & 1 )
      {
        c_offset = 6;
        c_height = 2;
      }
    else
      {
        c_offset = 0;
        c_height = 8;
      }

    fs = FontSize ();  

    col = ( ( vcb.col > vcb.linelen ) ? vcb.linelen : vcb.col ) - 1;
    rp_c_offset = col * fs;
    rp_r_offset = ( vcb.ln  - 1 ) * 8 + TOP_OFFSET + c_offset;

    if ( nvmodes.ctype & 0x02L )
        planesflag = ( Screen->BitMap.Depth == 2 ) ? 0x02L : 0x0eL;
      
    planesflag |= 0x01L;

    BltBitMap ( RPort->BitMap,
               (long) rp_c_offset,
               (long) rp_r_offset,
               RPort->BitMap,
               (long) rp_c_offset,
               (long) rp_r_offset,
               (long) fs, (long) c_height, 0x50L,
               (long) planesflag, 0L
              );
    vcb.cursor_ln  = vcb.ln;
    vcb.cursor_col = ++col;
    vcb.cursor_fs  = fs;
 }

/****************************************************************************/

void RemoveCursor ()
 {
    register unsigned int       c_offset,
                                c_height,
                                planesflag;
    register unsigned short int rp_c_offset,
                                rp_r_offset;


    if ( !cursor_on )
        return;

    cursor_on--;

    if ( nvmodes.ctype & 1 )
      {
        c_offset = 6;
        c_height = 2;
      }
    else
      {
        c_offset = 0;
        c_height = 8;
      }

    rp_c_offset = ( vcb.cursor_col - 1 ) * vcb.cursor_fs;
    rp_r_offset = ( vcb.cursor_ln  - 1) * 8 + TOP_OFFSET + c_offset;

    if ( nvmodes.ctype & 0x02L )
        planesflag = ( Screen->BitMap.Depth == 2 ) ? 0x02L : 0x0eL;
      
    planesflag |= 0x01L;
    
    BltBitMap ( RPort->BitMap,
               (long) rp_c_offset,
               (long) rp_r_offset,
               RPort->BitMap,
               (long) rp_c_offset,
               (long) rp_r_offset,
               (long) vcb.cursor_fs, (long) c_height, 0x50L,
               (long) planesflag, 0L
              );
 }

/****************************************************************************/

void VwipeEOL ()
{
    memset ( CharAttr    + ((vcb.ln-1) * 132 + (vcb.col-1)),
             FASCII, vcb.linelen - (vcb.col-1) );
    memset ( ScreenImage + ((vcb.ln-1) * 132 + (vcb.col-1)),
             ' ', vcb.linelen - (vcb.col-1) );
    ClearEOL ( RPort );
}

/****************************************************************************/

void VwipeSOL ()
{
    register unsigned short int fs;

    memset ( CharAttr    + ((vcb.ln-1) * 132),
             FASCII, vcb.col );
    memset ( ScreenImage + ((vcb.ln-1) * 132),
             ' ', vcb.col );
             
    fs = FontSize ();
    ScrollRaster ( RPort, 0L,   8L,
                   0L,                  (vcb.ln - 1L) * 8L + TOP_OFFSET,
                   (long) vcb.col * fs, (vcb.ln - 1L) * 8L + TOP_OFFSET + 7L );
}

/****************************************************************************/

void Vcls ( top, bottom )
register int    top,
                bottom;
{
    register int i;

    memset ( CharAttr   + ((top-1) * 132), FASCII | NON_ERASE,
             132 * (bottom-top+1));
    memset ( ScreenImage+ ((top-1) * 132), ' ',132 * (bottom-top+1));
    for ( i = top; i <= bottom; i++ )
        LineAttr[ i - 1 ] = SWSH;

    if ( Screen->BitMap.Depth != 2 )
        WaitTOF ();
        
    ScrollRaster ( RPort, SCR_WIDTH, 0L,
                          0L,   ( top    - 1L ) * 8L + TOP_OFFSET,
                          (SCR_WIDTH - 1L), ( bottom - 1 ) * 8L + TOP_OFFSET + 7L );
}

/****************************************************************************/

void VswipeEOL ()
  {
    unsigned short int col,
                       tfs,
                       tea,
                       tfont,
                       force_cup,
                       force_flush;
    unsigned char      *aptr;
                  
    aptr = CharAttr + ((vcb.ln-1) * 132 + (vcb.col-1));
    col = vcb.col;
    tfs = tmodes.fontstyle;
    tea = tmodes.eraseattr;
    tfont = tmodes.font;
    
    tmodes.fontstyle = GR_NORMAL;
    tmodes.font      = FASCII;
    tmodes.eraseattr = ERASE;
    ChangeFont ( NormalFont );
    Vsetcup ();
    force_cup   = 0;
    force_flush = 0;
    while ( vcb.col <= vcb.linelen )
      {
        if ( !(*aptr & 0x80 ) )
          {
            if ( force_cup )
              {
                Vsetcup ();
                force_cup = 0;
              }
            Vtty ( ' ' );
            force_flush = 1;
          }
        else
          {
            if ( force_flush )
              {
                ScrFlush ();
                force_flush = 0;
              }
            vcb.col++;
            force_cup = 1;
          }
        
        aptr++;
      }
    vcb.col = col;
    tmodes.fontstyle = tfs;
    tmodes.font      = tfont;
    tmodes.eraseattr = tea;
    
    ChangeFont ( NormalFont );
    Vsetcup ();
  }

/****************************************************************************/

void VswipeSOL ()
{
    unsigned short int col,
                       tfs,
                       tea,
                       tfont,
                       force_cup,
                       force_flush;
    unsigned char *aptr;
                  
    col = vcb.col;
    vcb.col = 1;

    tfs = tmodes.fontstyle;
    tea = tmodes.eraseattr;
    tfont = tmodes.font;
    
    tmodes.fontstyle = GR_NORMAL;
    tmodes.font      = FASCII;
    tmodes.eraseattr = ERASE;
    ChangeFont ( NormalFont );
    
    Vsetcup ();
    force_cup   = 0;
    force_flush = 0;
    aptr = CharAttr + ((vcb.ln-1) * 132 + (vcb.col-1));
    while ( vcb.col <= col )
      {
        if ( !(*aptr & 0x80 ) )
          {
            if ( force_cup )
              {
                Vsetcup ();
                force_cup = 0;
              }
            Vtty ( ' ' );
            force_flush = 1;
          }
        else
          {
            if ( force_flush )
              {
                ScrFlush ();
                force_flush = 0;
              }
            vcb.col++;
            force_cup = 1;
          }
        
        aptr++;
      }
    vcb.col = col;
    tmodes.fontstyle = tfs;
    tmodes.font      = tfont;
    tmodes.eraseattr = tea;
    
    ChangeFont ( NormalFont );
    Vsetcup ();
}

/****************************************************************************/

void Vscls ( top, bottom )
register int    top,
                bottom;
{
    register int i;
    unsigned short int ln;

    ln = vcb.ln;
    for ( i = top; i <= bottom; i++ )
      {
        vcb.ln = i;
        Vsetcup ();
        VswipeEOL ();
        VswipeSOL ();
      }
    vcb.ln = ln;
    Vsetcup ();
}

/****************************************************************************/

void VscrollUp ()
{
    register short int   i;

    if ( scr_out_count )
      {
        Text ( RPort, scr_out_buffer, (long) scr_out_count );
        scr_out_count = 0;
      }

    VscrollUpAttr ( 1, (short)(vcb.stm - 1), (short)(vcb.sbm - 1) );
    
    if ( tmodes.decsclm == SET )
        for ( i = 0; i < 8; i++ )
          {
            if ( Screen->BitMap.Depth != 2 )
                WaitTOF ();
        
            ScrollRaster ( RPort, 0L,   1L,
                                  0L,   (vcb.stm - 1L) * 8L + TOP_OFFSET,
                                  (SCR_WIDTH - 1L), (vcb.sbm-1) * 8L + TOP_OFFSET + 7L );
          }
    else
      {
        if ( Screen->BitMap.Depth != 2 )
            WaitTOF ();
        
        ScrollRaster ( RPort, 0L,   8L,
                              0L,   (vcb.stm - 1L ) * 8L + TOP_OFFSET,
                              (SCR_WIDTH - 1L), (vcb.sbm -1L ) * 8L + TOP_OFFSET + 7L);
      }
}

/****************************************************************************/

void VscrollDn ()
{
    register short int   i;

    if ( scr_out_count )
      {
        Text ( RPort, scr_out_buffer, (long) scr_out_count );
        scr_out_count = 0;
      }

    VscrollDownAttr ( 1, (short)(vcb.stm - 1), (short)(vcb. sbm - 1) );
    
    if ( tmodes.decsclm == SET )
        for ( i = 0; i < 8; i++ )
          {
            if ( Screen->BitMap.Depth != 2 )
                WaitTOF ();
        
            ScrollRaster ( RPort, 0L,   -1L,
                                  0L,   (vcb.stm - 1L ) * 8L + TOP_OFFSET,
                                  (SCR_WIDTH - 1L), (vcb.sbm - 1L ) * 8L + TOP_OFFSET + 7L );
          }
    else
      {
        if ( Screen->BitMap.Depth != 2 )
            WaitTOF ();
        
        ScrollRaster ( RPort, 0L,   -8L,
                              0L,   (vcb.stm - 1L ) * 8L + TOP_OFFSET,
                              (SCR_WIDTH - 1L ), (vcb.sbm - 1L ) * 8L + TOP_OFFSET + 7L );
      }
}

/****************************************************************************/

void VtDspCh (c)
register unsigned char  c;
  {
    unsigned short int gset;
    

    if ( !Screen )
        return;

    if ( (c > 0x1f && c < 0x7f) || (c > 0xa0 && c < 0xff) )
      {
        gset = ( c > 0xa0 ) ? grset : glset;
        if (vcb.col == vcb.linelen - 5 && nvmodes.marbell == SET ) Beep ();
        if ( nvmodes.decanm >= VT100 )
          {
            if ( *g[gset] == SGCSFont )
              {
                if ( *g[gset] != CurrentFont && (c & '\x7f' ) > 94 )
                    ChangeFont ( *g[gset] );
                else if ( CurrentFont != NormalFont && (c & '\x7f' ) < 95 )
                    ChangeFont ( NormalFont );
              }
            else if ( *g[gset] != CurrentFont )
                ChangeFont ( *g[gset] );
          }
        else /* VT52 */
          {
            if ( *g[glset] != CurrentFont && c > 96 )
                ChangeFont ( *g[glset] );
            else if ( CurrentFont != NormalFont && c < 97 )
                ChangeFont ( NormalFont );
          }
      }
      
    if ( c == 0x1a || c == 0x18 )
      {
        ScrFlush ();
        ChangeFont ( DSCSFont );
        Vtty ( 0x3f );
      }
    else
        Vtty ( c );
    if ( tmodes.ss2 != -1)
      {
        glset = tmodes.ss2;
        tmodes.ss2 = -1;
      }
    else if ( tmodes.ss3 != -1 )
      {
        glset = tmodes.ss3;
        tmodes.ss3 = -1;
      }
  }

/****************************************************************************/

void Vtty ( c )
register unsigned char c;
  {
    register    VCB *vcb_ptr = &vcb;

    if ( !Screen )
        return;

    if ( (c > 0x1f && c < 0x7f) || (c > 0xa0 && c < 0xff) )
      {
        c &= '\x7f';
        if ((c=='#') && ( g[glset] == &UKFont ))
          {
            c=POUND;
            ChangeFont ( DSCSFont );
          }
        if (tmodes.irm == SET ) InsChar ();
        if (vcb_ptr->col > vcb_ptr->linelen )
          {
            if (nvmodes.decawm==SET)
              {
                if ( tmodes.printing )
                    PrintIt ( vcb.ln, vcb.ln, '\n' );
                if ((vcb_ptr->ln == vcb_ptr->bm) || ( vcb_ptr->ln == vcb_ptr->sbm ))
                  {
                    VscrollUp ();
                    vcb_ptr->ln -= 1;
                  }
                vcb_ptr->col = 1;
                vcb_ptr->ln += 1;
                Vsetcup ();
              }
            else
              {
                vcb_ptr->col = vcb_ptr->linelen;
                Vsetcup ();
              }
          }
        Vputc ( c );
        ++vcb_ptr->col;
        return;
      }
    switch (c)
      {
        case BELL :
            Beep ();
            break;
        case BKSPACE : 
            ScrFlush ();
            if(vcb_ptr->col>1)
                vcb_ptr->col -= 1;
            Vsetcup ();
            break;
        case '\t' :
            ScrFlush ();
            if (vcb_ptr->col < vcb_ptr->linelen )
                Vlocate (vcb_ptr->ln,vcb_ptr->tabs[vcb_ptr->col-1]);
            break;
        case '\n' : 
        case 0x0b :
        case 0x0c :
            if ( tmodes.printing )
                PrintIt ( vcb.ln, vcb.ln, c );
            if ( (vcb_ptr->ln == vcb_ptr->bm) || (vcb_ptr->ln == vcb_ptr->sbm) )
              {
                VscrollUp ();
                vcb_ptr->ln -= 1;
              }
            vcb_ptr->ln += 1;
            if ( nvmodes.lnm == SET ) vcb_ptr->col = 1;
            Vsetcup ();
            break;
        case '\r' :
            Vlocate ( vcb_ptr->ln, 1 );
            break;
        case SO   : 
            glset=1;
            break;
        case SI   : 
            glset=0;
            break;
        case IND  :
            ScrFlush ();
            Ind ();
            break;
        case NEL  :
            ScrFlush ();
            Nel ();
            break;
        case HTS  :
            ScrFlush ();
            Hts ();
            break;
        case RI  :
            ScrFlush ();
            Ri ();
            break;
      }
  }

/****************************************************************************/

void Vlocate ( x, y )
register unsigned   x,
                    y;
  {
    vcb.ln  = x;
    vcb.col = y;
    Vsetcup ();
  }

/****************************************************************************/

void InsChar ()
  {
    unsigned short int FontSize ();
    register unsigned short int fs;

    fs = FontSize ();

    VscrollRightAttr ( 1 );
    
    if ( Screen->BitMap.Depth != 2 )
        WaitTOF ();
        
    ScrollRaster ( RPort, -(long) fs,   0L,
                 (vcb.col - 1L ) * fs,  (vcb.ln - 1L ) * 8L + TOP_OFFSET,
                 vcb.linelen * fs - 1L, (vcb.ln - 1L ) * 8L + TOP_OFFSET + 7L );
  }

/****************************************************************************/

static unsigned short int orig_bgcolor,
                          orig_fgcolor;
void SaveOrigFgBg ()
  {
    orig_bgcolor = GetRGB4 ( VPort->ColorMap, (long)vcb.bgcolor );
    orig_fgcolor = GetRGB4 ( VPort->ColorMap, (long)vcb.fgcolor );
  }

/****************************************************************************/

void SetOrigFgBg ()
  {
    SetRGB4 ( VPort, (long)vcb.fgcolor, ( orig_fgcolor >> 8L ) & 0xfL,
                        ( orig_fgcolor >> 4L ) & 0xfL,
                          orig_fgcolor & 0x0fL );
    SetRGB4 ( VPort, (long)vcb.bgcolor, ( orig_bgcolor >> 8L ) & 0xfL,
                        ( orig_bgcolor >> 4L ) & 0xfL,
                          orig_bgcolor & 0x0fL );
  }

/****************************************************************************/

void SwapFgBg ()
  {
    register unsigned short int bgcolor,
                                fgcolor;

    bgcolor = GetRGB4 ( VPort->ColorMap, 0L );
    fgcolor = GetRGB4 ( VPort->ColorMap, 1L );
    SetRGB4 ( VPort, 0L, (long) fgcolor >> 8,
                      ( fgcolor >> 4L ) & 0xfL,
                        fgcolor & 0x0fL );
    SetRGB4 ( VPort, 1L, (long) bgcolor >> 8,
                      ( bgcolor >> 4 ) & 0xfL,
                        bgcolor & 0x0fL );
  }

/****************************************************************************/

void SetColors ()
  {
    if ( nvmodes.bitplane <= 2 )
      {
        if ( nvmodes.smallmap[0] != nvmodes.smallmap[1] )
          {
            SetRGB4 ( VPort, 0L, (long)nvmodes.smallmap[0] >> 8,
                              (  (long)nvmodes.smallmap[0] >> 4 ) & 0xfL,
                                 (long)nvmodes.smallmap[0] & 0xfL );
            SetRGB4 ( VPort, 1L, (long)nvmodes.smallmap[1] >> 8,
                              (  (long)nvmodes.smallmap[1] >> 4 ) & 0xfL,
                                 (long)nvmodes.smallmap[1] & 0xfL );
            SetRGB4 ( VPort, 2L, (long)nvmodes.smallmap[2] >> 8,
                              (  (long)nvmodes.smallmap[2] >> 4 ) & 0xfL,
                                 (long)nvmodes.smallmap[2] & 0xfL );
            SetAPen ( RPort, (long)(vcb.fgcolor = 1) );
          }
      }
    else
      {
        if ( nvmodes.colormap[0] != nvmodes.colormap[1] )
          {
            SetRGB4 ( VPort, 0L, (long)nvmodes.colormap[0] >> 8,
                              (  (long)nvmodes.colormap[0] >> 4 ) & 0xfL,
                                 (long)nvmodes.colormap[0] & 0xfL );
            SetRGB4 ( VPort, 1L, (long)nvmodes.colormap[1] >> 8,
                              (  (long)nvmodes.colormap[1] >> 4 ) & 0xfL,
                                 (long)nvmodes.colormap[1] & 0xfL );
            SetRGB4 ( VPort, 2L, (long)nvmodes.colormap[2] >> 8,
                              (  (long)nvmodes.colormap[2] >> 4 ) & 0xfL,
                                 (long)nvmodes.colormap[2] & 0xfL );
            SetRGB4 ( VPort, 3L, (long)nvmodes.colormap[3] >> 8,
                              (  (long)nvmodes.colormap[3] >> 4 ) & 0xfL,
                                 (long)nvmodes.colormap[3] & 0xfL );
            SetRGB4 ( VPort, 4L, (long)nvmodes.colormap[4] >> 8,
                              (  (long)nvmodes.colormap[4] >> 4 ) & 0xfL,
                                 (long)nvmodes.colormap[4] & 0xfL );
            SetRGB4 ( VPort, 5L, (long)nvmodes.colormap[5] >> 8,
                              (  (long)nvmodes.colormap[5] >> 4 ) & 0xfL,
                                 (long)nvmodes.colormap[5] & 0xfL );
            SetRGB4 ( VPort, 6L, (long)nvmodes.colormap[6] >> 8,
                              (  (long)nvmodes.colormap[6] >> 4 ) & 0xfL,
                                 (long)nvmodes.colormap[6] & 0xfL );
            SetRGB4 ( VPort, 7L, (long)nvmodes.colormap[7] >> 8,
                              (  (long)nvmodes.colormap[7] >> 4 ) & 0xfL,
                                 (long)nvmodes.colormap[7] & 0xfL );
            SetAPen ( RPort, (long)(vcb.fgcolor = 7) );
          }
      }
  }
  
/****************************************************************************/

void SaveColors ()
  {
    if ( nvmodes.bitplane <= 2 )
      {
        nvmodes.smallmap[0] = GetRGB4 ( VPort->ColorMap, 0L );
        nvmodes.smallmap[1] = GetRGB4 ( VPort->ColorMap, 1L );
        nvmodes.smallmap[2] = GetRGB4 ( VPort->ColorMap, 2L );
      }
    else
      {
        nvmodes.colormap[0] = GetRGB4 ( VPort->ColorMap, 0L );
        nvmodes.colormap[1] = GetRGB4 ( VPort->ColorMap, 1L );
        nvmodes.colormap[2] = GetRGB4 ( VPort->ColorMap, 2L );
        nvmodes.colormap[3] = GetRGB4 ( VPort->ColorMap, 3L );
        nvmodes.colormap[4] = GetRGB4 ( VPort->ColorMap, 4L );
        nvmodes.colormap[5] = GetRGB4 ( VPort->ColorMap, 5L );
        nvmodes.colormap[6] = GetRGB4 ( VPort->ColorMap, 6L );
        nvmodes.colormap[7] = GetRGB4 ( VPort->ColorMap, 7L );
      }
    
  }
  
/****************************************************************************/

void SetScreen24 ()
  {
    static unsigned short int oldinlm = 0;
    struct Window *was_window = Window;
    
    if ( vcb.screenlen != 24  || Screen->BitMap.Depth != NewScreen.Depth || 
         nvmodes.decinlm != oldinlm )
      {
        if ( was_window )
          {
            StopBlinkTask ();
            StopSerialTasks ();
            RemoveCursor ();
            ( ( struct Process * ) main_task )->pr_WindowPtr = window_ptr;
            SetFont ( Window->RPort, 0 );
            CloseWindow ( Window );
            CloseScreen ( Screen );
            Screen = NULL;
            Window = NULL;
          }
        if (!MakeVtWindow ( 204 ) )
          {
            nvmodes.bitplane = 2;
            nvmodes.decinlm  = oldinlm;
            RefreshMenu ();
            NewScreen.Depth  = nvmodes.bitplane;
            if ( !MakeVtWindow ( 204 ) )
              {
                DisplayAlert ( AG_NoMemory, MemMessage, 60L );
                ShutDown ();
              }
            nvmodes.bitplane = 2;
            Error ( "Not enough memory for new screen" );
          }
        VCBInit ( 24, vcb.linelen );
        oldinlm = nvmodes.decinlm;
        if ( was_window )
          {
            StartBlinkTask ();
            StartSerialTasks ();
            SetFont ( Window->RPort, CurrentFont );
          }
        CurrentFont  = 0L;
        RemoveCursor ();
        Vlocate ( 1, 1 );
        
        SetAPen ( RPort, (long)vcb.fgcolor );
        SetBPen ( RPort, (long)vcb.bgcolor );
        
        if ( was_window )
            PlaceCursor ();
            
        ScreenToFront ( Screen );
        
        SetColors ();
      }
  }


/****************************************************************************/

void SetScreen48 ()
  {
    static unsigned short int oldinlm = 0;
    struct Window *was_window = Window;
    
    if ( vcb.screenlen != 48 || Screen->BitMap.Depth != NewScreen.Depth ||
         oldinlm != nvmodes.decinlm )
      {
        if ( was_window )
          {
            StopBlinkTask ();
            StopSerialTasks ();
            RemoveCursor ();
            ( ( struct Process * ) main_task )->pr_WindowPtr = window_ptr;
            SetFont ( Window->RPort, 0 );
            CloseWindow ( Window );
            CloseScreen ( Screen );
            Screen = NULL;
            Window = NULL;
          }
        if (!MakeVtWindow ( 408 ) )
          {
            nvmodes.bitplane = 2;
            nvmodes.decinlm = oldinlm;
            RefreshMenu ();
            NewScreen.Depth = nvmodes.bitplane;
            if ( !MakeVtWindow ( 204 ) )
              {
                DisplayAlert ( AG_NoMemory, MemMessage, 60L );
                ShutDown ();
              }
            VCBInit ( 24, vcb.linelen );
            Error ( "Not enough memory for new screen" );
          }
        else
            VCBInit ( 48, vcb.linelen );
        oldinlm = nvmodes.decinlm;
        if ( was_window )
          {
            StartBlinkTask ();
            StartSerialTasks ();
            SetFont ( Window->RPort, CurrentFont );
          }
        CurrentFont  = 0L;
        RemoveCursor ();
        Vlocate ( 1, 1 );
        
        SetAPen ( RPort, (long)vcb.fgcolor );
        SetBPen ( RPort, (long)vcb.bgcolor );
        
        if ( was_window )
            PlaceCursor ();
            
        ScreenToFront ( Screen );
        SetColors ();
      }
  }

/****************************************************************************/

