/* -----------------------------------------------------------------------
 * fonts.c		handshake_src
 *
 * $Locker:  $
 *
 * $Id: fonts.c,v 1.1 91/05/09 16:19:28 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/RCS/fonts.c,v 1.1 91/05/09 16:19:28 bj Exp $
 *
 * $Log:	fonts.c,v $
 * Revision 1.1  91/05/09  16:19:28  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

/***
*
* Special character fonts
*
***/

#include "termall.h"
#include "exec/alerts.h"

/*
* ASCII TextAttr definitions
*/
static struct TextAttr EightyColumnFont =
  {
    NULL,          /* Font Name    */
    8,             /* Font Height  */
    FS_NORMAL,     /* Font Style   */
    NULL           /* Preferences  */
  };


static unsigned short int   vt52sgcschars[] =
  {
    /*
       1 2    3 4    5 6    7 8    910   1112   1314   1516   1718   1920   2122   2324   2526   2728   2930
    */
    0xff61,0xf1f1,0xf130,0x1800,0x0018,0x18ff,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0xffe3,0x33c3,0x33cc,0x1818,0x0018,0x1800,0xff00,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x00fe,
    0xff66,0xf6f6,0x36cc,0xfe0c,0x0000,0x1800,0x00ff,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0076,
    0xff6c,0x3c3c,0x3c30,0x18fe,0x00fe,0x5a00,0x0000,0xff00,0x0000,0x001e,0x0c1e,0x1e14,0x1e1e,0x1e1e,0x1e76,
    0xfff8,0xf8f8,0x3800,0x18fe,0x00fe,0x7e00,0x0000,0x00ff,0x0000,0x0012,0x1c06,0x0614,0x1810,0x0612,0x1236,
    0xff30,0x3030,0x3000,0x000c,0xdb00,0x3c00,0x0000,0x0000,0xff00,0x0012,0x0c1e,0x1e1e,0x1e1e,0x061e,0x1e36,
    0xff60,0x6060,0x6000,0xfe18,0xdb18,0x1800,0x0000,0x0000,0x00ff,0x0012,0x0c18,0x0604,0x0612,0x0612,0x0236,
    0xffc0,0xc0c0,0xc000,0x0000,0x0018,0x0000,0x0000,0x0000,0x0000,0xff1e,0x1e1e,0x1e04,0x1e1e,0x061e,0x0236
  };

static unsigned short int   vt52sgcscharloc[] =
  {
    0x0000,0x0008,0x0008,0x0008,0x0010,0x0008,0x0018,0x0008,
    0x0020,0x0008,0x0028,0x0008,0x0030,0x0008,0x0038,0x0008,
    0x0040,0x0008,0x0048,0x0008,0x0050,0x0008,0x0058,0x0008,
    0x0060,0x0008,0x0068,0x0008,0x0070,0x0008,0x0078,0x0008,
    0x0080,0x0008,0x0088,0x0008,0x0090,0x0008,0x0098,0x0008,
    0x00a0,0x0008,0x00a8,0x0008,0x00b0,0x0008,0x00b8,0x0008,
    0x00c0,0x0008,0x00c8,0x0008,0x00d0,0x0008,0x00d8,0x0008,
    0x00e0,0x0008,0x00e8,0x0008
  };

static unsigned short int   ansisgcschars[] =
  {
    /*
       1 2    3 4    5 6    7 8    910   1112   1314   1516   1718   1920   2122   2324   2526   2728   2930   3132
    */
    0x0010,0xaa90,0xf070,0x8000,0x0090,0x9018,0x0000,0x1818,0xff00,0x0000,0x0018,0x1818,0x0018,0x0480,0x000e,0x1c00,0xff00,
    0x0038,0x55f0,0x8080,0x8038,0x10f0,0x9018,0x0000,0x1818,0xffff,0x0000,0x0018,0x1818,0x0018,0x0840,0x7a10,0x2600,0x8100,
    0x007c,0xaa90,0xe080,0x806c,0x1090,0x6018,0x0000,0x1818,0x00ff,0x0000,0x0018,0x1818,0x0018,0x1020,0x2cfe,0x7218,0x8100,
    0x00fe,0x5590,0x807e,0xfe44,0xfe90,0x60f8,0xf81f,0x1fff,0x0000,0xff00,0x001f,0xf8ff,0xff18,0x2010,0x2410,0x203c,0x8100,
    0x007c,0xaa3e,0xfe12,0x106c,0x1010,0x3ef8,0xf81f,0x1fff,0x0000,0xff00,0x001f,0xf8ff,0xff18,0x4008,0x24fe,0x603c,0x8100,
    0x0038,0x5508,0x101c,0x1c38,0x1010,0x0800,0x1818,0x0018,0x0000,0x00ff,0x0018,0x1800,0x1818,0xfcfc,0x6c10,0xa218,0x8100,
    0x0010,0xaa08,0x1c14,0x1000,0x0010,0x0800,0x1818,0x0018,0x0000,0x00ff,0xff18,0x1800,0x1818,0x0000,0x6ce0,0xdc00,0x8100,
    0x0000,0x5508,0x1012,0x1000,0xfe1f,0x0800,0x1818,0x0018,0x0000,0x0000,0xff18,0x1800,0x1818,0xfcfc,0x0000,0x0000,0xff00
  };

