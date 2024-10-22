/* -----------------------------------------------------------------------
 * setupmenu.h		handshake_src
 *
 * $Locker:  $
 *
 * $Id: setupmenu.h,v 1.1 91/05/09 14:33:55 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/h/RCS/setupmenu.h,v 1.1 91/05/09 14:33:55 bj Exp $
 *
 * $Log:	setupmenu.h,v $
 * Revision 1.1  91/05/09  14:33:55  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

/*
* Port selection submenu
*/

static struct IntuiText port8text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "8",            /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem port8 =
  {
    NULL,           /* Next MenuItem            */
    90,80,40,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffeff,     /* MutualExclude            */
    (APTR) &port8text,/* ItemFill               */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText port7text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "7",            /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem port7 =
  {
    &port8,         /* Next MenuItem            */
    90,70,40,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xffffff7f,     /* MutualExclude            */
    (APTR) &port7text,/* ItemFill               */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText port6text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "6",            /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem port6 =
  {
    &port7,         /* Next MenuItem            */
    90,60,40,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xffffffbf,     /* MutualExclude            */
    (APTR) &port6text,/* ItemFill               */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText port5text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "5",            /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem port5 =
  {
    &port6,         /* Next MenuItem            */
    90,50,40,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xffffffdf,     /* MutualExclude            */
    (APTR) &port5text,/* ItemFill               */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText port4text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "4",            /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem port4 =
  {
    &port5,         /* Next MenuItem            */
    90,40,40,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xffffffef,     /* MutualExclude            */
    (APTR) &port4text,/* ItemFill               */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText port3text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "3",            /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem port3 =
  {
    &port4,         /* Next MenuItem            */
    90,30,40,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffff7,     /* MutualExclude            */
    (APTR) &port3text,/* ItemFill               */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText port2text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "2",            /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem port2 =
  {
    &port3,         /* Next MenuItem            */
    90,20,40,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffb,     /* MutualExclude            */
    (APTR) &port2text,/* ItemFill               */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText port1text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "1",            /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem port1 =
  {
    &port2,         /* Next MenuItem            */
    90,10,40,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &port1text,/* ItemFill               */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText port0text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "0",            /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem port0 =
  {
    &port1,         /* Next MenuItem            */
    90,0,40,10,     /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &port0text,/* ItemFill               */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/*
* Flow control submenu
*/
static struct IntuiText xonxoffofftext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "none",         /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem xonxoffoff =
  {
    NULL,           /* No Next MenuItem         */
    80,10,100,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &xonxoffofftext,/* ItemFill          */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText xonxoffontext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "XON XOFF",     /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem xonxoffon =
  {
    &xonxoffoff,    /* Next MenuItem            */
    80,0,100,10,     /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &xonxoffontext,/* ItemFill           */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/*
* Hardware Handshaking submenu
*/
static struct IntuiText con3wiretext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "none",         /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem con3wire =
  {
    NULL,           /* No Next MenuItem         */
    80,10,100,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &con3wiretext,/* ItemFill            */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText con7wiretext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "RTS/CTS",      /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem con7wire =
  {
    &con3wire,      /* Next MenuItem            */
    80,0,100,10,     /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &con7wiretext,/* ItemFill            */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/*
* Parity submenu
*/
static struct IntuiText pspacetext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "space",        /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem pspace =
  {
    NULL,           /* No Next MenuItem         */
    80,40,60,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xffffffef,     /* MutualExclude            */
    (APTR) &pspacetext,/* ItemFill              */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText pmarktext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "mark",         /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem pmark =
  {
    &pspace,         /* No Next MenuItem         */
    80,30,60,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffff7,     /* MutualExclude            */
    (APTR) &pmarktext,/* ItemFill               */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText peventext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "even",         /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem peven =
  {
    &pmark,         /* No Next MenuItem         */
    80,20,60,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffb,     /* MutualExclude            */
    (APTR) &peventext,/* ItemFill               */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText poddtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "odd",          /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem podd =
  {
    &peven,         /* Next MenuItem            */
    80,10,60,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &poddtext,/* ItemFill                */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText pnonetext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "none",         /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem pnone =
  {
    &podd,          /* Next MenuItem            */
    80,0,60,10,     /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKED     |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &pnonetext,/* ItemFill               */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/*
* Stop bit submenu
*/
static struct IntuiText sb2text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "two",          /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem sb2 =
  {
    NULL,           /* Next MenuItem            */
    80,20,60,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffb,     /* MutualExclude            */
    (APTR) &sb2text,/* ItemFill                 */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText sb1text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "one",          /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem sb1 =
  {
    &sb2,           /* Next MenuItem            */
    80,10,60,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &sb1text,/* ItemFill                 */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText sb0text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "zero",          /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem sb0 =
  {
    &sb1,           /* Next MenuItem            */
    80,0,60,10,     /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKED     |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &sb0text,/* ItemFill              */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/*
* Data bits submenu
*/
static struct IntuiText db8text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "eight",        /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem db8 =
  {
    NULL,           /* Next MenuItem            */
    80,10,65,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKED     |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &db8text,/* ItemFill                 */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText db7text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "seven",        /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem db7 =
  {
    &db8,           /* Next MenuItem            */
    80,0,65,10,     /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &db7text,/* ItemFill              */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/*
* Baud rate submenu
*/
static struct IntuiText b19200text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "19200",        /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem b19200 =
  {
    NULL,           /* Next MenuItem            */
    80,90,68,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffdff,     /* MutualExclude            */
    (APTR) &b19200text, /* ItemFill             */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText b9600text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    " 9600",         /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem b9600 =
  {
    &b19200,        /* Next MenuItem            */
    80,80,68,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffeff,     /* MutualExclude            */
    (APTR) &b9600text, /* ItemFill              */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText b4800text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    " 4800",         /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem b4800 =
  {
    &b9600,         /* Next MenuItem            */
    80,70,68,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xffffff7f,     /* MutualExclude            */
    (APTR) &b4800text, /* ItemFill              */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText b3600text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    " 3600",        /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem b3600 =
  {
    &b4800,         /* Next MenuItem            */
    80,60,68,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xffffffbf,     /* MutualExclude            */
    (APTR) &b3600text, /* ItemFill              */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText b2400text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    " 2400",         /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem b2400 =
  {
    &b3600,         /* Next MenuItem            */
    80,50,68,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xffffffdf,     /* MutualExclude            */
    (APTR) &b2400text, /* ItemFill              */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText b1800text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    " 1800",        /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem b1800 =
  {
    &b2400,         /* Next MenuItem            */
    80,40,68,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xffffffef,     /* MutualExclude            */
    (APTR) &b1800text, /* ItemFill              */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText b1200text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    " 1200",        /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem b1200 =
  {
    &b1800,         /* Next MenuItem            */
    80,30,68,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKED     |
    CHECKIT     |
    HIGHCOMP,
    0xfffffff7,     /* MutualExclude            */
    (APTR) &b1200text,/* ItemFill               */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText b600text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "  600",        /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem b600 =
  {
    &b1200,         /* Next MenuItem            */
    80,20,68,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffb,     /* MutualExclude            */
    (APTR) &b600text,/* ItemFill                */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText b450text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "  450",        /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem b450 =
  {
    &b600,          /* Next MenuItem            */
    80,10,68,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &b450text,/* ItemFill                */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText b300text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "  300",        /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem b300 =
  {
    &b450,         /* Next MenuItems           */
    80,0,68,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |  /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &b300text, /* ItemFill               */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/***
*
* Serial Port menu level 1 items
*
***/

static struct IntuiText drivertext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Device Driver",/* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem driveritem =
  {
    NULL,           /* No Next menu item        */
    0,70,110,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &drivertext, /* ItemFill             */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText porttext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Serial Port",  /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem portitem =
  {
    &driveritem,    /* No Next menu item        */
    0,60,110,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &porttext, /* ItemFill               */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &port0,         /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText xonxofftext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Flow Control", /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem xonxoffitem =
  {
    &portitem,      /* No Next menu item        */
    0,50,110,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &xonxofftext, /* ItemFill            */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &xonxoffon,     /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText connecttext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Handshaking",  /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem connectitem =
  {
    &xonxoffitem,   /* No Next menu item        */
    0,40,110,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &connecttext, /* ItemFill            */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &con7wire,      /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText paritytext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Parity",       /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem parityitem =
  {
    &connectitem,   /* No Next menu item        */
    0,30,110,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &paritytext, /* ItemFill             */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &pnone,         /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText stpbtstext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Stop Bits",    /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem sbitem =
  {
    &parityitem,    /* Next menu item           */
    0,20,110,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &stpbtstext, /* ItemFill             */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &sb0,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText databitstext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Data Bits",    /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem dbitem =
  {
    &sbitem,        /* Next menu item           */
    0,10,110,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &databitstext, /* ItemFill           */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &db7,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText baudtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Baud Rate",    /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem bauditem =
  {
    &dbitem,        /* Next menu item           */
    0,0,110,10,     /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &baudtext, /* ItemFill               */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &b300,          /* First subitem            */
    0               /* Next Select              */
  };

