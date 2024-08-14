/* -----------------------------------------------------------------------
 * terminalmenu.h		handshake_src
 *
 * $Locker:  $
 *
 * $Id: terminalmenu.h,v 1.1 91/05/09 14:36:12 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/h/RCS/terminalmenu.h,v 1.1 91/05/09 14:36:12 bj Exp $
 *
 * $Log:	terminalmenu.h,v $
 * Revision 1.1  91/05/09  14:36:12  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

/*
* Bit plane submenu
*/

static struct IntuiText bitplane4text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Colour/Slow",  /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem bitplane4 =
  {
    NULL,           /* No Next MenuItem         */
    100,10,147,10,  /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKED     |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &bitplane4text,/* ItemFill           */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText bitplane2text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Monochrome/Fast",/* Itext                  */
    NULL            /* No next string           */
  };

static struct MenuItem bitplane2 =
  {
    &bitplane4,     /* Next MenuItem            */
    100,0,147,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &bitplane2text,/* ItemFill           */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/*
* Answer back submenu
*/

static struct IntuiText answrtranstext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Transmit",     /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem answertransmit =
  {
    NULL,           /* No Next MenuItem         */
    100,10,120,10,  /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    COMMSEQ     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &answrtranstext,/* ItemFill          */
    NULL,           /* SelectFill               */
    'm',            /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText answersettext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Set",          /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem answerset =
  {
    &answertransmit,/* Next MenuItem            */
    100,0,120,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &answersettext,/* ItemFill           */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/*
* Margin bell submenu
*/

static struct IntuiText marbellofftext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Off",          /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem marbelloff =
  {
    NULL,           /* No Next MenuItem         */
    100,10,100,10,  /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKED     |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &marbellofftext,/* ItemFill          */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText marbellontext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "On",           /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem marbellon =
  {
    &marbelloff,    /* Next MenuItem            */
    100,0,100,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &marbellontext,/* ItemFill           */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/*
* Bell type submenu
*/

static struct IntuiText bellnonetext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "None",         /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem bellnone =
  {
    NULL,           /* No Next MenuItem         */
    100,30,100,10,  /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKED     |
    CHECKIT     |
    HIGHCOMP,
    0xfffffff7,     /* MutualExclude            */
    (APTR) &bellnonetext,/* ItemFill            */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText bellbothtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Both",         /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem bellboth =
  {
    &bellnone,      /* Next MenuItem            */
    100,20,100,10,  /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffb,     /* MutualExclude            */
    (APTR) &bellbothtext,/* ItemFill            */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText bellvisualtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Visual",       /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem bellvisual =
  {
    &bellboth,      /* Next MenuItem            */
    100,10,100,10,  /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &bellvisualtext,/* ItemFill          */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText bellaudiotext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Audio",        /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem bellaudio =
  {
    &bellvisual,    /* Next MenuItem            */
    100,0,100,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &bellaudiotext,/* ItemFill           */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/*
* Backspace and Delete key submenu
*/

static struct IntuiText bkdelrevtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Reversed",     /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem bkdelrev =
  {
    NULL,           /* No Next MenuItem         */
    100,10,100,10,  /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKED     |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &bkdelrevtext,/* ItemFill            */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText bkdelnormtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Normal",       /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem bkdelnorm =
  {
    &bkdelrev,      /* Next MenuItem            */
    100,0,100,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &bkdelnormtext,/* ItemFill           */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/*
* Interlace mode submenu
*/

static struct IntuiText lines48text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "48 lines",     /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem lines48 =
  {
    NULL    ,       /* No Next MenuItem         */
    100,30,195,10,  /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffff7,     /* MutualExclude            */
    (APTR) &lines48text,/* ItemFill       */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText interlacefulltext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "24 line interlace",/* Itext                */
    NULL            /* No next string           */
  };

static struct MenuItem interlacefull =
  {
    &lines48,       /* No Next MenuItem         */
    100,20,195,10,  /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffb,     /* MutualExclude            */
    (APTR) &interlacefulltext,/* ItemFill       */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText interlaceontext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "24 line half screen",/* Itext              */
    NULL            /* No next string           */
  };

static struct MenuItem interlaceon =
  {
    &interlacefull, /* No Next MenuItem         */
    100,10,195,10,  /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &interlaceontext,/* ItemFill         */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText interlaceofftext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "24 line interlace off",/* Itext            */
    NULL            /* No next string           */
  };

static struct MenuItem interlaceoff =
  {
    &interlaceon,   /* Next MenuItem            */
    100,0,195,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &interlaceofftext,/* ItemFill        */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/*
* Column submenu
*/

static struct IntuiText column128text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "132",          /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem column128 =
  {
    NULL,           /* No Next MenuItem         */
    100,10,80,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &column128text,/* ItemFill           */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText column80text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "80",           /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem column80 =
  {
    &column128,     /* Next MenuItem            */
    100,0,80,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &column80text,/* ItemFill            */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/*
* Screen mode submenu
*/

static struct IntuiText screenreversetext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Reverse",      /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem screenreverse =
  {
    NULL,           /* No Next MenuItem         */
    100,10,80,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKED     |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &screenreversetext,/* ItemFill       */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText screennormaltext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Normal",       /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem screennormal =
  {
    &screenreverse, /* Next MenuItem         */
    100,0,80,10,     /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &screennormaltext,/* ItemFill        */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/*
* New Line submenu
*/

static struct IntuiText newlineofftext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Off",          /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem newlineoff =
  {
    NULL,           /* No Next MenuItem         */
    100,10,60,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKED     |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &newlineofftext,/* ItemFill          */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText newlineontext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "On",           /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem newlineon =
  {
    &newlineoff,    /* Next MenuItem            */
    100,0,70,10,     /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &newlineontext,/* ItemFill           */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/*
* Character set submenu
*/
static struct IntuiText charsetUKtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "UK",          /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem charsetUK =
  {
    NULL,           /* No Next MenuItem         */
    100,10,60,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKED     |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &charsetUKtext,/* ItemFill           */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText charsetUStext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "ASCII",        /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem charsetUS =
  {
    &charsetUK,     /* Next MenuItem            */
    100,0,70,10,     /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &charsetUStext,/* ItemFill           */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/*
* Echo submenu
*/
static struct IntuiText echoofftext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Off",          /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem echooff =
  {
    NULL,           /* No Next MenuItem         */
    100,10,60,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKED     |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &echoofftext,/* ItemFill              */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText echoontext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "On",           /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem echoon =
  {
    &echooff,       /* Next MenuItem            */
    100,0,60,10,     /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &echoontext,/* ItemFill              */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/*
* Cursor submenu
*/
static struct IntuiText cursorblinktext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Blinking",     /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem cursorblink =
  {
    NULL,           /* No Next MenuItem         */
    100,40,125,10,  /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKED     |
    CHECKIT     |
    HIGHCOMP,
    0xfffffff4,     /* MutualExclude            */
    (APTR) &cursorblinktext,/* ItemFill     v   */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText cursorsteadytext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Non Blinking", /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem cursorsteady =
  {
    &cursorblink,   /* No Next MenuItem         */
    100,30,125,10,  /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKED     |
    CHECKIT     |
    HIGHCOMP,
    0xfffffff8,     /* MutualExclude            */
    (APTR) &cursorsteadytext,/* ItemFill        */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText cursorultext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Underline",    /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem cursorul =
  {
    &cursorsteady,  /* No Next MenuItem         */
    100,10,125,10,  /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKED     |
    CHECKIT     |
    HIGHCOMP,
    0xfffffff1,     /* MutualExclude            */
    (APTR) &cursorultext,/* ItemFill            */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText cursorbktext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Block",        /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem cursorbk =
  {
    &cursorul,      /* Next MenuItem            */
    100,0,125,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffff2,     /* MutualExclude            */
    (APTR) &cursorbktext,/* ItemFill            */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/*
* Scroll submenu
*/
static struct IntuiText scrslowtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Smooth",       /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem scrslow =
  {
    NULL,           /* No Next MenuItem         */
    100,10,80,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &scrslowtext,/* ItemFill             */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText scrfasttext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Jump",         /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem scrfast =
  {
    &scrslow,       /* Next MenuItem            */
    100,0,80,10,     /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKED     |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &scrfasttext,/* ItemFill             */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/*
* Wrap submenu
*/
static struct IntuiText wrpofftext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Off",          /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem wrpoff =
  {
    NULL,           /* No Next MenuItem         */
    100,10,60,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKED     |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &wrpofftext,/* ItemFill              */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText wrpontext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "On",           /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem wrpon =
  {
    &wrpoff,        /* Next MenuItem            */
    100,0,60,10,     /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &wrpontext,/* ItemFill               */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/*
* Mode submenu
*/
static struct IntuiText vt200_8text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "VT220 8 bit controls",/* Itext             */
    NULL            /* No next string           */
  };

static struct MenuItem vt200_8 =
  {
    NULL,           /* Next MenuItem            */
    100,30,190,10,  /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    CHECKED     |
    HIGHCOMP,
    0xfffffff7,     /* MutualExclude            */
    (APTR) &vt200_8text,/* ItemFill             */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText vt200_7text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "VT220 7 bit controls",/* Itext             */
    NULL            /* No next string           */
  };

static struct MenuItem vt200_7 =
  {
    &vt200_8,       /* Next MenuItem            */
    100,20,190,10,  /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    CHECKED     |
    HIGHCOMP,
    0xfffffffb,     /* MutualExclude            */
    (APTR) &vt200_7text,/* ItemFill             */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText vt100text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "VT102",        /* Itext            */
    NULL            /* No next string           */
  };

static struct MenuItem vt100 =
  {
    &vt200_7,      /* Next MenuItem            */
    100,10,190,10,  /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    CHECKED     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &vt100text,/* ItemFill               */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText vt52text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "VT52",         /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem vt52 =
  {
    &vt100,         /* No Next MenuItem         */
    100,0,190,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &vt52text,/* ItemFill                */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/***
*
* Terminal Menu level 1 items
*
***/

static struct IntuiText bitplaneitemtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Display Mode", /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem bitplaneitem =
  {
    NULL,           /* No Next menu item        */
    0,150,110,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &bitplaneitemtext,/* ItemFill        */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &bitplane2,     /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText marbellitemtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Margin Bell",  /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem marbellitem =
  {
    &bitplaneitem,  /* No Next menu item        */
    0,140,110,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &marbellitemtext,/* ItemFill         */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &marbellon,     /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText bellitemtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Bell Type",    /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem bellitem =
  {
    &marbellitem,   /* No Next menu item        */
    0,130,110,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &bellitemtext,/* ItemFill            */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &bellaudio,     /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText bkdelitemtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Bksp & Del",   /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem bkdelitem =
  {
    &bellitem,      /* No Next menu item        */
    0,120,110,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &bkdelitemtext, /* ItemFill          */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &bkdelnorm,     /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText interlacetext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Screen Format",/* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem interlaceitem =
  {
    &bkdelitem,     /* No Next menu item        */
    0,110,110,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &interlacetext, /* ItemFill          */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &interlaceoff,  /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText columntext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Columns/Line", /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem columnitem =
  {
    &interlaceitem, /* Next menu item           */
    0,100,110,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &columntext, /* ItemFill             */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &column80,      /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText screentext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Screen Mode",  /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem screenitem =
  {
    &columnitem,    /* No Next menu item        */
    0,90,110,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &screentext, /* ItemFill             */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &screennormal,  /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText newlinetext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "New Line Mode",/* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem newlineitem =
  {
    &screenitem,    /* No Next menu item        */
    0,80,110,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &newlinetext, /* ItemFill            */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &newlineon,     /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText charsettext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Char. Set",    /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem charsetitem =
  {
    &newlineitem,   /* No Next menu item        */
    0,70,110,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &charsettext, /* ItemFill            */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &charsetUS,     /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText answerbacktext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Answer Back",  /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem answerbackitem =
  {
    &charsetitem,   /* No Next menu item        */
    0,60,110,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &answerbacktext, /* ItemFill         */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &answerset,     /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText settabtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Set Tabs",     /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem settabitem =
  {
    &answerbackitem,/* No Next menu item        */
    0,50,110,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &settabtext, /* ItemFill             */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText echotext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Local Echo",   /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem echoitem =
  {
    &settabitem,    /* No Next menu item        */
    0,40,110,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &echotext, /* ItemFill               */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &echoon,        /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText cursortext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Cursor",       /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem cursoritem =
  {
    &echoitem,      /* No Next menu item        */
    0,30,110,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &cursortext, /* ItemFill             */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &cursorbk,      /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText scrolltext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Scroll Mode",  /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem scrollitem =
  {
    &cursoritem,    /* No Next menu item        */
    0,20,110,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &scrolltext, /* ItemFill             */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &scrfast,       /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText wraptext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Line Wrap",    /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem wrapitem =
  {
    &scrollitem,    /* No Next menu item        */
    0,10,110,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &wraptext, /* ItemFill               */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &wrpon ,        /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText modetext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Terminal Type",/* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem modeitem =
  {
    &wrapitem,      /* No Next menu item        */
    0,0,110,10,     /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &modetext, /* ItemFill               */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &vt52,          /* First subitem            */
    0               /* Next Select              */
  };