static unsigned short int   ansisgcscharloc[] =
  {
    0x0000,0x0008,0x0008,0x0008,0x0010,0x0008,0x0018,0x0008,
    0x0020,0x0008,0x0028,0x0008,0x0030,0x0008,0x0038,0x0008,
    0x0040,0x0008,0x0048,0x0008,0x0050,0x0008,0x0058,0x0008,
    0x0060,0x0008,0x0068,0x0008,0x0070,0x0008,0x0078,0x0008,
    0x0080,0x0008,0x0088,0x0008,0x0090,0x0008,0x0098,0x0008,
    0x00a0,0x0008,0x00a8,0x0008,0x00b0,0x0008,0x00b8,0x0008,
    0x00c0,0x0008,0x00c8,0x0008,0x00d0,0x0008,0x00d8,0x0008,
    0x00e0,0x0008,0x00e8,0x0008,0x00f0,0x0008,0x00f8,0x0008
  };

static unsigned short int dscschars    [98 * 8 / sizeof(unsigned short int)];
static unsigned short int dscscharloc  [98 * 2];

static unsigned short int softchars    [98 * 8 / sizeof(unsigned short int)];
static unsigned short int softcharloc  [98 * 2];

void FontInit ()
  {
    ansisgcsfont    = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    dscs80font      = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    soft80font      = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    vt52sgcsfont    = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );

    dw40font        = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    topdh40font     = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    botdh40font     = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    ascii128font    = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    dw64font        = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    topdh64font     = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    botdh64font     = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    
    dwsgcs40font    = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    topdhsgcs40font = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    botdhsgcs40font = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    ansisgcs128font = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    dwsgcs64font    = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    topdhsgcs64font = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    botdhsgcs64font = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    
    dwdscs40font    = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    topdhdscs40font = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    botdhdscs40font = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    dscs128font     = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    dwdscs64font    = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    topdhdscs64font = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    botdhdscs64font = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    
    dwsoft40font    = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    topdhsoft40font = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    botdhsoft40font = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    soft128font     = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    dwsoft64font    = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    topdhsoft64font = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );
    botdhsoft64font = (struct TextFont *) HSAllocMem ( sizeof (struct TextFont),
                                                     MEMF_PUBLIC | MEMF_CLEAR );

    if (
          ! ( ansisgcsfont    && dscs80font      && soft80font      &&
              vt52sgcsfont 
              &&
              dw40font        && topdh40font     && botdh40font     &&
              dw64font        && topdh64font     && botdh64font     &&
              ascii128font    && 
              dwsgcs40font    && topdhsgcs40font && botdhsgcs40font &&
              dwsgcs64font    && topdhsgcs64font && botdhsgcs64font &&
              ansisgcs128font &&
              dwdscs40font    && topdhdscs40font && botdhdscs40font &&
              dwdscs64font    && topdhdscs64font && botdhdscs64font &&
              dscs128font     && 
              dwsoft40font    && topdhsoft40font && botdhsoft40font &&
              dwsoft64font    && topdhsoft64font && botdhsoft64font &&
              soft128font
              )
       )
      {
        DisplayAlert ( AG_NoMemory, MemMessage, 60L );
        ShutDown ();;
      }

    ansisgcsfont->tf_Message.mn_Node.ln_Name =  "ansisgcs.font";
    ansisgcsfont->tf_YSize     = 8;
    ansisgcsfont->tf_Style     = 0;
    ansisgcsfont->tf_Flags     = FPF_DESIGNED | FPF_PROPORTIONAL;
    ansisgcsfont->tf_XSize     = 8;
    ansisgcsfont->tf_Baseline  = 6;
    ansisgcsfont->tf_BoldSmear = 1;
    ansisgcsfont->tf_Accessors = 0;
    ansisgcsfont->tf_LoChar    = 95;
    ansisgcsfont->tf_HiChar    = 126;
    ansisgcsfont->tf_CharData  = (APTR) ansisgcschars;
    ansisgcsfont->tf_Modulo    = 34;
    ansisgcsfont->tf_CharLoc   = (APTR) ansisgcscharloc;
    ansisgcsfont->tf_CharSpace = NULL;
    ansisgcsfont->tf_CharKern  = NULL;

    dscs80font->tf_Message.mn_Node.ln_Name =  "dscs80.font";
    dscs80font->tf_YSize     = 8;
    dscs80font->tf_Style     = 0;
    dscs80font->tf_Flags     = FPF_DESIGNED | FPF_PROPORTIONAL;
    dscs80font->tf_XSize     = 8;
    dscs80font->tf_Baseline  = 6;
    dscs80font->tf_BoldSmear = 1;
    dscs80font->tf_Accessors = 0;
    dscs80font->tf_LoChar    = 32;
    dscs80font->tf_HiChar    = 126;
    dscs80font->tf_CharData  = (APTR) dscschars;
    dscs80font->tf_Modulo    = 98;
    dscs80font->tf_CharLoc   = (APTR) dscscharloc;
    dscs80font->tf_CharSpace = NULL;
    dscs80font->tf_CharKern  = NULL;

    soft80font->tf_Message.mn_Node.ln_Name =  "soft80.font";
    soft80font->tf_YSize     = 8;
    soft80font->tf_Style     = 0;
    soft80font->tf_Flags     = FPF_DESIGNED | FPF_PROPORTIONAL;
    soft80font->tf_XSize     = 8;
    soft80font->tf_Baseline  = 6;
    soft80font->tf_BoldSmear = 1;
    soft80font->tf_Accessors = 0;
    soft80font->tf_LoChar    = 32;
    soft80font->tf_HiChar    = 126;
    soft80font->tf_CharData  = (APTR) softchars;
    soft80font->tf_Modulo    = 98;
    soft80font->tf_CharLoc   = (APTR) softcharloc;
    soft80font->tf_CharSpace = NULL;
    soft80font->tf_CharKern  = NULL;

    vt52sgcsfont->tf_Message.mn_Node.ln_Name =  "vt52sgcs.font";
    vt52sgcsfont->tf_YSize     = 8;
    vt52sgcsfont->tf_Style     = 0;
    vt52sgcsfont->tf_Flags     = FPF_DESIGNED | FPF_PROPORTIONAL;
    vt52sgcsfont->tf_XSize     = 8;
    vt52sgcsfont->tf_Baseline  = 6;
    vt52sgcsfont->tf_BoldSmear = 1;
    vt52sgcsfont->tf_Accessors = 0;
    vt52sgcsfont->tf_LoChar    = 97;
    vt52sgcsfont->tf_HiChar    = 126;
    vt52sgcsfont->tf_CharData  = (APTR) vt52sgcschars;
    vt52sgcsfont->tf_Modulo    = 30;
    vt52sgcsfont->tf_CharLoc   = (APTR) vt52sgcscharloc;
    vt52sgcsfont->tf_CharSpace = NULL;
    vt52sgcsfont->tf_CharKern  = NULL;

    EightyColumnFont.ta_Name  = nvmodes.font;
    EightyColumnFont.ta_Flags = FPF_ROMFONT | FPF_DESIGNED;
    Normal80 = OpenFont ( &EightyColumnFont );
    if ( !Normal80 )
      {
        EightyColumnFont.ta_Flags = FPF_DISKFONT | FPF_DESIGNED;
        DiskfontBase = OpenLibrary ( "diskfont.library", 0L );
        Normal80 = OpenDiskFont ( &EightyColumnFont );
        CloseLibrary ( DiskfontBase );
      }


    if ( !Normal80 )
      {
        Error ( "Could not find specified font" );
        Error ( nvmodes.font );
        strcpy ( nvmodes.font, "topaz.font" );
        EightyColumnFont.ta_Name = nvmodes.font;
        EightyColumnFont.ta_Flags = FPF_ROMFONT | FPF_DESIGNED;
        Normal80 = OpenFont ( &EightyColumnFont );
      }
    else if ( Normal80->tf_XSize != 8 || Normal80->tf_YSize != 8 )
      {
        Error ( "Font is not 8x8" );
        CloseFont ( Normal80 );
        strcpy ( nvmodes.font, "topaz.font" );
        EightyColumnFont.ta_Name  = nvmodes.font;
        EightyColumnFont.ta_Flags = FPF_ROMFONT | FPF_DESIGNED;
        Normal80 = OpenFont ( &EightyColumnFont );
      }

    SetFont ( Window->RPort, Normal80 );
    CurrentFont  = Normal80;

    BuildDSCSFont ( dscs80font, Normal80 );
    BuildDSCSFont ( soft80font, Normal80 );
    
    BuildDoubleWidthFont  ( "DW40.font",        dw40font,        Normal80   );
    BuildDoubleHeightFont ( "TopDH40.font",     topdh40font,     dw40font, 0 );
    BuildDoubleHeightFont ( "BotDH40.font",     botdh40font,     dw40font, 4 );
    BuildNarrowWidthFont  ( "ASCII128.font",    ascii128font,    Normal80 );

    BuildDoubleWidthFont  ( "DWSGCS40.font",    dwsgcs40font,    ansisgcsfont );
    BuildDoubleHeightFont ( "TopSGCS40.font",   topdhsgcs40font, dwsgcs40font, 0 );
    BuildDoubleHeightFont ( "BotSGCS40.font",   botdhsgcs40font, dwsgcs40font, 4 );
    BuildNarrowWidthFont  ( "ansisgcs128.font", ansisgcs128font, ansisgcsfont);
    
    BuildDoubleWidthFont  ( "DWDSCS40.font",    dwdscs40font,    dscs80font );
    BuildDoubleHeightFont ( "TopDSCS40.font",   topdhdscs40font, dwdscs40font, 0 );
    BuildDoubleHeightFont ( "BotDSCS40.font",   botdhdscs40font, dwdscs40font, 4 );
    BuildNarrowWidthFont  ( "dscs128.font",     dscs128font,     dscs80font);

    BuildDoubleWidthFont  ( "DWSoft40.font",    dwsoft40font,    soft80font );
    BuildDoubleHeightFont ( "TopSoft40.font",   topdhsoft40font, dwsoft40font, 0 );
    BuildDoubleHeightFont ( "BotSoft40.font",   botdhsoft40font, dwsoft40font, 4 );
    BuildNarrowWidthFont  ( "soft128.font",     soft128font,     soft80font);

    BuildDoubleWidthFont  ( "DW64.font",     dw64font,        ascii128font   );
    BuildDoubleHeightFont ( "TopDH64.font",  topdh64font,     dw64font,     0 );
    BuildDoubleHeightFont ( "BotDH64.font",  botdh64font,     dw64font,     4 );
    
    BuildDoubleWidthFont  ( "DWSGCS64.font", dwsgcs64font,    ansisgcs128font );
    BuildDoubleHeightFont ( "TopSGCS64.font",topdhsgcs64font, dwsgcs64font, 0 );
    BuildDoubleHeightFont ( "BotSGCS64.font",botdhsgcs64font, dwsgcs64font, 4 );
    
    BuildDoubleWidthFont  ( "DWDSCS64.font", dwdscs64font,    dscs128font );
    BuildDoubleHeightFont ( "TopDSCS64.font",topdhdscs64font, dwdscs64font, 0 );
    BuildDoubleHeightFont ( "BotDSCS64.font",botdhdscs64font, dwdscs64font, 4 );

    BuildDoubleWidthFont  ( "DWSoft64.font", dwsoft64font,    soft128font );
    BuildDoubleHeightFont ( "TopSoft64.font",topdhsoft64font, dwsoft64font, 0 );
    BuildDoubleHeightFont ( "BotSoft64.font",botdhsoft64font, dwsoft64font, 4 );

    SetFont ( Window->RPort, Normal80 );
    CurrentFont  = Normal80;
    ClearSoftFont ();
  }

