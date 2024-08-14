/* -----------------------------------------------------------------------
 * phonemenu.h      handshake_src
 *
 * $Locker:  $
 *
 * $Id: phonemenu.h,v 1.1 91/05/09 14:33:04 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/h/RCS/phonemenu.h,v 1.1 91/05/09 14:33:04 bj Exp $
 *
 * $Log:	phonemenu.h,v $
 * Revision 1.1  91/05/09  14:33:04  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

/***
*
* Dial mode submenu
*
***/

/*
* Auto Hangup menu submenu
*/

static struct IntuiText hangupyestext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Yes",          /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem hangupyes =
  {
    NULL,           /* No Next MenuItem         */
    130,10,60,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKED     |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &hangupyestext,/* ItemFill           */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText hangupnotext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "No",           /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem hangupno =
  {
    &hangupyes,     /* Next MenuItem            */
    130,0,70,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &hangupnotext,/* ItemFill            */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/*
* Auto dialing submenu
*/

static struct IntuiText autodialofftext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Off",          /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem autodialoff =
  {
    NULL,           /* No Next MenuItem         */
    130,10,60,10,    /* LE, TE, Width, Height   */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKED     |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &autodialofftext,/* ItemFill         */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText autodialontext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "On",           /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem autodialon =
  {
    &autodialoff,   /* Next MenuItem            */
    130,0,70,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &autodialontext,/* ItemFill          */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/*
* Redial delay subnemu
*/
static struct IntuiText dialdelay300text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "300 seconds",  /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem dialdelay300 =
  {
    NULL,   /* No Next MenuItem         */
    80,60,120,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xffffffbf,     /* MutualExclude            */
    (APTR) &dialdelay300text,/* ItemFill        */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText dialdelay180text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "180 seconds",  /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem dialdelay180 =
  {
    &dialdelay300,   /* No Next MenuItem         */
    80,50,120,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xffffffdf,     /* MutualExclude            */
    (APTR) &dialdelay180text,/* ItemFill         */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText dialdelay120text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "120 seconds",  /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem dialdelay120 =
  {
    &dialdelay180,  /* No Next MenuItem         */
    80,40,120,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xffffffef,     /* MutualExclude            */
    (APTR) &dialdelay120text,/* ItemFill        */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText dialdelay90text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    " 90 seconds",  /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem dialdelay90 =
  {
    &dialdelay120,    /* No Next MenuItem        */
    80,30,120,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffff7,     /* MutualExclude            */
    (APTR) &dialdelay90text,/* ItemFill          */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText dialdelay60text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    " 60 seconds",  /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem dialdelay60 =
  {
    &dialdelay90,    /* No Next MenuItem         */
    80,20,120,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffb,     /* MutualExclude            */
    (APTR) &dialdelay60text,/* ItemFill          */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText dialdelay30text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    " 30 seconds",  /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem dialdelay30 =
  {
    &dialdelay60,    /* No Next MenuItem        */
    80,10,120,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &dialdelay30text,/* ItemFill         */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText dialdelay0text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Immediately",  /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem dialdelay0 =
  {
    &dialdelay30,    /* Next MenuItem            */
    80,0,120,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &dialdelay0text,/* ItemFill          */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/*
* Dial timeout subnemu
*/
static struct IntuiText dialtime120text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "120 seconds",  /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem dialtime120 =
  {
    NULL,           /* No Next MenuItem         */
    80,50,120,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xffffffdf,     /* MutualExclude            */
    (APTR) &dialtime120text,/* ItemFill         */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText dialtime90text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    " 90 seconds",  /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem dialtime90 =
  {
    &dialtime120,   /* No Next MenuItem         */
    80,40,120,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xffffffef,     /* MutualExclude            */
    (APTR) &dialtime90text,/* ItemFill          */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText dialtime60text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    " 60 seconds",  /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem dialtime60 =
  {
    &dialtime90,    /* No Next MenuItem         */
    80,30,120,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffff7,     /* MutualExclude            */
    (APTR) &dialtime60text,/* ItemFill          */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText dialtime45text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    " 45 seconds",  /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem dialtime45 =
  {
    &dialtime60,    /* No Next MenuItem         */
    80,20,120,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffb,     /* MutualExclude            */
    (APTR) &dialtime45text,/* ItemFill          */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText dialtime30text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    " 30 seconds",  /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem dialtime30 =
  {
    &dialtime45,    /* Next MenuItem            */
    80,10,120,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &dialtime30text,/* ItemFill          */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText dialtime20text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    " 20 seconds",  /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem dialtime20 =
  {
    &dialtime30,    /* Next MenuItem            */
    80,0,120,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &dialtime20text,/* ItemFill          */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/***
*
* Phone menu level 1 items
*
***/

static struct IntuiText dialhanguptext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Hang up before dial", /* Itext             */
    NULL            /* No next string           */
  };

static struct MenuItem dialhangupitem =
  {
    NULL,           /* Next menu item           */
    0,70,           /* LE, TE                   */
    150+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &dialhanguptext,/* ItemFill          */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &hangupno,      /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText dialdelaytext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Redial delay", /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem dialdelayitem =
  {
    &dialhangupitem,/* Next menu item           */
    0,60,           /* LE, TE                   */
    150+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &dialdelaytext,/* ItemFill           */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &dialdelay0,    /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText dialtimetext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Dial timeout", /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem dialtimeitem =
  {
    &dialdelayitem, /* Next menu item           */
    0,50,           /* LE, TE                   */
    150+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &dialtimetext,/* ItemFill            */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &dialtime20,    /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText dialstringtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Dial Pre-String",/* Itext                  */
    NULL            /* No next string           */
  };

static struct MenuItem dialstringitem =
  {
    &dialtimeitem,  /* Next menu item           */
    0,40,           /* LE, TE                   */
    150+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &dialstringtext,/* ItemFill          */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText autodialtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Auto Redial",  /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem autodialitem =
  {
    &dialstringitem,/* Next menu item           */
    0,30,           /* LE, TE                   */
    150+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &autodialtext,/* ItemFill            */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &autodialon,    /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText hanguptext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Hang Up",      /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem hangupitem =
  {
    &autodialitem,  /* Next menu item           */
    0,20,           /* LE, TE                   */
    150+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    COMMSEQ     |
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &hanguptext,/* ItemFill              */
    NULL,           /* SelectFill               */
    'h',            /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText redialtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Redial",      /* Itext                     */
    NULL            /* No next string           */
  };

static struct MenuItem redialitem =
  {
    &hangupitem,    /* Next menu item           */
    0,10,           /* LE, TE                   */
    150+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    COMMSEQ     |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &redialtext,/* ItemFill              */
    NULL,           /* SelectFill               */
    'd',            /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText dialtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Dial Phone",   /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem dialitem =
  {
    &redialitem,    /* Next menu item           */
    0,0,            /* LE, TE                   */
    150+COMMWIDTH,10,  /* Width, Height            */
    ITEMTEXT    |   /* Flags                    */
    COMMSEQ     |
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &dialtext,/* ItemFill                */
    NULL,           /* SelectFill               */
    'p',            /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

