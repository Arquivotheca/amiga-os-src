/* -----------------------------------------------------------------------
 * projectmenu.h		handshake_src
 *
 * $Locker:  $
 *
 * $Id: projectmenu.h,v 1.1 91/05/09 14:35:11 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/h/RCS/projectmenu.h,v 1.1 91/05/09 14:35:11 bj Exp $
 *
 * $Log:	projectmenu.h,v $
 * Revision 1.1  91/05/09  14:35:11  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

/***
*
* Project menu level 2 items
*
***/

/*
* Printer submenu
*/
static struct IntuiText printnonetext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "None",         /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem printnone =
  {
    NULL,           /* No Next MenuItem         */
    100,20,100,10,  /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffb,     /* MutualExclude            */
    (APTR) &printnonetext,/* ItemFill           */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText printpartext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "PAR:",         /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem printpar =
  {
    &printnone,     /* No Next MenuItem         */
    100,10,100,10,  /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &printpartext,/* ItemFill           */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText printprttext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "PRT:",         /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem printprt =
  {
    &printpar,      /* Next MenuItem            */
    100,0,100,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &printprttext,/* ItemFill            */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/*
* Filerequest submenu
*/
static struct IntuiText arpyestext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "ARP",          /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem arpyes =
  {
    NULL,           /* No Next MenuItem         */
    100,10,100,10,  /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &arpyestext,/* ItemFill              */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText arpnotext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Simple",       /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem arpno =
  {
    &arpyes,        /* Next MenuItem            */
    100,0,100,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKED     |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &arpnotext,/* ItemFill               */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/*
* Save screen to  file/device submenu
*/
static struct IntuiText screenprinttext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Print",        /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem screenprint =
  {
    NULL,           /* No Next MenuItem         */
    100,30,150,10,  /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    COMMSEQ     |
    HIGHCOMP,
    0xfffffff7,     /* MutualExclude            */
    (APTR) &screenprinttext,/* ItemFill        */
    NULL,           /* SelectFill               */
    '\\',           /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText screenappendtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Write Screen", /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem screenappend =
  {
    &screenprint,   /* No Next MenuItem         */
    100,20,150,10,  /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    COMMSEQ     |
    HIGHCOMP,
    0xfffffffb,     /* MutualExclude            */
    (APTR) &screenappendtext,/* ItemFill        */
    NULL,           /* SelectFill               */
    '=',            /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText screenclosetext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Close File",   /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem screenclose =
  {
    &screenappend,  /* Next MenuItem            */
    100,10,150,10,  /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    COMMSEQ     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &screenclosetext,/* ItemFill         */
    NULL,           /* SelectFill               */
    0x27,           /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };
static struct IntuiText screennewtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "New File",     /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem screennew =
  {
    &screenclose,   /* Next MenuItem            */
    100,0,150,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    COMMSEQ     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &screennewtext,/* ItemFill           */
    NULL,           /* SelectFill               */
    '-',            /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/*
* Icon submenu
*/
static struct IntuiText iconyestext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Yes",          /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem iconyes =
  {
    NULL,           /* No Next MenuItem         */
    100,10,100,10,  /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKED     |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &iconyestext,/* ItemFill             */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

static struct IntuiText iconnotext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "No",           /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem iconno =
  {
    &iconyes,      /* Next MenuItem            */
    100,0,100,10,   /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    CHECKIT     |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &iconnotext,/* ItemFill           */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* No subitem               */
    0               /* Next Select              */
  };

/***
*
* Project menu level 1 items
*
***/

static struct IntuiText fonttext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Font",         /* Itext                   */
    NULL            /* No next string           */
  };

static struct MenuItem fontitem =
  {
    NULL,           /* Next menu item           */
    0,110,          /* LE, TE                   */
    130+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &fonttext,/* ItemFill               */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText printtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Printer",      /* Itext                   */
    NULL            /* No next string           */
  };

static struct MenuItem printitem =
  {
    &fontitem,      /* Next menu item           */
    0,100,          /* LE, TE                   */
    130+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &printtext,/* ItemFill               */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &printprt,      /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText arptext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "File Requester",/* Itext                   */
    NULL            /* No next string           */
  };

static struct MenuItem arpitem =
  {
    &printitem,     /* Next menu item           */
    0,90,           /* LE, TE                   */
    130+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &arptext,/* ItemFill                 */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &arpno,         /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText abouttext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "About ...",    /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem aboutitem =
  {
    &arpitem,       /* Next menu item           */
    0,80,           /* LE, TE                   */
    130+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &abouttext,/* ItemFill               */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText screensavetext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Save Image",   /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem screensaveitem =
  {
    &aboutitem,     /* Next menu item           */
    0,70,           /* LE, TE                   */
    130+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &screensavetext,/* ItemFill          */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &screennew,     /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText icontext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Create Icons", /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem iconitem =
  {
    &screensaveitem,/* Next menu item           */
    0,60,           /* LE, TE                   */
    130+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &icontext,/* ItemFill                */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &iconno,        /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText breaktext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Send Break",   /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem breakitem =
  {
    &iconitem,     /* Next menu item           */
    0,50,           /* LE, TE                   */
    130+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    COMMSEQ     |
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &breaktext,/* ItemFill               */
    NULL,           /* SelectFill               */
   'b' ,            /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText resettext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Reset Term",   /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem resetitem =
  {
    &breakitem,     /* Next menu item           */
    0,40,           /* LE, TE                   */
    130+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    COMMSEQ     |
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &resettext,/* ItemFill               */
    NULL,           /* SelectFill               */
   'v' ,            /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText colourtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Set colours",  /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem colouritem =
  {
    &resetitem,     /* Next menu item           */
    0,30,           /* LE, TE                   */
    130+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    COMMSEQ     |
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &colourtext,/* ItemFill              */
    NULL,           /* SelectFill               */
    '*',            /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText functionkeytext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Function keys",/* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem functionkeyitem =
  {
    &colouritem,     /* Next menu item           */
    0,20,           /* LE, TE                   */
    130+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    COMMSEQ     |
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &functionkeytext,/* ItemFill         */
    NULL,           /* SelectFill               */
    'f',            /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText loadtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Load Parms",   /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem loaditem =
  {
    &functionkeyitem,/* Next menu item          */
    0,10,           /* LE, TE                   */
    130+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    COMMSEQ     |
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &loadtext,/* ItemFill                */
    NULL,           /* SelectFill               */
   'l',            /* Command                   */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText savetext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Save Parms",   /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem saveitem =
  {
    &loaditem,      /* Next menu item           */
    0, 0,           /* LE, TE                   */
    130+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    COMMSEQ     |
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &savetext,/* ItemFill                 */
    NULL,           /* SelectFill               */
    's',              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText quittext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Quit",         /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem quititem =
  {
    &saveitem,      /* Next menu item           */
    0,120,          /* LE, TE                   */
    130+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    COMMSEQ     |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &quittext, /* ItemFill               */
    NULL,           /* SelectFill               */
    'q',            /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