void CloseFonts ()
  {
    if ( Window )
        SetFont ( Window->RPort, 0 );

    if ( Normal80 )
        CloseFont ( Normal80    );
    
    if ( ansisgcsfont )
        HSFreeMem ( (char *) ansisgcsfont, (long) sizeof (struct TextFont) );
    if ( dscs80font )
        HSFreeMem ( (char *) dscs80font,   (long) sizeof (struct TextFont) );
    if ( soft80font )
        HSFreeMem ( (char *) soft80font,   (long) sizeof (struct TextFont) );
    if ( vt52sgcsfont )
        HSFreeMem ( (char *) vt52sgcsfont, (long) sizeof (struct TextFont) );

    FreeFont ( dw40font        );
    FreeFont ( topdh40font     );
    FreeFont ( botdh40font     );
    FreeMainFont ( ascii128font );
    FreeFont ( dw64font        );
    FreeFont ( topdh64font     );
    FreeFont ( botdh64font     );
    
    FreeFont ( dwsgcs40font    );
    FreeFont ( topdhsgcs40font );
    FreeFont ( botdhsgcs40font );
    FreeMainFont ( ansisgcs128font );
    FreeFont ( dwsgcs64font    );
    FreeFont ( topdhsgcs64font );
    FreeFont ( botdhsgcs64font );
    
    FreeFont ( dwdscs40font    );
    FreeFont ( topdhdscs40font );
    FreeFont ( botdhdscs40font );
    FreeMainFont ( dscs128font );
    FreeFont ( dwdscs64font    );
    FreeFont ( topdhdscs64font );
    FreeFont ( botdhdscs64font );

    FreeFont ( dwsoft40font    );
    FreeFont ( topdhsoft40font );
    FreeFont ( botdhsoft40font );
    FreeMainFont ( soft128font );
    FreeFont ( dwsoft64font    );
    FreeFont ( topdhsoft64font );
    FreeFont ( botdhsoft64font );
  }

void FreeMainFont ( infont )
struct TextFont *infont;
  {
    unsigned short int numchars;

    if ( !infont)
        return;
        
    numchars = infont->tf_Modulo;

    if ( infont->tf_CharData  )
        HSFreeMem ( (char *) infont->tf_CharData, (long) numchars * infont->tf_YSize );
    if ( infont->tf_CharLoc   ) 
        HSFreeMem ( (char *) infont->tf_CharLoc,  (long) numchars * 4 );
    if ( infont->tf_CharSpace )
        HSFreeMem ( (char *) infont->tf_CharSpace,(long) numchars * 2);
    if ( infont->tf_CharKern )
        HSFreeMem ( (char *) infont->tf_CharKern, (long) numchars * 2);
    if ( infont )
        HSFreeMem ( (char *) infont, (long) sizeof(struct TextFont) );
  }

void FreeFont ( infont )
struct TextFont *infont;
  {
    unsigned short int numchars;

    if ( !infont )
        return;
        
    numchars = infont->tf_Modulo;

    if ( infont->tf_CharData  )
        HSFreeMem ( (char *) infont->tf_CharData, (long) numchars * infont->tf_YSize );
    if ( infont->tf_CharLoc   )
        HSFreeMem ( (char *) infont->tf_CharLoc,  (long) numchars * 2 );
    if ( infont->tf_CharSpace )
        HSFreeMem ( (char *) infont->tf_CharSpace,(long) numchars );
    if ( infont->tf_CharKern )
        HSFreeMem ( (char *) infont->tf_CharKern, (long) numchars );
    if ( infont )
        HSFreeMem ( (char *) infont, (long) sizeof(struct TextFont) );
  }

void BuildDoubleWidthFont ( name, outfont, infont )
unsigned char               *name;
register struct TextFont    *outfont,
                            *infont;
  {
    register short int          HiChar,
                                numchars,
                                i,j,
                                k;

    register unsigned char      byte,
                                *byteptr;
    register unsigned short int    word,
                                bit,
                                *wordptr,
                                *word2ptr;
    register unsigned long int  *longptr;

    if ( !outfont )
        return;
        
    HiChar    = (infont->tf_HiChar > 126 ) ? 126 : infont->tf_HiChar;
    numchars  = HiChar - infont->tf_LoChar + 1;
    numchars  = (numchars + 1) & 0xfffe;

    outfont->tf_Message.mn_Node.ln_Name =  name;
    outfont->tf_YSize     = infont->tf_YSize;
    outfont->tf_Style     = infont->tf_Style & 0x4f;
    outfont->tf_Flags     = infont->tf_Flags;
    outfont->tf_XSize     = infont->tf_XSize * 2;
    outfont->tf_Baseline  = infont->tf_Baseline;
    outfont->tf_BoldSmear = infont->tf_BoldSmear;
    outfont->tf_Accessors = 0;
    outfont->tf_LoChar    = infont->tf_LoChar;
    outfont->tf_HiChar    = HiChar;
    
    if ( !outfont->tf_CharData )
      {
        outfont->tf_CharData  =
            (APTR) HSAllocMem ( (long) numchars * 2 * infont->tf_YSize,
                              MEMF_CLEAR );
        if ( !outfont->tf_CharData )
          {
            DisplayAlert ( AG_NoMemory, MemMessage, 60L );
            ShutDown ();
          }
      }
      
    outfont->tf_Modulo    = numchars * 2;
    if ( infont->tf_CharLoc && !outfont->tf_CharLoc )
      {
        outfont->tf_CharLoc
        = (APTR) HSAllocMem ( (long) numchars * 4, MEMF_CLEAR );
        if ( !outfont->tf_CharLoc )
          {
            DisplayAlert ( AG_NoMemory, MemMessage, 60L );
            ShutDown ();
          }
      }
      
    if ( infont->tf_CharSpace && !outfont->tf_CharSpace )
      {
        outfont->tf_CharSpace
        = (APTR) HSAllocMem ( (long) numchars * 2, MEMF_CLEAR );
        if ( !outfont->tf_CharSpace )
          {
            DisplayAlert ( AG_NoMemory, MemMessage, 60L );
            ShutDown ();
          }
      }
      
    if ( infont->tf_CharKern && !outfont->tf_CharKern )
      {
        outfont->tf_CharKern
        = (APTR) HSAllocMem ( (long) numchars * 2, MEMF_CLEAR );
        if ( !outfont->tf_CharKern )
          {
            DisplayAlert ( AG_NoMemory, MemMessage, 60L );
            ShutDown ();
          }
      }

    byteptr = (unsigned char *)       infont->tf_CharData;
    wordptr = (unsigned short int * ) outfont->tf_CharData;

    for ( i = 0; i < numchars; i++ )
        for ( j = 0; j < infont->tf_YSize; j++ )
          {
            word = 0;
            byte = byteptr[ j * infont->tf_Modulo + i ];
            for ( k = 0; k < 8; k++ )
              {
                bit  = ( byte & 0x80 ) ? 1 : 0;
                word <<= 2;
                word |= bit | ( bit << 1 );
                byte <<= 1;
              }
           wordptr[ j * numchars + i ] = word;
          }

    longptr  = (unsigned long  int *) outfont->tf_CharLoc;
    wordptr  = (unsigned short int *) outfont->tf_CharSpace;
    word2ptr = (unsigned short int *) outfont->tf_CharKern;

    for ( i = 0; i < numchars; i++ )
      {
        if ( longptr )
            longptr[i] = ((unsigned long int *) infont->tf_CharLoc)[i]   * 2;
        if ( wordptr )
            wordptr[i] = ((unsigned short int *) infont->tf_CharSpace)[i] * 2;
        if ( word2ptr )
            word2ptr[i] = ((unsigned short int *) infont->tf_CharKern)[i] * 2;
      }
 }

void BuildDoubleHeightFont ( name, outfont, infont, startrow )
unsigned char     *name;
register struct TextFont    *outfont,
                            *infont;
unsigned short int          startrow;
  {
    register unsigned short int  numchars,
                                 i,
                                 j;

    register unsigned short int  *word1ptr,
                                 *word2ptr;
    register unsigned long int   *longptr;

    if ( !outfont )
        return;
        
    numchars = infont->tf_Modulo / 2;

    outfont->tf_Message.mn_Node.ln_Name =  name;
    outfont->tf_YSize     = infont->tf_YSize;
    outfont->tf_Style     = infont->tf_Style & 0x4f;
    outfont->tf_Flags     = infont->tf_Flags;
    outfont->tf_XSize     = infont->tf_XSize;
    outfont->tf_Baseline  = infont->tf_Baseline;
    outfont->tf_BoldSmear = infont->tf_BoldSmear;
    outfont->tf_Accessors = 0;
    outfont->tf_LoChar    = infont->tf_LoChar;
    outfont->tf_HiChar    = infont->tf_HiChar;
    if ( !outfont->tf_CharData )
      {
        outfont->tf_CharData
        = (APTR) HSAllocMem ( (long) infont->tf_Modulo * infont->tf_YSize,
                            MEMF_CLEAR );
        if ( !outfont->tf_CharData )
          {
            DisplayAlert ( AG_NoMemory, MemMessage, 60L );
            ShutDown ();
          }
      }
      
    outfont->tf_Modulo    = infont->tf_Modulo;
    if ( infont->tf_CharLoc && !outfont->tf_CharLoc )
      {
        outfont->tf_CharLoc
        = (APTR) HSAllocMem ( (long) numchars * 4, MEMF_CLEAR );
        if ( !outfont->tf_CharLoc )
          {
            DisplayAlert ( AG_NoMemory, MemMessage, 60L );
            ShutDown ();
          }
      }
      
    if ( infont->tf_CharSpace && !outfont->tf_CharSpace )
      {
        outfont->tf_CharSpace
        = (APTR) HSAllocMem ( (long) numchars * 2, MEMF_CLEAR );
        if ( !outfont->tf_CharSpace )
          {
            DisplayAlert ( AG_NoMemory, MemMessage, 60L );
            ShutDown ();
          }
      }
    if ( infont->tf_CharKern  && !outfont->tf_CharKern )
      {
        outfont->tf_CharKern
        = (APTR) HSAllocMem ( (long) numchars * 2, MEMF_CLEAR );
        if ( !outfont->tf_CharKern )
          {
            DisplayAlert ( AG_NoMemory, MemMessage, 60L );
            ShutDown ();
          }
      }

    word1ptr = (unsigned short int *) infont->tf_CharData;
    word2ptr = (unsigned short int *) outfont->tf_CharData;
    numchars = infont->tf_Modulo / 2;

    for ( i = 0; i < numchars; i++ )
      {
        for ( j = 0; j < 4; j++ )
          {
            word2ptr [ numchars *   j * 2          + i ] =
            word2ptr [ numchars * ( j * 2 + 1 )    + i ] =
            word1ptr [ numchars * ( j + startrow ) + i ];
          }
      }

    longptr  = (unsigned long  int *) outfont->tf_CharLoc;
    word1ptr = (unsigned short int *) outfont->tf_CharSpace;
    word2ptr = (unsigned short int *) outfont->tf_CharKern;

    for ( i = 0; i < numchars; i++ )
      {
        if ( longptr )
            longptr[i] = ( (unsigned long  int *) infont->tf_CharLoc)[i];
        if ( word1ptr )
            word1ptr[i] = ( (unsigned short int *) infont->tf_CharSpace)[i];
        if ( word2ptr )
            word2ptr[i] = ( (unsigned short int *) infont->tf_CharKern)[i];
      }
  }

void BuildNarrowWidthFont ( name, outfont, infont )
unsigned char     *name;
register struct TextFont    *outfont,
                            *infont;

  {
    register unsigned short int  numchars,
                                 i,j;


    register unsigned char       chr;

    register unsigned short int  *word1ptr,
                                 *word2ptr;
    register unsigned char       HiChar,
                                 *byte1ptr,
                                 *byte2ptr;
    register unsigned long int   *longptr;

    if ( !outfont )
        return;
        
    HiChar = (infont->tf_HiChar > 126 ) ? 126 : infont->tf_HiChar;
    numchars = HiChar - infont->tf_LoChar + 1;
    numchars = (numchars + 1) & 0xfffe;

    outfont->tf_Message.mn_Node.ln_Name =  name;
    outfont->tf_YSize     = infont->tf_YSize;
    outfont->tf_Style     = infont->tf_Style & 0x4f;
    outfont->tf_Flags     = infont->tf_Flags;
    outfont->tf_XSize     = 5;
    outfont->tf_Baseline  = infont->tf_Baseline;
    outfont->tf_BoldSmear = infont->tf_BoldSmear;
    outfont->tf_Accessors = 0;
    outfont->tf_LoChar    = infont->tf_LoChar;
    outfont->tf_HiChar    = HiChar;
    if ( !outfont->tf_CharData )
      {
        outfont->tf_CharData
        = (APTR) HSAllocMem ( (long) numchars * infont->tf_YSize, MEMF_CLEAR );
        if ( !outfont->tf_CharData )
          {
            DisplayAlert ( AG_NoMemory, MemMessage, 60L );
            ShutDown ();
          }
      }
    outfont->tf_Modulo    = numchars;

    if ( infont->tf_CharLoc   && !outfont->tf_CharLoc )
      {
        outfont->tf_CharLoc
        = (APTR) HSAllocMem ( (long) numchars * 4, MEMF_CLEAR );
        if ( !outfont->tf_CharLoc )
          {
            DisplayAlert ( AG_NoMemory, MemMessage, 60L );
            ShutDown ();
          }
      }
    if ( infont->tf_CharSpace && !outfont->tf_CharSpace )
      {
        outfont->tf_CharSpace
        = (APTR) HSAllocMem ( (long) numchars * 2, MEMF_CLEAR );
        if ( !outfont->tf_CharSpace )
          {
            DisplayAlert ( AG_NoMemory, MemMessage, 60L );
            ShutDown ();
          }
      }
    if ( infont->tf_CharKern  && !outfont->tf_CharKern )
      {
        outfont->tf_CharKern
        = (APTR) HSAllocMem ( (long) numchars * 2, MEMF_CLEAR );
        if ( !outfont->tf_CharKern )
          {
            DisplayAlert ( AG_NoMemory, MemMessage, 60L );
            ShutDown ();
          }
      }

    byte1ptr = (unsigned char *) infont->tf_CharData;
    byte2ptr = (unsigned char *) outfont->tf_CharData;

    for ( i = 0; i < numchars; i++ )
      {
        for ( j = 0; j < infont->tf_YSize; j++ )
          {
            chr = byte1ptr [ j * infont->tf_Modulo + i ];
            byte2ptr [ j * numchars + i ]  = chr & 0xc0;
            chr <<= 1;
            byte2ptr [ j * numchars + i ] |= chr & 0x60;
            chr <<= 1;
            byte2ptr [ j * numchars + i ] |= chr & 0x30;
            chr <<= 1;
            byte2ptr [ j * numchars + i ] |= chr & 0x18;
          }
      }

    longptr  = (unsigned long  int *) outfont->tf_CharLoc;
    word1ptr = (unsigned short int *) outfont->tf_CharSpace;
    word2ptr = (unsigned short int *) outfont->tf_CharKern;

    for ( i = 0; i < numchars; i++ )
      {
        if ( longptr )
            longptr[i]
            = ( ( (unsigned long int *) infont->tf_CharLoc)[i] & 0xffff0000 ) | 5;
        if ( word1ptr )
            word1ptr[i] = 5;
        if ( word2ptr )
            word2ptr[i] = ( (unsigned short int *) infont->tf_CharKern)[i];
      }
  }

void BuildDSCSFont ( outfont, infont )
register struct TextFont    *outfont,
                            *infont;
  {
    register short int          offset,
                                offset2,
                                i;

    register unsigned char      *byteptr,
                                *byteptr2;
    register unsigned long int  *longptr;

    offset  = 0xa1 - infont->tf_LoChar;
    offset2 = (((unsigned long *)infont->tf_CharLoc)[offset] >> 16) /
              infont->tf_XSize;
              
    byteptr  = ((unsigned char *) infont->tf_CharData ) + offset2;
    byteptr2 = ((unsigned char *) outfont->tf_CharData) + 1;  
    
    for ( i = 0; i < infont->tf_YSize; i++ )
      {
        memcpy ( byteptr2, byteptr, outfont->tf_Modulo - 1 );
        byteptr  += infont->tf_Modulo;
        byteptr2 += outfont->tf_Modulo;
      }

    longptr  = (unsigned long  int *) outfont->tf_CharLoc;

    for ( i = 0; i < outfont->tf_Modulo; i++ )
        longptr[i] = ( ((long)i * infont->tf_XSize) << 16L) | 8L;
 }

void SetSoftChar ( num, char_box )
unsigned short int  num;
unsigned char       *char_box;
  {
    unsigned short int  softrow;
    unsigned short int  charrow;
    unsigned char       *sptr;
    
    sptr = (unsigned char *)softchars;
    for ( softrow = 0; softrow < 8; softrow++ )
        sptr[ softrow * 98 + num ] = '\0';
        
    softrow = 0;
    for ( charrow = 0; charrow < 10; charrow++ )
      {
        sptr[ softrow * 98 + num ] |= char_box[ charrow ];
        if ( charrow != 0 && charrow != 8 )
            softrow++;
      }
 }

void ClearSoftFont ()
  {
    unsigned char       char_box[10];

    memset ( (unsigned char *) softchars, 0xaa, sizeof(softchars) );
    
    memset ( char_box, 0x00, sizeof(char_box ) );
    SetSoftChar ( 0, char_box );
    
    BuildDoubleWidthFont  ( "DWSoft40.font",  dwsoft40font,    soft80font );
    BuildDoubleHeightFont ( "TopSoft40.font", topdhsoft40font, dwsoft40font, 0 );
    BuildDoubleHeightFont ( "BotSoft40.font", botdhsoft40font, dwsoft40font, 4 );
    BuildNarrowWidthFont  ( "soft128.font",   soft128font,     soft80font);
    
    BuildDoubleWidthFont  ( "DWSoft64.font",  dwsoft64font,    soft128font );
    BuildDoubleHeightFont ( "TopSoft64.font", topdhsoft64font, dwsoft64font, 0 );
    BuildDoubleHeightFont ( "BotSoft64.font", botdhsoft64font, dwsoft64font, 4 );

  }

void BuildSoftFont ( dcsptr )
unsigned char   *dcsptr;
  {
  
    unsigned short int  currchar;
    unsigned short int  topwidth;
    unsigned short int  botwidth;
    unsigned short int  justify;
    unsigned char       char_box[10];
    unsigned char       top_bot;
    unsigned char       col;
    unsigned char       bcount;
    
    currchar = ep[2];
    
    if ( !ep[3] )
        ClearSoftFont ();
        
    memset ( char_box, '\0', sizeof(char_box ) );
    
    top_bot = 0;
    topwidth = botwidth = 0;
    for (;; ++dcsptr )
      {
        if ( *dcsptr == '/' )
          {
            top_bot = 1;
          }
        else if ( *dcsptr == ';' || *dcsptr == '\0' )
          {
            justify = 8 - topwidth;
            for ( bcount = 0; bcount < 6; bcount++ )
              {
                char_box[bcount] <<= justify;
              }
            justify = 8 - botwidth;
            for ( bcount = 6; bcount < 10; bcount++ )
              {
                char_box[bcount] <<= justify;
              }
              
            SetSoftChar ( currchar, char_box );
            ++currchar;
            memset ( char_box, '\0', sizeof(char_box) );
            top_bot = 0;
            topwidth = botwidth = 0;
          }
        else if ( *dcsptr < '?' )
            continue;
        else
          {
            switch ( top_bot )
              {
                case 0: /* Top    */
                    topwidth++;
                    col = *dcsptr - 0x3f;
                    for ( bcount = 0; bcount < 6; bcount++ )
                      {
                        char_box[bcount] <<= 1;
                        char_box[bcount] |= col & 0x01;
                        col >>= 1;
                      }
                    break;
                case 1: /* Bottom */
                    botwidth++;
                    col = *dcsptr - 0x3f;
                    for ( bcount = 6; bcount < 10; bcount++ )
                      {
                        char_box[bcount] <<= 1;
                        char_box[bcount] |= col & 0x01;
                        col >>= 1;
                      }
                    break;
              }
          }
        if ( !*dcsptr )
            break;
      }
      
    BuildDoubleWidthFont  ( "DWSoft40.font",  dwsoft40font,    soft80font );
    BuildDoubleHeightFont ( "TopSoft40.font", topdhsoft40font, dwsoft40font, 0 );
    BuildDoubleHeightFont ( "BotSoft40.font", botdhsoft40font, dwsoft40font, 4 );
    BuildNarrowWidthFont  ( "soft128.font",   soft128font,     soft80font);
    
    BuildDoubleWidthFont  ( "DWSoft64.font",  dwsoft64font,    soft128font );
    BuildDoubleHeightFont ( "TopSoft64.font", topdhsoft64font, dwsoft64font, 0 );
    BuildDoubleHeightFont ( "BotSoft64.font", botdhsoft64font, dwsoft64font, 4 );

  }

/****************************************************************************/

void ChangeFont ( font )
struct TextFont *font;
  {
    ScrFlush ();
    SetFont ( RPort, font );
    SetFontAttributes ();
    CurrentFont = font;
    if ( font == NormalFont )
        tmodes.font = FASCII;
    else if ( font == UKFont )
        tmodes.font = FUK;
    else if ( font == SGCSFont )
        tmodes.font = FSGCS;
    else if ( font == DSCSFont )
        tmodes.font = FDSCS;
    else if ( font == SoftFont )
        tmodes.font = FSOFT;
  }
  

